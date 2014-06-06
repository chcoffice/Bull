#include "RefSource.h"
#include "IBaseSource.h"
#include "Log.h"
#include "GSPProDebug.h"
#include "GSPMemory.h"


using namespace GSP;



static void _OnThreadPoolFreeData( CRefObject  *pObject)
{
    if( pObject )
    {
        pObject->UnrefObject();
    }
}

CRefSource::CRefSource(CBaseSource *pSource)
:CRefObject()
{
   GS_ASSERT_EXIT(pSource, -1);
   m_pSrc = pSource;  
   m_pSrc->RefObject();
   m_bWaitKey = TRUE;
   m_bStart = FALSE;
   m_pFnEvtOwner = NULL;
   m_fnEvtCallback = NULL;
   m_bPlay = FALSE;  
   m_eTranModel = GSP_TRAN_RTPLAY;
   m_iDelayCounts = 0;
   m_pSourcePrivateData = NULL;
   m_iFlowctrl = 0;
   m_bEnableSysHeader = 0;
   m_bQuickFrame = TRUE;
}

CRefSource::~CRefSource(void)
{
	
    m_pSrc->UnrefSource(this);
	m_pSrc->UnrefObject();
}


CIBaseSource *CRefSource::Source(void)
{
    return m_pSrc;
}

void CRefSource::SetTransMode( INT32 eMode )
{
	if( eMode==GSP_TRAN_RTPLAY )
	{
		m_bEnableSysHeader = 5;
		m_iDelayCounts =  0;
		m_bQuickFrame = TRUE;
	}
	else
	{
		m_bEnableSysHeader = 1;		
		m_bQuickFrame = FALSE;
	}
	m_eTranModel = eMode;
    m_pSrc->SetTransMode(eMode);        
}



void CRefSource::SetListener(CGSPObject *pFnEvtOwner, FuncPtrRefSourceEvent fnEvtCallback)
{
    m_pFnEvtOwner = pFnEvtOwner;
    m_fnEvtCallback = fnEvtCallback;
}

BOOL CRefSource::Start(void)
{
    GS_ASSERT_RET_VAL(m_fnEvtCallback, FALSE);
    m_bStart = TRUE;
    return m_bStart;
}

void CRefSource::Stop(void)
{
    m_bStart = FALSE;
}


INT32 CRefSource::GetCtrlAbilities(void)
{
     return  m_pSrc->GetCtrlAbilities()|GSP_CTRL_PLAY|GSP_CTRL_STOP|GSP_CTRL_FLOWCTRL;
}

 BOOL CRefSource::WaitSysHeader(void) 
 {
    if( m_bEnableSysHeader == 0 )
	{
		return FALSE;
	}
	m_bEnableSysHeader--;
	return TRUE;
 }



EnumErrno CRefSource::Ctrl(const StruGSPCmdCtrl &stCtrl)
{
	
    if( !m_bStart )
    {

       MY_LOG_WARN(g_pLog,  "%s Ctrl.( Cmd:%s ) refsource not start.\n", 
            m_pSrc->GetKey(), GSPCtrlName(stCtrl.iCtrlID ) );
       return eERRNO_SYS_ESTATUS;
    }

    MY_LOG_INFO(g_pLog,  "%s Ctrl.( Cmd:%s, Chn:%d, Args:0x%x-0x%x )\n", 
        m_pSrc->GetKey(), GSPCtrlName(stCtrl.iCtrlID ),
		stCtrl.iSubChn, stCtrl.iArgs1, stCtrl.iArgs2 );
    if( !(m_pSrc->GetCtrlAbilities()&stCtrl.iCtrlID)  )
    {
        return  eERRNO_SYS_ENCTRL;
    }

    switch( stCtrl.iCtrlID)
    {
	case GSP_CTRL_SECTION :
	case GSP_CTRL_PLAY:
	case GSP_CTRL_FAST :
	case GSP_CTRL_BFAST :
	case GSP_CTRL_BSTEP :
	case GSP_CTRL_STEP :
        {
            if(!m_bPlay)
            {
                m_bPlay = TRUE;   
				if( m_eTranModel == GSP_TRAN_RTPLAY )
				{
					m_bWaitKey = TRUE;   
				}
            }
        }
    break;
	case GSP_CTRL_PAUSE :
		 {           
			if( m_eTranModel == GSP_TRAN_RTPLAY )
			{
				//实时流 数据源不用停止
				 m_bPlay = FALSE;
				return eERRNO_SUCCESS;
			}
        }
    break;
    case GSP_CTRL_STOP :
        {
            m_bPlay = FALSE;
			if( m_eTranModel == GSP_TRAN_RTPLAY )
			{
				//实时流 数据源不用停止
				return eERRNO_SUCCESS;
			}
        }
    break;
	case GSP_CTRL_FLOWCTRL :
		{
			if(  stCtrl.iArgs1 )
			{
				m_iFlowctrl++;				
			}
			else
			{
				m_iFlowctrl = 0;
			}
		}
	break;
    default :
        {

        }
    }


	if( m_eTranModel != GSP_TRAN_RTPLAY )
	{
		m_bWaitKey = FALSE;
		if( m_bPlay && stCtrl.iCtrlID&GSP_CTRL_PLAY )
		{
			m_iDelayCounts =  5;
		}
	}
    EnumErrno eRet = m_pSrc->Ctrl( stCtrl );
	if( stCtrl.iCtrlID&(GSP_CTRL_PAUSE|GSP_CTRL_STOP) )
	{
		MSLEEP(10);
	}
	return eRet;
}

EnumErrno  CRefSource::OnSourceEvent(EnumRefSourceEvent eEvt, void *pParam)
{

	
    switch( eEvt  )
    {
    case eEVT_STREAM_FRAME :
        {
			CFrameCache *pFrame = (CFrameCache *)pParam;

			if( pFrame->m_stFrameInfo.bSysHeader )
			{
				if( m_bEnableSysHeader > 0)
				{
					return eERRNO_SRC_EUNUSED;
				}
				/*MY_LOG_WARN(g_pLog, "****RefSouce Start Send Sys Frame.\n");*/
			}

			if( m_iFlowctrl   )
			{
				if(m_eTranModel==GSP_TRAN_RTPLAY  )
				{
					if( m_iFlowctrl>2 || !pFrame->m_stFrameInfo.bKey )
					{
						//只发送I Frame
						return eERRNO_SYS_EFLOWOUT;
					}					
				}
				else
				{
					m_iDelayCounts = m_iFlowctrl*4+1;
				}
				
			}
			while( m_iDelayCounts>0 && m_bPlay && m_bStart  )
			{
				//文件下载/回放时 进行延时
				MSLEEP(10);
				m_iDelayCounts--;
			}
			

			if( (!m_bPlay && m_eTranModel==GSP_TRAN_RTPLAY) || !m_bStart )
			{
				return eERRNO_SRC_EUNUSED;
			}


            
            if(!pFrame->m_stFrameInfo.bSysHeader 
				&& m_bWaitKey 
				&& pFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_VIDEO )
            {

                if( pFrame->m_stFrameInfo.bKey )
                {
                    m_bWaitKey = FALSE;
                }
                else
                {
                    return eERRNO_SUCCESS;
                }
            }

// 			if( pFrame->m_bSysHeader )
// 			{
// 				MY_LOG_WARN(g_pLog, "****RefSouce Send Sys Frame.\n");
// 			}
        }
        break;
    case eEVT_PLAY_STATUS :
        if( !m_bPlay || !m_bStart )
        {
              return eERRNO_SRC_EUNUSED;
        }
    break;

    case eEVT_SOURCE_RELEASE :
	case eEVT_SOURCE_ASSERT :
	case eEVT_PLAY_END :
    break;
    default :
        return eERRNO_SYS_EINVALID;
    break;
    }
    if( m_fnEvtCallback )
    {
        return (m_pFnEvtOwner->*m_fnEvtCallback)(this, eEvt, pParam);
    }
     return eERRNO_SRC_EUNUSED; 
}


void CRefSource::Release(void)
{
	m_bStart = FALSE;
	m_pSrc->UnrefSource(this);
	m_pSrc->UnrefObject();
}

