#include <time.h>
#include "IServer.h"
#include "MainLoop.h"
#include "cmdline.h"
#include "List.h"
#include "BTree.h"
#include "md5.h"
#include "Log.h"
#include "Rtp/RtpStru.h"
#include "ISocket.h"


using namespace GSP;    
using namespace GSP::RTP;

static CIServer *s_pSever = NULL;
static CBTree s_csSourceList(BTreeUINT32Cmp);


#define _BUFFER_SIZE (KBYTES*64+34)

class CMakeRandBytes
{
public :
    INT m_iFrameRate;
    INT m_iByteRate; 
    INT m_iFrameCounts;
    double m_fPer[100];
    CMakeRandBytes(INT iFrameRate = 25, UINT iBitRate = 512 ) // iBitRate 单位 KBytes
        : m_iFrameRate(iFrameRate)
        ,m_iByteRate(iBitRate*1024L)
        ,m_iFrameCounts(0)        
    {
        if(iFrameRate>100)
        {
            iFrameRate = 100;
        }
        MakePer();
    }
    ~CMakeRandBytes(void)
    {

    }

    void MakePer(void)
    {
#define MIN_PER 0.01

        m_fPer[0] =  0.01;
        for( int i  = 1; i<100; i++ )
        {
           m_fPer[i] =  m_fPer[i-1]+0.01;
        }

      /*  double x;             
        double fTotal=0.0;
        double fLeft;
        int i;
        for( i = 0; i<(m_iFrameCounts-1); i++ )
        {

            fLeft = 1.0-fTotal-(m_iFrameRate-i)*MIN_PER;
            do 
            {
                x =   MIN_PER+(rand() / (RAND_MAX + 1.0));
            } while(x>fLeft || x>_BUFFER_SIZE );
            m_fPer[i] = x;
            fTotal += m_fPer[i];
        }
        m_fPer[i] = 1.0-fTotal;*/
    }

    UINT GetSize(void)
    {   

	//	return KBYTES*2;
     
        double iRet;
        iRet = m_fPer[m_iFrameCounts]*_BUFFER_SIZE; //m_iByteRate;
        m_iFrameCounts++;
        if(m_iFrameCounts >=100 /*m_iFrameCounts==m_iFrameRate*/)
        {
            m_iFrameCounts = 0;
            MakePer();
        }
        UINT iSize =  (UINT)iRet+MD5_LEN+77+sizeof(StruGSFrameHeader);
        if( iSize > _BUFFER_SIZE )
        {
            return _BUFFER_SIZE;
        }
        return iSize;
        
    }

};

class CSendData;

static void *SendThread(CSendData *csSender);

class CSendData : public CGSPObject
{
  
public :

#define SEND_EXIT 0
#define SEND_STOP  1
#define SEND_DOING 2
#define SEND_START  3
#define SEND_WAIT  4

    BOOL m_bStart; 
    FILE *m_fp;
    unsigned char  *m_pBuffer;
    CMakeRandBytes m_csRander;
    UINT64 m_iSendTotal;
    UINT64 m_iLastSend;
    UINT64 m_iLastTv;
    UINT32 m_iIndex;
    BOOL m_bModeFile;
    CISource *m_pSource;
	INT     m_iSendStatus;
	CGSMutex m_csSendSync;
	CGSMutex m_csMutex;

    CSendData(void) : 
    CGSPObject()      
        ,m_bStart(FALSE)
        ,m_fp(NULL)        
        ,m_csRander(25, 30)
        ,m_iSendTotal(0)
        ,m_iLastSend(0)
        ,m_iLastTv(0)
        ,m_iIndex(0)
        ,m_bModeFile(FALSE)
        ,m_pSource(NULL)
		,m_iSendStatus(0)
		,m_csSendSync()
		,m_csMutex()
    {
          
       
        m_pBuffer =(unsigned char  *) malloc( _BUFFER_SIZE );
        GS_ASSERT_RET( m_pBuffer!=NULL );

    }

    virtual ~CSendData(void)
    {
		m_csMutex.Lock();
		m_bStart = FALSE;
		m_csMutex.Unlock();

		m_csSendSync.Lock();
		if( m_iSendStatus!=SEND_EXIT )
		{
			GS_ASSERT(0);
		} 
		m_csSendSync.Unlock();
        
        if(m_fp )
        {
            fclose(m_fp);
        }

    }
    void OnSendTick( void )
    {
// 		static  int  inumbs = 0;
// 
// 		if( inumbs>0  )
// 		{
// 			return;
// 		}
// 		inumbs++;


        if( !m_bStart || !m_pSource )
        {
            return;
        }
        INT l;
        INT s;
        l = m_csRander.GetSize();
        if( l>_BUFFER_SIZE)
        {
            l =  _BUFFER_SIZE;
        }
        s = l;

        if( m_bModeFile )
        {
            if( !m_fp )
            {
                MY_PRINTF( "Source %s trans start. Send file complete.\n", m_pSource->GetKey() );
				m_csMutex.Lock();				
				m_bStart = FALSE;
				m_csMutex.Unlock();
                
            }
            
           
            s = fread(m_pBuffer, 1, l ,m_fp );
            if( s<=0 )
            {
                if( !feof(m_fp) )
                {
                    MY_PRINTF("Read fail.\n");
                }
                fclose( m_fp);
                m_fp = NULL;
				m_csMutex.Lock();				
				m_bStart = FALSE;
				m_csMutex.Unlock();
                MY_PRINTF( "Source %s trans start. Send file complete.\n", m_pSource->GetKey() );
                return;
            }

        } 
		else 
        {                
            m_iIndex ++;         
            memcpy( m_pBuffer, &m_iIndex, sizeof(UINT32) );          
            sprintf_s( (char*)(m_pBuffer+sizeof(UINT32)),s-4-MD5_LEN, "Size:%d, Index:%d\n", s, m_iIndex );
            unsigned char md5res[MD5_LEN];
            MD5Sum(m_pBuffer,s-MD5_LEN, md5res);
            memcpy( m_pBuffer+(s-MD5_LEN), md5res, MD5_LEN);
        }
       //  m_pBuffer[s-MD5_LEN-5] = (unsigned char )m_iIndex;

       // m_pSource->WriteData(m_pBuffer,s, 1, TRUE);

        StruIOV struIOV[2];
        struIOV[0].pBuffer = m_pBuffer; 
        struIOV[0].iSize = sizeof(StruGSFrameHeader);
        struIOV[1].pBuffer = (void*) (m_pBuffer+sizeof(StruGSFrameHeader)); 
        struIOV[1].iSize = s-sizeof(StruGSFrameHeader);

        m_pSource->WriteDataV( struIOV, 2, 1, TRUE );
       

        m_iSendTotal += s;


    }

    void Start(BOOL bFile = FALSE)
    {
        if(m_bStart )
        {
            return;
        }
		CGSAutoMutex locker(&m_csMutex);
		if( m_bStart )
		{
			return;
		}
        if(bFile)
        {
            if( !m_fp )
            {
                m_fp = fopen("D:/Develop/C3M-Video/SourceCodes/Common/GSPSDK/Debug/1.rmvb", "rb");   
                if( !m_fp )
                {
                    MY_WARNING("Open 1.rmvb %s fail.\n",
                        strerror(errno));
                } else
                {
                    m_iSendTotal = 0;
                    m_iLastSend = 0;

                }
            }
            m_bModeFile  = TRUE;
        }
        else
        {
            m_bModeFile  = FALSE;
        }  

	     m_bStart = TRUE;
		 m_iSendStatus = SEND_START;
		 
		 if( !TestCreateMyThread((void*(*)(void*))SendThread, this))
		 {
			 m_iSendStatus = SEND_EXIT;
			 m_bStart = FALSE;
			 GS_ASSERT(0);
		 }
         MY_PRINTF("Source %s Trans  Start mode %s.\n", m_pSource->GetKey(), m_bModeFile ? "File Data" : "Rand Data" );
    }
    void Stop(void)     
    {
		if( !m_bStart )
		{
			return;
		}
		m_csMutex.Lock();
		if( !m_bStart )
		{
			m_csMutex.Unlock();
			return;
		}
		m_bStart = FALSE;
		m_csMutex.Unlock();
		m_csSendSync.Lock();
		m_csSendSync.Unlock();
		GS_ASSERT( m_iSendStatus==SEND_EXIT);
        if( m_fp )
        {
          fclose(m_fp);
          m_fp = NULL;
        }  
        if( m_pSource )
        {   
            MY_PRINTF("Source %s Trans  Stop.\n", m_pSource->GetKey() );
        }
    }

    void PrintStatus(void)
    {

        UINT64 iCur = DoGetTickCount();
        UINT64 iTotalSend = m_iSendTotal;

        INT iISend;           
        INT iITv;
        double iTemp;
        double  fRate;
        iISend =(INT)(iTotalSend-m_iLastSend );
        iITv = (INT)(iCur-m_iLastTv);

        

        iTemp = (iITv/1000);
        if( iTemp==0 )
        {
            iTemp = 1;
        }

        fRate = ((iISend/1024.00)/iTemp);

        MY_PRINTF("%s In %d MSec Send %lld-%lld = %d    %.3f KB/Sec.\n", 
            m_pSource ? m_pSource->GetKey() : "   ", 
            iITv,
            iTotalSend,m_iLastSend, iISend, fRate );
        m_iLastTv  = iCur;
        m_iLastSend  = iTotalSend;

    }
};

static void *SendThread(CSendData *csSender)
{
	CGSAutoMutex locker(&csSender->m_csSendSync);
	csSender->m_iSendStatus = SEND_DOING;
	while(csSender->m_bStart )
	{
		csSender->OnSendTick();
		MSLEEP(500);
	}
	csSender->m_iSendStatus = SEND_EXIT;
	return 0;
}

class CMySource : public CGSPObject
{

public :
    CSendData m_csSender;
    CISource *m_pSource;

    CMySource( CISource *pSource )
        :CGSPObject()
        ,m_csSender()
        ,m_pSource(pSource)
    {
        m_csSender.m_pSource = pSource;
    }

    virtual ~CMySource(void)
    {
        
        m_csSender.Stop();
        m_csSender.m_pSource = NULL;
        if( m_pSource )
        {
            MY_PRINTF("%s Release.\n", m_pSource->GetKey() );           
            m_pSource->Release();
            m_pSource = NULL;
        }
       
    }
};


static void _FreeMySourceObject(CMySource *pObject)
{
    
    if( pObject )
    {
       
        delete pObject;
    }
}




static void *OnServerEvent(CIServer *pServer, CISource *pChannel,EnumGSPServerEvent eEvent, 
                          void *pEventPararms, INT iLen,  void *pParamData )
{
    CMySource *pMySource = (CMySource*)pChannel->GetUserData();
    GS_ASSERT_RET_VAL(pMySource!=NULL, 0);

    switch(eEvent)
    {
    case GSP_SRV_EVT_REF :
        {
            MY_PRINTF("Source(%s) Ref(%d).\n", pChannel->GetKey(), pChannel->GetSrcRefs() );
        }
        break;
    case GSP_SRV_EVT_UNREF :
        {
            MY_PRINTF("Source(%s) Unref(%d).\n", pChannel->GetKey(), pChannel->GetSrcRefs() );
            if( pChannel->GetSrcRefs()==0 )
            {
                pMySource->m_csSender.Stop();
            }
        }
        break;
    case GSP_SRV_EVT_CTRL :
        {
            StruGSPCmdCtrl *pCmd;
            pCmd = (StruGSPCmdCtrl*)pEventPararms;
            switch(pCmd->iCtrlID )
            {
            case GSP_CTRL_PLAY :
                MY_PRINTF("Source(%s) Play.\n", pChannel->GetKey());

                if( !pMySource->m_csSender.m_bStart )
                {
                    pMySource->m_csSender.Start();
                }
                break;
            default :
                {
                    MY_PRINTF("Rcv CtrlID(%d) \n", pCmd->iCtrlID );
                }
                break;
            }
        }
        break;
    default:
        {
            MY_PRINTF("On Server event:%d.\n", (int)eEvent);
        }

    }
	return 0;

}



static int _OnStartTest(const char *czCmd,const char *args)
{
    //开始服务器
INT iPort = 12345;
    if( s_pSever )
    {
        MY_PRINTF("Server was started.\n");
        return 0;

    }

    char *pParser;
    pParser = ArgsGetParser(args,NULL);
    if( pParser )
    {
        char *czKey =NULL, *czValue;
        int ret;
        while( (ret=ParserFetch(&pParser, &czKey, &czValue ) )==1 )
        {
            if( 0==strcmp("p", czKey ) )
            {
                //添加的个数
                if( !czValue )
                {
                    return -1;
                }
                iPort = atoi(czValue);
            }           
        }
    }

	 std::string strAppPath = GSGetApplicationPath();


    s_pSever = CreateGSPServerInterface();
    s_csSourceList.SetFreeFunction(NULL, (FuncPtrFree)_FreeMySourceObject );  
    s_pSever->SetEventCallback(OnServerEvent, NULL);
	std::string strTemp = strAppPath;
	strTemp += "GSPServer";
    s_pSever->InitLog(strTemp.c_str());   

	std::string strTemp2 = strAppPath;
	strTemp2 += "GSPSrvConf.ini";
    s_pSever->Init(strTemp2.c_str(), strTemp.c_str() ); 
    MY_PRINTF("Server start...\n");
    return 0;
}



static int _OnAddSourceTest(const char *czCmd,const char *args)
{
    // 添加数据源
    if( !s_pSever )
    {
        MY_PRINTF("Server not start.\n");
        return 0; 
    }



    

    char czSourceKey[56] = "Test00";
    int iCnts = 1;
    INT iBegin = 0;

    char *pParser;
    pParser = ArgsGetParser(args,NULL);
    if( pParser )
    {
        char *czKey =NULL, *czValue;
        int ret;
        while( (ret=ParserFetch(&pParser, &czKey, &czValue ) )==1 )
        {
            if( 0==strcmp("c", czKey ) )
            {
                //添加的个数
                if( !czValue )
                {
                    return -1;
                }
                iCnts = atoi(czValue);
            }
            else if( 0==strcmp("s", czKey ) )
            {
                //数据源索引开始
                if( !czValue )
                {
                    return -1;
                }
                iBegin = atoi(czValue);
            }

        }
    }

    CISource *pSource;
    CMySource *pMySource;

    StruGSMediaDescri descri;
    MY_DEBUG("Add source :%d->%d \n", iBegin, iBegin+iCnts );
    for( UINT32 i = iBegin; (INT)i< (iBegin+iCnts ); i++ )
    {
        sprintf_s( czSourceKey,56, "test_%d",  i);
        if( s_csSourceList.Find((void*)i).Second() )
        {
            //已经存在
            MY_WARNING("Source %s had exist.\n", czSourceKey);
            continue;
        }

        pSource = s_pSever->AddSource(czSourceKey);       
        if( pSource )
        {
            bzero( &descri, sizeof(StruGSMediaDescri));
            descri.eMediaType = GS_MEDIA_TYPE_SYSHEADER;
            descri.unDescri.struBinary.iSize = 1024;
            pSource->SetMediaInfo(0, &descri );

            bzero( &descri, sizeof(StruGSMediaDescri));
            descri.eMediaType = GS_MEDIA_TYPE_VIDEO;
            descri.unDescri.struVideo.eCodeID = GS_CODEID_GS_V461C;
            descri.unDescri.struVideo.iFrameRate = 12;
            descri.unDescri.struVideo.iFrameRate2 = 5;
            descri.unDescri.struVideo.iHeight = 320;
            descri.unDescri.struVideo.iWidth = 240; 
            pSource->SetMediaInfo(1, &descri );

            bzero( &descri, sizeof(StruGSMediaDescri));
            descri.eMediaType = GS_MEDIA_TYPE_AUDIO;
            descri.unDescri.struAudio.eCodeID = GS_CODEID_ST_MP3;
            descri.unDescri.struAudio.iBits = 8;
            descri.unDescri.struAudio.iChannels =1;
            descri.unDescri.struAudio.iSample= 16000;
            pSource->SetMediaInfo(2, &descri );

            CIUri *pUri = pSource->MakeURI();                   
            MY_PRINTF("Add source: URI(%s) Key(%s) OK.\n", pUri->GetURI(), pSource->GetKey()  );
			delete pUri;
            pSource->SourceEnableValid(TRUE);
            pSource->SourceEnableRef(TRUE);

            pMySource = new CMySource(pSource);

            pSource->SetUserData(pMySource);

            s_csSourceList.Insert( (void*) i, pMySource );
        }
        
    }
    return 0;
}



static int _OnRemoveSourceTest(const char *czCmd,const char *args)
{

    //删除数据源
    if( !s_pSever )
    {
        MY_PRINTF("Server not start.\n");
        return -1; 
    }
    char *pParser;
    INT iBegin = -1;
    INT iEnd = -1;


    pParser = ArgsGetParser(args,NULL);
    if( pParser )
    {
        char *czKey =NULL, *czValue;
        int ret;
        while( (ret=ParserFetch(&pParser, &czKey, &czValue ) )==1 )
        {
            //结束索引号
            if( 0==strcmp("e", czKey ) )
            {
                if( !czValue )
                {
                    return -1;
                }
                iEnd = atoi(czValue);
            }
            else if( 0==strcmp("s", czKey ) )
            {
                //开始索引号
                if( !czValue )
                {
                    return -1;
                }
                iBegin = atoi(czValue);
            }

        }
    }
    


    if( iBegin<0 && iEnd<0 )
    {
        MY_PRINTF("Remove source: All source.\n" );
        s_csSourceList.Clear();
    }
    else
    {
        CBTree::CIterator csIt;
        if( iBegin>=0 && iEnd <0 )
        {

            MY_PRINTF("Remove source: Key(test_%d --> end.\n", iBegin);
            s_csSourceList.Lock(TRUE);
            csIt = s_csSourceList.Travel(CBTree::OUT_ORDER);
            if( csIt.Second() )
            {
                iBegin = (UINT32)csIt.First();
            } 
            else
            {
                //已经没有对象
                s_csSourceList.Unlock(TRUE);
                return 0;
            }
            s_csSourceList.Unlock(TRUE);
        }
        else
        {
            MY_PRINTF("Remove source: Key(test_%d -->test_%d.\n", iBegin, iEnd);
        }
        UINT32 i;
        for( i=0; (INT)i<iEnd; i++  )
        {
            s_csSourceList.Erase((void*)i);
        }
    }
    return 0;
}



static int _OnTransStreamTest(const char *czCmd,const char *args)
{
    //开始、停止流传输
    char *pParser;
    INT iBegin = -1;
    INT iEnd = -1;

    BOOL bStart = TRUE;
    BOOL bFile = FALSE;

    pParser = ArgsGetParser(args,NULL);
    if( pParser )
    {
        char *czKey =NULL, *czValue;
        int ret;
        while( (ret=ParserFetch(&pParser, &czKey, &czValue ) )==1 )
        {
            if( 0==strcmp("e", czKey ) )
            {
                //数据源开始索引
                if( !czValue )
                {
                    return -1;
                }
                iEnd = atoi(czValue);
            }
            else if( 0==strcmp("s", czKey ) )
            {
                //数据源结束索引
                if( !czValue )
                {
                    return -1;
                }
                iBegin = atoi(czValue);
            } 
            else if( 0==strcmp("o", czKey) )
            {
                //开始/结束
                if( czValue )
                {
                    bStart  = atoi(czValue);
                }
            }
            else if( 0==strcmp("f", czKey) )
            {
                //文件传输方式
                bFile = TRUE;
            }


        }
    }



    CBTree::CIterator csIt;
    CMySource *pMySource;
    if( iBegin<0 && iEnd<0 )
    {
        MY_PRINTF("Start source: All source.\n" );               
        s_csSourceList.Lock(TRUE);
        for(csIt = s_csSourceList.Travel(); csIt.Second(); csIt++ )
        {
            pMySource = (CMySource*)csIt.First();
            if( bStart )
            {
                pMySource->m_csSender.Start(bFile);
            }
            else
            {
                pMySource->m_csSender.Stop();
            }
        }
        s_csSourceList.Unlock(TRUE);
    }
    else
    {
        if( iBegin<iEnd  )
        {
            //非法参数
            if( iEnd<0 )
            {
                return -1;
            }
            iBegin = 0;
        }

        UINT32 iKey;
        if( iBegin>=0 && iEnd <0 )
        {

            MY_PRINTF("Start source: Key(test_%d --> end.\n", iBegin);
            s_csSourceList.Lock(TRUE);
            csIt = s_csSourceList.Travel(CBTree::OUT_ORDER);
            if( csIt.Second() )
            {
                iBegin = (UINT32)csIt.First();
            } 
            else
            {
                //已经没有对象
                s_csSourceList.Unlock(TRUE);
                return 0;
            }
            s_csSourceList.Unlock(TRUE);
        }
        else
        {
            MY_PRINTF("Start source: Key(test_%d -->test_%d.\n", iBegin, iEnd);
        }
        s_csSourceList.Lock(TRUE);
        for(csIt = s_csSourceList.Travel(); csIt.Second(); csIt++ )
        {
            iKey = (UINT32)csIt.Key();
            if( (INT)iKey< iBegin || (INT)iKey>iEnd)
            {
                continue;
            }
            pMySource = (CMySource*)csIt.First();
            if( bStart )
            {
                pMySource->m_csSender.Start();
            }
            else
            {
                pMySource->m_csSender.Stop();
            }
        }
        s_csSourceList.Unlock(TRUE);
    }
    return 0;

}

static int _OnShowThreadStatusTest(const char *czCmd,const char *args)
{
    //打印状态
//     MY_PRINTF("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
//     MY_PRINTF("ThreadPool Max:%d Unuse:%d Exist:%d\n", 
//         CThreadPool::GetMaxGlobalUnuseThreads(), 
//         CThreadPool::GetGlobalUnuseThreads(),
//         CThreadPool::GetGlobalThreads() );
//      MY_PRINTF("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    return 0;
}


static int _OnShowStatusTest(const char *czCmd,const char *args)
{
    //打印状态
    if( !s_pSever )
    {
        MY_PRINTF("Server not start.\n");
        return 0; 
    }
    CBTree::CIterator csIt;
    CMySource *pMySource;
    MY_PRINTF("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
    
    s_csSourceList.Lock(TRUE);
    for(csIt = s_csSourceList.Travel(); csIt.Second(); csIt++ )
    {
        pMySource = (CMySource*)csIt.First();
        pMySource->m_csSender.PrintStatus();

    }
    s_csSourceList.Unlock(TRUE);

    MY_PRINTF("\n");

//    MY_PRINTF( "%s\n", s_pSever->GetStatusDescri() );

    MY_PRINTF("\n");


//     MY_PRINTF("ThreadPool Max:%d Unuse:%d Exist:%d\n", 
//         CThreadPool::GetMaxGlobalUnuseThreads(), 
//         CThreadPool::GetGlobalUnuseThreads(),
//         CThreadPool::GetGlobalThreads() );

    MY_PRINTF("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    return 0;
}



static int _OnClearTest(const char *czCmd,const char *args)
{
    //清除资源
    s_csSourceList.Clear();
    if( s_pSever )
    {
        s_pSever->Stop();
        delete s_pSever;
        s_pSever = NULL;
    }
    MY_PRINTF("Clean server source ok.\n");
    return 0;
}

static int _AddSource( int iIndex)
{
	char czSourceKey[56];
	CISource *pSource;
	CMySource *pMySource;
	StruGSMediaDescri descri;
	sprintf_s( czSourceKey,56, "test_%d",  iIndex);
	if( s_csSourceList.Find((void*)iIndex).Second() )
	{
		//已经存在
		MY_WARNING("Source %s had exist.\n", czSourceKey);
		return -1;
	}
	pSource = s_pSever->AddSource(czSourceKey);       
	if( pSource )
	{
		bzero( &descri, sizeof(StruGSMediaDescri));
		descri.eMediaType = GS_MEDIA_TYPE_SYSHEADER;
		descri.unDescri.struBinary.iSize = 1024;
		pSource->SetMediaInfo(0, &descri );

		bzero( &descri, sizeof(StruGSMediaDescri));
		descri.eMediaType = GS_MEDIA_TYPE_VIDEO;
		descri.unDescri.struVideo.eCodeID = GS_CODEID_GS_V461C;
		descri.unDescri.struVideo.iFrameRate = 12;
		descri.unDescri.struVideo.iFrameRate2 = 5;
		descri.unDescri.struVideo.iHeight = 320;
		descri.unDescri.struVideo.iWidth = 240; 
		pSource->SetMediaInfo(1, &descri );

		bzero( &descri, sizeof(StruGSMediaDescri));
		descri.eMediaType = GS_MEDIA_TYPE_AUDIO;
		descri.unDescri.struAudio.eCodeID = GS_CODEID_ST_MP3;
		descri.unDescri.struAudio.iBits = 8;
		descri.unDescri.struAudio.iChannels =1;
		descri.unDescri.struAudio.iSample= 16000;
		pSource->SetMediaInfo(2, &descri );

		CIUri *pUri = pSource->MakeURI();                   
		MY_PRINTF("Add source: URI(%s) Key(%s) OK.\n", pUri->GetURI(), pSource->GetKey()  );
		delete pUri;
		pSource->SourceEnableValid(TRUE);
		pSource->SourceEnableRef(TRUE); 
		pMySource = new CMySource(pSource);
		pSource->SetUserData(pMySource); 
		s_csSourceList.Insert( (void*) iIndex, pMySource );
	}
	return 0;
}
static int _RmSource(int iIndex)
{
	if( !s_pSever )
	{
		MY_PRINTF("Server not start.\n");
		return 0; 
	}
	s_csSourceList.Erase((void*)iIndex);
	return 0;
}

static int s_iAutoTestBegin = 0;
static int s_iAutoTestEnd = 30;
static BOOL s_bAutoTestStart = FALSE;
static void *OnAutoTestThread(void *pParam)
{
int i;
int x;
	while(s_bAutoTestStart)
	{
		for( i = s_iAutoTestBegin; i<s_iAutoTestEnd; i++ )
		{
		 x = (int)(s_iAutoTestBegin+ s_iAutoTestEnd*(rand() / (RAND_MAX + 1.0)));
		 _AddSource(x);
		 MSLEEP(500);
		}
		x = (int)(100+ 5000L*(rand() / (RAND_MAX + 1.0))); 
		MSLEEP(x);

		for(i = s_iAutoTestBegin;  i<s_iAutoTestEnd; i++ )
		{
			x =  (int)( s_iAutoTestBegin+ s_iAutoTestEnd*(rand() / (RAND_MAX + 1.0)));
			_RmSource(x);
			MSLEEP(500);
		}
		x = (int)(5000+ 100*(rand() / (RAND_MAX + 1.0))); 
		MSLEEP(x);

	}
	return 0;
}

static int _OnStartAutoTest(const char *czCmd,const char *args)
{
    // 自动测试
    if( !s_pSever )
    {
        MY_PRINTF("Server not start.\n");
        return 0; 
    }
	if( s_bAutoTestStart )
	{
		MY_PRINTF("Auto test is runing.\n");
		return 0;
	}




    char czSourceKey[56] = "Test00";
    int iCnts = 1;
    INT iBegin = 0;

    char *pParser;
    pParser = ArgsGetParser(args,NULL);
    if( pParser )
    {
        char *czKey =NULL, *czValue;
        int ret;
        while( (ret=ParserFetch(&pParser, &czKey, &czValue ) )==1 )
        {
            if( 0==strcmp("c", czKey ) )
            {
                //添加的个数
                if( !czValue )
                {
                    return -1;
                }
                iCnts = atoi(czValue);
            }
            else if( 0==strcmp("s", czKey ) )
            {
                //数据源索引开始
                if( !czValue )
                {
                    return -1;
                }
                iBegin = atoi(czValue);
            }

        }
    }

	s_iAutoTestBegin = iBegin;
	s_iAutoTestBegin = iBegin+iCnts;
	MY_PRINTF("Auto Test souce key test_%d -> test_%d\n",
			s_iAutoTestBegin, s_iAutoTestEnd );
	s_bAutoTestStart = TRUE;
	if( !TestCreateMyThread(OnAutoTestThread, NULL))
	{
		s_bAutoTestStart = FALSE;
		MY_PRINTF("Start auto test fail.\n");
		return -1;
	}

  
    return 0;
}

static int _OnStopAutoTest(const char *czCmd,const char *args)
{
	s_bAutoTestStart = FALSE;
    return 0;
}

static StruOptionLine _Options[]=
{
    {
        "-c",
            "-clean",
            "Clear server test source.",
            _OnClearTest
    },
    {
        "-pf",
            "-printstatus",
            "Print server status.",
            _OnShowStatusTest
        },
        {
            "-pfth",
                "-printthread",
                "Print threa status.",
                _OnShowThreadStatusTest
        },
        {
            "-t",
                "-transstream",
                " Turn on/off trans data. Args: [-s:number -e:number -o:[1|0] -f] \n \
                -s:number Source key start index, default -1.\n \
                -e:number  Source key end index, default -1.\n \
                -o:[1|0]   Trun on/off trans, default on.\n \
                -f         Trans mode of send file. default is rand data.\n \
                If start and end index < 0 will operate all source.\n \
                if start index >0 and end index < 0 will operate all source index > start index.\n", 
                _OnTransStreamTest
        },
        {
            "-i",
                "-init",
                "Init Server function argus:[ -p:port]. listen port.\n",
                _OnStartTest
            },
            {
                "-a",
                    "-addsource",
                    " Add source to server key is 'test_index'.Args： [-c:number -s:number]\n \
                    -s:number Source key start index, default 0.\n \
                    -c:number  Source counts. default 1.\n ",                       
                    _OnAddSourceTest
            },
            {
                "-r",
                "--removesource",
                " Remove source frome server key is 'test_index'. Args： [-s:number -e:number]\n \
                    -s:number Source key start index, default -1.\n \
                    -e:number  Source key end index, default -1.\n \
                    If start and end index < 0 will operate all source.\n \
                    If start index >0 and end index < 0 will operate all source index > start index.\n",    
                    _OnRemoveSourceTest,
                },
                {
                    "-o",
                    "--auto",
                    " Auto Test. Args： Args： [-s:number -e:number]\n \
                    -s:number Source key start index, default -1.\n \
                    -e:number  Source key end index, default -1.\n",
                    _OnStartAutoTest,
                },

                {
                    "-d",
                        "--dis",
                        " Stop auto test\n",
                        _OnStopAutoTest,
                    },


                {
                    NULL,
                        NULL,
                        NULL,
                        NULL,
                }
};

static void Convert(void)
{
FILE *fpR;
FILE *fpW;
FILE *fpC;
		fpR = fopen("hk_udp_rtp.raw", "rb" );
	//	fpR = fopen("yis_rtp.raw", "rb" );
		
		if( !fpR )
		{
			GS_ASSERT(0);
			printf("Open read file fail.\n");
			return;
		}
		fpW = fopen("rtp_info.txt", "wb+" );
		if(!fpW)
		{
			GS_ASSERT(0);
			printf("rtp_info.txt fail.\n");
			return;
		}
		fpC = fopen("rtp_convert_ps.raw", "wb+");
		if( !fpC )
		{
			GS_ASSERT(0);
			return;
		}
		INT32 iLen;
		int iRet;
		char *pBuf = new char[1024*1024*10];
		unsigned char *p;
		StruRTPHeader stRTP;
		int iIndex = 0;
		char czBuf[1024];
		unsigned char vMark[8] = {0x0,0x0,0x01, 0xee, 'g','s','a','v' };
		while(  1 )
		{
			iRet = fread(&iLen, 1,4, fpR);
			if( iRet != 4 )
			{
				break;
			}
			if( iRet > 1024*1024 )
			{
				GS_ASSERT(0);
				break;
			}
			iRet = fread(pBuf, 1,iLen, fpR);
			if( iRet!=iLen )
			{
				break;
			}
			p = (unsigned char *)pBuf;

			memcpy(&stRTP, p, sizeof(stRTP));
			p += sizeof(stRTP);

			stRTP.iSeq = COSSocket::Int16N2H(stRTP.iSeq);
			stRTP.iSSRC = COSSocket::Int32N2H(stRTP.iSSRC);
			stRTP.iTS  = COSSocket::Int32N2H(stRTP.iTS);
			
			GS_SNPRINTF(czBuf, 1024, "%08x, Ver:%d,PT:%d, Mark:%d, SSRC:%d, TS:%d, Seq: %d, len:%d\r\n",
									iIndex++, stRTP.iVer,stRTP.iPT, stRTP.iM,stRTP.iSSRC, stRTP.iTS, stRTP.iSeq, iLen-sizeof(stRTP));

			fputs(czBuf,fpW);

			//fwrite(vMark, 1,sizeof(vMark), fpC);
			//fwrite(&stRTP.iTS, 1,4, fpC);
					fwrite(p, 1, iLen-sizeof(stRTP), fpC);



		}
		delete pBuf;
		fclose(fpC);
		fclose(fpR);
		fclose(fpW);
		printf("Convert end.\n" );
}
 
int TestGSPServer(const char *czCmd,const char *args)
{
    MY_PRINTF("Test CGSPServer enter..\n");
	//Convert();

    OptionsEntery(_Options);
    MY_PRINTF("Test CGSPServer leave..\n");

    return 0;


}

