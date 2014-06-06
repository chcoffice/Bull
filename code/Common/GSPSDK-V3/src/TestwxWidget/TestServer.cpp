/////////////////////////////////////////////////////////////////////////////
// Name:        TestServer.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "TestServer.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "TestServer.h"
#include "OSThread.h"
#include "MainApp.h"
#include "md5.h"
#include "Log.h"
#include "GSPProDebug.h"

using namespace GSP;




static CGSString s_strTestFile = "hall_monitor_1M_300f_3sI.mp4";  //"test_stmp4.mp4"; 
static BOOL s_bTestModFile = 1;
static CGSString s_strTestIni = "test.ini";
static UINT s_iNumFrame = 100;
static UINT s_iFrameRate = 25;


static void OnSourceEvent(CIServer *pServer, CISource *pSource,
							   EnumGSPServerEvent eEvent, 
							   void *pEventPararms, INT iLen,
                               CTestSrvDialog *pTestSrvDlg)
{
    GS_ASSERT_RET(pTestSrvDlg);
    pTestSrvDlg->OnServerSourceEvent(pServer, pSource, eEvent, pEventPararms);
}


// WDR: class implementations 

//----------------------------------------------------------------------------
// CSenderObject
//----------------------------------------------------------------------------
CSenderObj::CSenderObj( void )
:CGSPObject()
,m_pDlg(NULL)
,m_csTasker("SrvSender")
{
    m_iInterval = 0;
    m_iTicks = 0;
    m_iVideoFileSize = 0;
	m_csTasker.Init(this,(FuncPtrObjThreadPoolEvent) &CSenderObj::OnTaskEvent, 1,TRUE);
	m_csTasker.SetMaxWaitTask(5000);
    m_iStep = 0;
     m_iBufSize = 0;
     m_pBuffer = NULL;
     if( s_bTestModFile )
     {
         CGSString strFile = GSGetApplicationPath()+s_strTestFile;
         FILE *fp = fopen( strFile.c_str(), "rb");
         if( fp )
         {
             m_iBufSize = MBYTES*4;
             m_pBuffer = (unsigned char *)::malloc( m_iBufSize );
             if( !m_pBuffer )
             {
                 m_iBufSize = 0;
             } 
             bzero(m_pBuffer, m_iBufSize);

             m_iVideoFileSize = fread(m_pBuffer, 1, m_iBufSize, fp);
             fclose(fp);
             if( m_iVideoFileSize<1 )
             {
                 GS_ASSERT(0);
             }
         }
         else
         {
             GS_ASSERT(0);

         }
     }
    else
    {
        m_iBufSize = KBYTES*125;
        m_pBuffer = (unsigned char *)malloc( m_iBufSize );
        if( !m_pBuffer )
        {
            m_iBufSize = 0;
        }
    }
}

CSenderObj::~CSenderObj(void)
{
    m_csTasker.Uninit();
    if(m_pBuffer ) 
    {
		::free(m_pBuffer);
        m_pBuffer = NULL;
    }
}

void CSenderObj::OnTaskEvent(CObjThreadPool *pTkPool,void *pTaskData)
{
INT iIndex = (INT)pTaskData;
    iIndex--;
    CGSWRMutexAutoRead alocker( &m_pDlg->m_csSrcWRMutex );
    CTestSrvDialog::CSourceMapIterator csIt;
    csIt = m_pDlg->m_csSourceMap.find( iIndex );
    if( csIt == m_pDlg->m_csSourceMap.end() || !(*csIt).second.bSendData )
    {
         return;
    }
    if( AtomicInterCompareExchange( (*csIt).second.bWrite, 0, 1 ) )
    {
        //保证只有一个人在写
        StruIOV struIOV[5];
        StruMySource *pNode = &((*csIt).second);

        if( s_bTestModFile )
        {
           //文件方式
            if( m_iVideoFileSize< 64 )
            {
                
               AtomicInterSet((*csIt).second.bWrite, 0);
                GS_ASSERT(0);
                return;
            }

            if( (pNode->iFReadPos+4) >= m_iVideoFileSize )
            {
                pNode->iFReadPos = 0;
            }

            INT32 iDataLen = 0;
            ::memcpy(&iDataLen,  m_pBuffer+pNode->iFReadPos, 4 );
            pNode->iFReadPos += 4;

            if( iDataLen > (m_iVideoFileSize-pNode->iFReadPos) )
            {
                AtomicInterSet((*csIt).second.bWrite, 0);                
                return;
            }

            struIOV[0].pBuffer = (void*) (m_pBuffer+pNode->iFReadPos); 
            struIOV[0].iSize = iDataLen;

			GS_ASSERT(iDataLen);

            pNode->iFReadPos += iDataLen;   

            pNode->pSource->WriteDataV( struIOV, 1, 0, TRUE );
            pNode->iSends += iDataLen;    

        }
        else
        {

            INT s = 0;
            INT iTotals = 0;
            pNode->iWIndex++;
            pNode->iRandData = rand();

            float fMax = m_iBufSize-1028;
            s = 1027 + (int) (fMax * (rand() / (RAND_MAX + 1.0)));

            MD5_CTX stCtx;
            MD5Init(&stCtx);
            MD5Update(&stCtx,(unsigned char *)&pNode->iWIndex, sizeof(INT32)); 
            struIOV[0].pBuffer = (void*) &pNode->iWIndex; 
            struIOV[0].iSize = sizeof(INT32);

            MD5Update(&stCtx,(unsigned char *)&pNode->iRandData, sizeof(INT32)); 
            struIOV[1].pBuffer = (void*) &pNode->iRandData; 
            struIOV[1].iSize = sizeof(INT32);

            MD5Update(&stCtx,(unsigned char *)m_pBuffer, s); 
            struIOV[2].pBuffer = (void*)m_pBuffer; 
            struIOV[2].iSize = s;

            MD5Final(pNode->md5res,&stCtx); 
            struIOV[3].pBuffer = pNode->md5res; 
            struIOV[3].iSize = MD5_LEN;

            iTotals = struIOV[0].iSize+struIOV[1].iSize+struIOV[2].iSize+struIOV[3].iSize;
            pNode->pSource->WriteDataV( struIOV, 4, 1, TRUE );
            pNode->iSends += iTotals;        
			GS_ASSERT(iTotals);
            
        }
    }
   AtomicInterSet((*csIt).second.bWrite, 0);
    
}

void CSenderObj::SendStep(void)
{
    CGSWRMutexAutoRead alocker( &m_pDlg->m_csSrcWRMutex );

    if( s_bTestModFile )
    {
        //文件方式
        UINT64 iTicks = DoGetTickCount();
        if( iTicks<m_iTicks )
        {
            m_iTicks = iTicks;
            return;
        }
        if( (iTicks-m_iTicks)<m_iInterval )
        {
             return;
        }
        m_iTicks = iTicks;
    }
    CTestSrvDialog::CSourceMapIterator csIt;
    if( m_csTasker.GetWaitTask() > (2*m_pDlg->m_csSourceMap.size()) )
    {
        return;
    }
    for( csIt=m_pDlg->m_csSourceMap.begin();csIt!=m_pDlg->m_csSourceMap.end(); csIt++ )
    {
        if( (*csIt).second.bSendData )
        {
            INT i = (*csIt).first+1;
        if( m_csTasker.Task( (void*)i) )
        {
            return;
        }
        }
    }
    
}


//----------------------------------------------------------------------------
// CTestSrvDialog
//----------------------------------------------------------------------------

// WDR: event table for CTestSrvDialog

BEGIN_EVENT_TABLE(CTestSrvDialog,wxDialog)
    EVT_BUTTON( ID_BUTTON_SELCFGFILE, CTestSrvDialog::OnSelCfgFileBtnClick )
    EVT_TOGGLEBUTTON( ID_TOGGLEBUTTON_SSSERVER, CTestSrvDialog::OnStarSrvBtnClick )
    EVT_BUTTON( ID_BUTTON_DEL_SRC, CTestSrvDialog::OnBtnDelSrcClick )
    EVT_TOGGLEBUTTON( ID_TOGGLEBUTTON_SRC_SENDDATA, CTestSrvDialog::OnBtnSrcSendDataClick )
    EVT_SPINCTRL( ID_SPINCTRL_SRC_REFINTERVAL, CTestSrvDialog::OnSpinSrcRefreshIntervalChanged )
    EVT_BUTTON( ID_BUTTON_ADD_SINGLE, CTestSrvDialog::OnBtnAddSingleClick )
    EVT_BUTTON( ID_BUTTON_ADD_LOTS, CTestSrvDialog::OnBtnAddSrcLotsClick )
    EVT_TOGGLEBUTTON( ID_TOGGLEBUTTON_AD_RAND, CTestSrvDialog::OnBtnADRandSrcClick )
    EVT_SPINCTRL( ID_SPINCTRL_AD_INTERVAL, CTestSrvDialog::OnSpinADIntervalChanged )
    EVT_SPINCTRL( ID_SPINCTRL_CLI_REFINTERVAL, CTestSrvDialog::OnSpinCliRefreshIntervalChanged )
    EVT_TIMER(REFRESH_SRC_INFO_TIMER, CTestSrvDialog::OnRefreshSrcInfoTimer)
    EVT_TIMER(REFRESH_CLIENT_INFO_TIMER, CTestSrvDialog::OnRefreshClientInfoTimer)
    EVT_SPINCTRL( ID_SPINCTRL_SINTERVAL, CTestSrvDialog::OnSendIntervalChanged )
    EVT_TIMER(REFRESH_SEND_DATA_TIMER, CTestSrvDialog::OnSendDataTimerEvent)
	EVT_CLOSE( CTestSrvDialog::OnDialogClose)
END_EVENT_TABLE()

CTestSrvDialog::CTestSrvDialog( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
    ,m_csRefSrcTimer(this,REFRESH_SRC_INFO_TIMER)
    ,m_csRefCliTimer(this,REFRESH_CLIENT_INFO_TIMER)
    ,m_csSender()
    ,m_csSendDataTimer(this, REFRESH_SEND_DATA_TIMER)
{
    // WDR: dialog function CreateTestServerDialog for CTestSrvDialog
    CreateTestServerDialog( this, TRUE ); 
    m_pSever = NULL;
    m_csSender.m_pDlg = this;
}

CTestSrvDialog::~CTestSrvDialog()
{
	m_csSender.m_csTasker.Uninit();
	if( m_pSever )
	{
		delete m_pSever;
		m_pSever = NULL;

	}
}

// WDR: handler implementations for CTestSrvDialog


void CTestSrvDialog::OnDialogClose( wxCloseEvent &event)
{
	delete this;
}


void CTestSrvDialog::OnSendIntervalChanged( wxSpinEvent &event )
{
    if( m_csSendDataTimer.IsRunning() && GetTogglebuttonSrcSenddata()->GetValue()  )
    {
        m_csSendDataTimer.Stop();
        m_csSendDataTimer.Start(GetSpinctrlSinterval()->GetValue() );
    }
}

EnumErrno CTestSrvDialog::OnServerSourceEvent(CIServer *pServer, 
                                                        CISource *pSource, 
                                                        EnumGSPServerEvent eEvent, 
                                                        void *pEventPararms)
{    
    INT iIndex = (INT)pSource->GetUserData();
    GS_ASSERT_RET_VAL(iIndex>-1, eERRNO_SYS_EINVALID);

    switch(eEvent)
    {
    case GSP_SRV_EVT_REF :
    case GSP_SRV_EVT_UNREF :
        {
         
            CGSWRMutexAutoRead alocker(&m_csSrcWRMutex);
            CSourceMapIterator csIt;
            csIt = m_csSourceMap.find((UINT16)iIndex);
            if( csIt!=m_csSourceMap.end() )
            {
                (*csIt).second.iRefs = pSource->GetSrcRefs();
            }
         
        }
        break;
    case GSP_SRV_EVT_CTRL :
        {
			StruGSPCmdCtrl *pCtrl;
			pCtrl = (StruGSPCmdCtrl*)pEventPararms;
            GS_ASSERT_RET_VAL(pCtrl, eERRNO_SYS_EINVALID);
           
            CGSWRMutexAutoRead alocker(&m_csSrcWRMutex);
            CSourceMapIterator csIt;
            csIt = m_csSourceMap.find((UINT16)iIndex);
            if( csIt!=m_csSourceMap.end() )
            {
                (*csIt).second.iRefs = pSource->GetSrcRefs();
                switch(pCtrl->iCtrlID )
                {
                case GSP_CTRL_PLAY :
                    (*csIt).second.bSendData = true;
                    break;
                case GSP_CTRL_STOP :
                    {
                        if( !pSource->GetSrcRefs() )
                        {
                            (*csIt).second.bSendData = false;
                        }
                    }
                break;
                default :
                    {
                        MY_DEBUG_PRINTF("Rcv CtrlID(0x%x) not handle.\n", (INT) pCtrl->iCtrlID );
                    }
                    break;
                }
            }
        }
        break;  
    default:
        {
           MY_DEBUG_PRINTF("On Server event:%d.\n", (int)eEvent);
        }

    }
    return eERRNO_SUCCESS;
/*    return  CISource::SRC_RET_EUNUSED;*/
}

void CTestSrvDialog::InitDialog()
{
    wxString strFilename = GSGetApplicationPath().c_str();
    if ( !strFilename.empty() )
    {
        // work with the file
        strFilename += "GSPSrvConf.ini";
        GetTextctrlCfgfile()->SetValue(strFilename);
    }
    wxGrid* p = GetGridSrc();
    p->SetLabelFont( g_pMainApp->m_pMainWindow->GetFont());
    p->SetRowLabelSize(30);
    p->SetLabelTextColour(wxColour(0,0,255));
    p->AppendCols(7);
    p->SetColLabelValue(0,wxT("Key"));
    p->SetColSize(0,200);
    p->SetColLabelValue(1,wxT("ID"));
    p->SetColLabelValue(2,wxT("Rcvs(Bytes)"));
     p->SetColSize(2,100);
    p->SetColLabelValue(3,wxT("Sends(Bytes)"));
     p->SetColSize(3,100);
    p->SetColLabelValue(4,wxT("SendData"));
    p->SetColLabelValue(5,wxT("Refs"));
    p->SetColLabelValue(6,wxT("Index"));
    p->HideCol(6);
    p->EnableEditing(false);


    p = GetGridClient();
    p->SetLabelFont( g_pMainApp->m_pMainWindow->GetFont());
    p->SetRowLabelSize(30);
    p->SetLabelTextColour(wxColour(0,0,255));
    p->AppendCols(6);
    p->SetColLabelValue(0,wxT("Client IP"));
    p->SetColSize(0,150);
    p->SetColLabelValue(1,wxT("URI"));
    p->SetColSize(1,300);
    p->SetColLabelValue(2,wxT("Rcvs(Bytes)"));
    p->SetColSize(2,70);
    p->SetColLabelValue(3,wxT("Sends(Bytes)"));
    p->SetColSize(3,70);
    p->SetColLabelValue(4,wxT("Losts"));
    p->SetColLabelValue(5,wxT("Pro"));
    p->EnableEditing(false);

    p = GetGridSrvSt();
    p->SetLabelFont( g_pMainApp->m_pMainWindow->GetFont());
    p->SetRowLabelSize(30);
    p->SetLabelTextColour(wxColour(0,0,255));
    p->AppendCols(3);

    p->SetColLabelValue(0,wxT("Pro"));
      p->SetColSize(0,40); 
    p->SetColLabelValue(1,wxT("Network"));
    p->SetColSize(1,140); 
    p->SetColLabelValue(2,wxT("Status"));
     p->SetColSize(2,70); 
     p->EnableEditing(false);

    EnableOtherFunction(false);
     
    
}

void CTestSrvDialog::UpdateClientInfo( bool bUpdate)
{

    int iRows;
    wxGrid* p = GetGridClient();
    iRows = p->GetRows();
    if(iRows>0 )
    {
        p->DeleteRows(0, iRows );
    }
    if( !bUpdate )
    {
        return;
    }

	char *pResult = NULL;
	INT iSize = 0;

    if ( !m_pSever->QueryStatus(GSP_SRV_STATUS_GET_CLIENTS_INFO, &pResult, &iSize) )
	{
		GS_ASSERT(0);
		return; 
	}
    wxString strValue;
	StruClientVector *pVClient = (StruClientVector *)pResult;

    for( iRows = 0; iRows<pVClient->iNums; iRows++ )
    {
         p->AppendRows();
         strValue = pVClient->vClient[iRows].szRemoteIP;
         p->SetCellValue(iRows,0,strValue); 

         strValue =  pVClient->vClient[iRows].szSourceKey;
         p->SetCellValue(iRows,1,strValue);

         strValue = wxString::Format("%lld", pVClient->vClient[iRows].iRecv );
         p->SetCellValue(iRows,2,strValue);

         strValue = wxString::Format("%lld", pVClient->vClient[iRows].iSend );
         p->SetCellValue(iRows,3,strValue);

         strValue = wxString::Format("%ld",  pVClient->vClient[iRows].iLostFrames );
         p->SetCellValue(iRows,4,strValue);

		 strValue = GetProtocolName( pVClient->vClient[iRows].eProtocol );     
         p->SetCellValue(iRows,5,strValue);
    }
	m_pSever->FreeQueryStatusResult(pResult);

}

void CTestSrvDialog::UpdateModuleInfo( const StruProtocolService *pList )
{
    int iRows;
    wxGrid* p = GetGridSrvSt();
    iRows = p->GetRows();
    if(iRows>0 )
    {
        p->DeleteRows(0, iRows );
    }
    

    wxString strValue;
    for( iRows = 0;iRows<pList->iNums; iRows++  )
    {
        p->AppendRows();
        strValue = pList->vService[iRows].czProtocol;
        p->SetCellValue(iRows,0,strValue);

		strValue = wxString::Format("%s:%d",
			pList->vService[iRows].czSrvBindIP,
			pList->vService[iRows].iSrvBindPort );      
        p->SetCellValue(iRows,1,strValue);

        strValue = "运行中";
        p->SetCellValue(iRows,2,strValue);
    }


    
}

void CTestSrvDialog::UpdateSourceInfo(void)
{
    int iRows;
    wxGrid* p = GetGridSrc();
    iRows = p->GetRows();
    if(iRows>0 )
    {
        p->DeleteRows(0, iRows );
    }
//     else
//     {
//         if( m_csRefSrcTimer.IsRunning() && !GetTogglebuttonSsserver()->GetValue() )
//         {
//             m_csRefSrcTimer.Stop();
//         }
//     }
    iRows = 0;
    std::map<UINT16, StruMySource>::iterator csIt;
    wxString strValue;
    CGSWRMutexAutoRead aLocker( &m_csSrcWRMutex);
    p->Freeze();
    for( csIt = m_csSourceMap.begin(); csIt!=m_csSourceMap.end(); csIt++, iRows++  )
    {
        p->AppendRows();
        //key
        strValue = (*csIt).second.pSource->GetKey();
        p->SetCellValue(iRows,0,strValue);

        //ID
        strValue = wxString::Format("%d", (*csIt).second.pSource->GetSourceID() );
        p->SetCellValue(iRows,1,strValue);

        //Rcvs
        strValue = wxString::Format("%lld", (*csIt).second.iRcvs );
        p->SetCellValue(iRows,2,strValue);

        //Sends 
        strValue = wxString::Format("%lld", (*csIt).second.iSends );
        p->SetCellValue(iRows,3,strValue);

        //bSend
        strValue = (*csIt).second.bSendData ? "Yes" : "No";
        p->SetCellValue(iRows,4,strValue);

        //Refs
        strValue = wxString::Format("%d:%d",(*csIt).second.iRefs, (*csIt).second.pSource->GetSrcRefs() );
        p->SetCellValue(iRows,5,strValue); 
        
        //内部使用
        strValue = wxString::Format("%d", (*csIt).first );
        p->SetCellValue(iRows,6, strValue); 

    }
    p->Thaw();
}

void CTestSrvDialog::EnableOtherFunction( bool bEnable )
{
    if( bEnable )
    {
        m_csRefCliTimer.Start(GetSpinctrlCliRefinterval()->GetValue()*1000+1);
        m_csRefSrcTimer.Start(GetSpinctrlSrcRefinterval()->GetValue()*1000+1);
    }
    else
    {
       
        m_csRefSrcTimer.Stop();
        m_csRefCliTimer.Stop();
    }

    GetButtonAddSingle()->Enable(bEnable);
    GetButtonAddLots()->Enable(bEnable);
    GetTogglebuttonAdRand()->Enable(bEnable);  
    GetTogglebuttonSrcSenddata()->Enable(bEnable);
    GetButtonDelSrc()->Enable(bEnable);
  //  ClearSource();
    UpdateSourceInfo();
     UpdateClientInfo(false);

}


void CTestSrvDialog::OnSpinCliRefreshIntervalChanged( wxSpinEvent &event )
{
    
}

void CTestSrvDialog::OnSpinADIntervalChanged( wxSpinEvent &event )
{
    
}

void CTestSrvDialog::OnBtnADRandSrcClick( wxCommandEvent &event )
{
    
}


BOOL CTestSrvDialog::AddSource( UINT16 iIndex)
{
   
    wxString strKey;
    strKey = wxString::Format("test_%d", iIndex);
	m_csSrcWRMutex.LockWrite();
    if( m_csSourceMap.find( iIndex) != m_csSourceMap.end() )
    {
		m_csSrcWRMutex.UnlockWrite();
        //已经存在
        strKey += _GSTX(" 数据源已经存在");
        wxMessageBox(strKey, wxT(_GSTX("Warning"))  );
        return TRUE;

    }
	m_csSrcWRMutex.UnlockWrite();
		
        CISource *pSource = m_pSever->AddSource(strKey.c_str() ); 
		
        StruMySource stMySource;
        if( pSource )
        {
             StruGSMediaDescri descri;

//             bzero( &descri, sizeof(StruGSMediaDescri));
//             descri.eMediaType = GS_MEDIA_TYPE_SYSHEADER;
//             descri.unDescri.struBinary.iSize = 1024;
//             pSource->SetMediaInfo(0, &descri);

            bzero( &descri, sizeof(StruGSMediaDescri));
            descri.eMediaType = GS_MEDIA_TYPE_VIDEO;
            descri.unDescri.struVideo.eCodeID = GS_CODEID_ST_MP4;
            descri.unDescri.struVideo.iFrameRate = 12;
            descri.unDescri.struVideo.iFrameRate2 = 5;
            descri.unDescri.struVideo.iHeight = 320;
            descri.unDescri.struVideo.iWidth = 240; 
            pSource->SetMediaInfo(0, &descri );

//             bzero( &descri, sizeof(StruGSMediaDescri));
//             descri.eMediaType = GS_MEDIA_TYPE_AUDIO;
//             descri.unDescri.struAudio.eCodeID = GS_CODEID_ST_MP3;
//             descri.unDescri.struAudio.iBits = 8;
//             descri.unDescri.struAudio.iChannels =1;
//             descri.unDescri.struAudio.iSample= 16000;
//             pSource->SetMediaInfo(1, &descri );



            pSource->SourceEnableValid(TRUE);      
            pSource->SetUserData((void*)(INT)iIndex);
            stMySource.pSource = pSource;
			CGSWRMutexAutoWrite alocker( &m_csSrcWRMutex);
            m_csSourceMap.insert( std::make_pair(iIndex, stMySource) );
            return TRUE;

        }
    return FALSE;
}



void CTestSrvDialog::OnBtnAddSingleClick( wxCommandEvent &event )
{
    //增加一个数据源
INT iIndex = GetSpinctrlSrcidxStart()->GetValue();
    if( AddSource((UINT16) iIndex) )
    {
        GetSpinctrlSrcidxStart()->SetValue(iIndex+1);
    }
    UpdateSourceInfo();
    
}

void CTestSrvDialog::OnBtnAddSrcLotsClick( wxCommandEvent &event )
{
    //增加多为个数据源
    INT iBegin = GetSpinctrlSrcidxStart()->GetValue();
    INT iEnd = iBegin+GetSpinctrlSrcidEnd()->GetValue();
    
    for( INT iIndex = iBegin; iIndex<iEnd; iIndex++)
    { 
        AddSource((UINT16) iIndex );        
    }
    GetSpinctrlSrcidxStart()->SetValue(iEnd);
    UpdateSourceInfo();
}


void CTestSrvDialog::OnSpinSrcRefreshIntervalChanged( wxSpinEvent &event )
{

}

void CTestSrvDialog::OnBtnSrcSendDataClick( wxCommandEvent &event )
{
    if( GetTogglebuttonSrcSenddata()->GetValue() )
    {
        m_csSendDataTimer.Start(GetSpinctrlSinterval()->GetValue() );
       // GetTogglebuttonSrcSenddata()->SetLabel(wxT(_GSPT("Stop Send")));
    }
    else
    {
        m_csSendDataTimer.Stop();
    }
}

void CTestSrvDialog::ClearSource( INT iIndex )
{
   //清除数据源
   std::map<UINT16, StruMySource>::iterator csIt;
   CISource *pSrc =  NULL;
   if( iIndex>-1 )
   {
       m_csSrcWRMutex.LockWrite();
       csIt = m_csSourceMap.find( (UINT16) iIndex );
       if( csIt != m_csSourceMap.end() )
       {
           pSrc = (*csIt).second.pSource;
           m_csSourceMap.erase( csIt);
           m_csSrcWRMutex.UnlockWrite();
           pSrc->Release();
       } 
       else
       {
           m_csSrcWRMutex.UnlockWrite();
       }
   }
   else
   {
       m_csSrcWRMutex.LockWrite();
       while(1){
           csIt = m_csSourceMap.begin();
           if( csIt == m_csSourceMap.end() )
           {
               break;
           }
           pSrc = (*csIt).second.pSource;
           m_csSourceMap.erase( csIt);          
           m_csSrcWRMutex.UnlockWrite();
           pSrc->Release();
           m_csSrcWRMutex.LockWrite();
       }
        m_csSrcWRMutex.UnlockWrite();
   }
}

void CTestSrvDialog::OnBtnDelSrcClick( wxCommandEvent &event )
{
    //删除数据源                             
    wxGrid* p = GetGridSrc();
    wxArrayInt vRows;

    CMainApp::GetGridSelectRows(p, vRows);

    for( UINT i = 0; i<vRows.GetCount(); i++)
    {          
        wxString strVal;
        strVal = p->GetCellValue(vRows[i], 6);
        long iIndex = -1;
        strVal.ToLong(&iIndex);
        if( iIndex>-1)
        {
            ClearSource( iIndex );
        }
    }
    if( vRows.GetCount() )
    {
        UpdateSourceInfo();
    }

}



void CTestSrvDialog::OnStarSrvBtnClick( wxCommandEvent &event )
{
    //开启停止服务
   
     if( m_pSever )
     {
        // ClearSource(-1);
         delete m_pSever;
         m_pSever = NULL;
		 m_csSrcWRMutex.LockWrite();
		 m_csSourceMap.clear();
		 m_csSrcWRMutex.UnlockWrite();
         EnableOtherFunction(false);
         GetTogglebuttonSsserver()->SetLabel(wxT("Start Server"));         
     }
     else
     {
         m_pSever =  CreateGSPServerInterface();
		 m_pSever->SetEventCallback( (GSPServerEventFunctionPtr) OnSourceEvent, this);
         wxString strCfgFilename = GetTextctrlCfgfile()->GetValue();

         if( m_pSever->Init(strCfgFilename.c_str(), NULL) )
         {
             GetTogglebuttonSsserver()->SetLabel(wxT("Stop Server"));
			  StruProtocolService *pList = NULL;
			  INT iSize = 0;
			  if( m_pSever->QueryStatus(GSP_SRV_STATUS_SUPPORT_PROTOCOL, 
				  (char **)&pList, &iSize) )
			  {
				  UpdateModuleInfo(pList);
				  m_pSever->FreeQueryStatusResult((char*)pList);
			  }
             EnableOtherFunction(true);

         }
         else
         {
             delete m_pSever;
             m_pSever = NULL;
             wxMessageBox( "Init GSS Server fail.", "Error");
         }


     }
     
}

void CTestSrvDialog::OnSelCfgFileBtnClick( wxCommandEvent &event )
{
    //选择配置文件
    wxString strDefPath = GSGetApplicationPath();
    wxString strFilename = wxFileSelector("Choose server config file", strDefPath);
    if ( !strFilename.empty() )
    {
        // work with the file
       GetTextctrlCfgfile()->SetValue(strFilename);
    }
    

}

void CTestSrvDialog::OnRefreshSrcInfoTimer( wxTimerEvent& event)
{
    //刷新数据源信息定时器
    if( GetCheckboxSrcRefresh()->IsChecked() )
    {
      UpdateSourceInfo();
    }
}

void CTestSrvDialog::OnRefreshClientInfoTimer( wxTimerEvent& event)
{
     //刷新客户端信息定时器
    if( GetCheckboxCliRefresh()->IsChecked() )
    {
        UpdateClientInfo();
    }
}

void CTestSrvDialog::OnSendDataTimerEvent( wxTimerEvent& event)
{
   //发送数据
    if( GetTogglebuttonSrcSenddata()->GetValue() )
    {
        m_csSender.SendStep();
    }
}




