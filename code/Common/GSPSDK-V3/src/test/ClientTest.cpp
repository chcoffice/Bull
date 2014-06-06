/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : CLIENTTEST.CPP
Author :  zouyx
Version : 0.1.0.0
Date: 2010/8/27 9:00
Description: 客户端测试程序
********************************************
*/

#include <time.h>
#include "IGSPClient.h"
#include "MainLoop.h"
#include "cmdline.h"
#include "Frame.h"
#include "IUri.h"
#include "md5.h"
#include "Log.h"
#include "Uri.h"
#include <list>
#include <GSPMemory.h>
#include <RTP/RtpNet.h>


using namespace GSP;
using namespace GSP::RTP;


//#define CAT_RTP_PACKET 1
//#define SIM_RTP_SEND 1


#ifdef SIM_RTP_SEND 

typedef struct _StruSliceBuffer
{
	unsigned char *pBuffer;
	int iDataSize;
	_StruSliceBuffer(const unsigned char *pData, int iSize)

	{
		pBuffer = (unsigned char *)malloc(iSize+4);
		GS_ASSERT(pBuffer);
		memcpy(pBuffer, pData, iSize);
		iDataSize = iSize;
	}

	~_StruSliceBuffer(void)
	{
		
	}
}StruSliceBuffer;

typedef std::vector<StruSliceBuffer> CVectorSliceBuffer;


class CRtpCache
{
public :
	CVectorSliceBuffer m_vFrames;
	INT m_iRPos;
	CRtpCache(void)
	{
		
	}
	~CRtpCache(void)
	{

	}


	bool Init( const char *czFilename )
	{
		FILE *fp = fopen( czFilename, "rb");
		if( !fp )
		{
			printf( "Open '%s' fail file not exist.\n", czFilename);
			return false;
		}
		int iBufferSize = 20L*(1L<<20);
		unsigned char *pBuffer = (unsigned char *)::malloc( iBufferSize );
		if( !pBuffer )
		{			
			printf( "Open '%s' fail malloc fail.\n", czFilename);
			fclose(fp);
			return false;
		}
		int iFSize = fread(pBuffer, 1, iBufferSize, fp);
		fclose(fp);
		if( iFSize<1 )
		{
			free(pBuffer);
			printf( "Open '%s' fail file invalid.\n", czFilename);
			return false;
		}
		
		unsigned char *p = pBuffer;
		INT32 iDataLen = 0;
		int iFL = iFSize;
		for(int i = 0; iFL>4;i++)
		{
			::memcpy(&iDataLen,  p, 4 );
			p+=4;
			iFL -= 4;			
			if( iDataLen > iFL )
			{
				break;  			
			}
			m_vFrames.push_back( StruSliceBuffer(p, iDataLen) );
			
			iFL -= iDataLen;
			p+= iDataLen;
		}
		free(pBuffer);
		return m_vFrames.size()>10;
	}


};

#endif



#ifdef CAT_RTP_PACKET


class MYSave : public CRtpUdpReader
{
public :
	std::list<CFrameCache *> m_listCache;
	bool m_bSave;
	MYSave(void)
	{
		m_bSave = false;
	}

	virtual EnumErrno Start(void)
	{
		EnumErrno eRet = Init(6000, 0, "192.168.15.142", TRUE, FALSE);
		if( eRet )
		{
			GS_ASSERT(0);
			return eRet;
		}
		eRet = CRtpUdpReader::Start();
		GS_ASSERT(eRet==eERRNO_SUCCESS);
		return eRet;

	}

	void Save(void)
	{
		m_bSave = true;
		printf( "保存RTP 文件" );
		FILE *fp = fopen("save4rtp.bin", "wb+" );
		if( !fp )
		{
			GS_ASSERT(0);
			printf("RTP保存文件失败");
			return;
		}
		std::list<GSP::CGSPBuffer *>::iterator csIt;
		const unsigned char *p;
		INT32 iSize,iRet;
		unsigned char cbuf[4] = {0x00,0x00,0x01,0xba};

		std::vector<GSP::CGSPBuffer *> vFrame; // 一帧数据
		for( csIt=m_listCache.begin(); csIt!=m_listCache.end(); ++csIt )
		{
			

			p = (*csIt)->m_bBuffer;
			iSize = (*csIt)->m_iDataSize;

 			p+=RTP_PACKET_HEADER_LENGTH;
 			iSize-=RTP_PACKET_HEADER_LENGTH;
#if 1

			if( memcmp(p,cbuf, sizeof(cbuf) )==0  )
			{
				if( !vFrame.empty() )
				{
					//非空 保存一帧数据
					iSize = 0;
					for( UINT j = 0; j<vFrame.size(); j++ )
					{
						//计数一帧大小
						iSize += vFrame[j]->GetDataSize();
						iSize -= RTP_PACKET_HEADER_LENGTH;
					}
					iRet = fwrite( &iSize, 1, 4, fp);
					if( iRet != 4 )
					{
						GS_ASSERT(0);
						printf("RTP保存文件失败");
						fclose(fp);
						return;
					}
					for( UINT j = 0; j<vFrame.size(); j++ )
					{
						//计数一帧大小
						iSize = vFrame[j]->GetDataSize();
						iSize -= RTP_PACKET_HEADER_LENGTH;
						p = vFrame[j]->GetData();
						p+=RTP_PACKET_HEADER_LENGTH;

						iRet = fwrite( p, 1, iSize, fp);
						if( iRet != iSize )
						{
							GS_ASSERT(0);
							printf("RTP保存文件失败");
							fclose(fp);
							return;
						}
					}
					vFrame.clear();
				}
				vFrame.push_back(*csIt);
			} else if( vFrame.empty() )
			{
				//空
				continue;
			}
			else
			{
				vFrame.push_back(*csIt);
			}
#else
			GS_ASSERT(iSize>0);



			iRet = fwrite( &iSize, 1, 4, fp);
			if( iRet != 4 )
			{
				GS_ASSERT(0);
				printf("RTP保存文件失败");
				fclose(fp);
				return;
			}
			iRet = fwrite( p, 1, iSize, fp);
			if( iRet != iSize )
			{
				GS_ASSERT(0);
				printf("RTP保存文件失败");
				fclose(fp);
				return;
			}
#endif
		}
		fclose(fp);
		printf("RTP保存文件完成");
	}

	void *OnSocketEvent(EnumRtpNetEvent eEvt, void *pEvtArgs)
	{
		if( eEvt==eEVT_RTPNET_STREAM_FRAME )
		{
			
			if( m_bSave ) return 0;
			CFrameCache *pBuf = (CFrameCache *)pEvtArgs;		
			pBuf->RefObject();
			m_listCache.push_back(pBuf);
			printf( "rtp packet: %d\n",  m_listCache.size());
			if( m_listCache.size()>=300 )
			{
				//开始存储
				Save();
				return 0;
			}
			return (void*)TRUE;
		}
		return (void*)0;
	}

};
static MYSave *s_pRtp = NULL;

#endif

static CIClientSection *s_pCliSection = NULL;
static GSAtomicInter s_iFIndex = 0;




#define _BUFFER_SIZE (KBYTES*64+34)


class CMyChannel : public CGSPObject
{
public :
    CIClientChannel *m_pChannel;    
    FILE *m_fp;
    GSAtomicInter m_iRcvTotal;
    UINT64 m_iLastRcv;
    UINT64 m_iLastTv;
    BOOL m_bRecord;
    BOOL m_bOpen;
    INT m_iLastRcvTV;
    UINT32 m_iLastIndex;
    CMyChannel(CIClientChannel *pChannel )
        :CGSPObject()
        ,m_pChannel(pChannel) 
        ,m_fp(NULL)
        ,m_iRcvTotal(0)
        ,m_iLastRcv(0)
        ,m_iLastTv(0)
        ,m_bRecord(FALSE)
        ,m_bOpen(FALSE)
    {
        m_iLastRcvTV = 0;
        m_iLastIndex = MAX_UINT32;
    }
    virtual ~CMyChannel(void)
    {
        
        if( m_pChannel )
        {

           MY_PRINTF("%s Close. Rcv: %d LastRcvTv: %d - %d.\n", m_pChannel->GetURI(), m_iRcvTotal
                ,m_iLastRcvTV, time(NULL) );
            m_pChannel->SetUserData(NULL);
            m_pChannel->Release();
        }
        if( m_fp )
        {
            fclose(m_fp);
            m_fp = NULL;
        }
    }

    void PrintStatus(void)
    {
        UINT64 iCur = DoGetTickCount();
        UINT64 iTotalRcv = m_iRcvTotal;

        INT iIRcv;           
        INT iITv;
        double iTemp;
        double  fRate;
        iIRcv =(INT)(iTotalRcv-m_iLastRcv );
        iITv = (INT)(iCur-m_iLastTv);

		const CIClientChannel::StruChannelInfo *pInfo;

        iTemp = (iITv/1000);
        if( iTemp==0 )
        {
            iTemp = 1;
        }



        fRate = ((iIRcv/1024.00)/iTemp);
		if( m_pChannel)
		{
			pInfo = m_pChannel->GetInfo();
			MY_PRINTF("Channel(%u) In %d MSec. Rcv:%d , %.3f KB/Sec. Chn: R:%lld, W:%lld, LostFrame:%lld, NetLostFrame:%lld\n", 
			m_pChannel->GetAutoID(),
            iITv,
            iIRcv, fRate
			,pInfo->iRcvFromNet,pInfo->iSendToNet, 
			pInfo->iLostFrames, pInfo->iLostNetFrames);
      
		}
		else
		{
			MY_PRINTF("Channel(%u) In %d MSec. Rcv:%d , %.3f KB/Sec.\n", 
				MAX_UINT32, 
				iITv,
				iIRcv, fRate);
		}
		m_iLastTv  = iCur;
		m_iLastRcv  = iTotalRcv;
    }


    void Play( BOOL bRecord )
    {
        //开始通道播放
       if(bRecord)
       {
            if( !m_fp )
            {

                FILE *fp;
                CGSString csFilename;
				GSStrUtil::AppendWithFormat(csFilename,
							"D:/Develop/C3M-Video/SourceCodes/Common/GSPSDK/Debug/rcv%d.rmvb",
							AtomicInterDec(s_iFIndex)+1 );
                fp = fopen( csFilename.c_str() , "wb+");
                if( !fp )
                {
                    MY_PRINTF( "Open %s fail. %s.\n",csFilename.c_str(), strerror( errno));
                }
            }
            m_bRecord = TRUE;
        }
        else
        {
            m_bRecord = FALSE;
            if(m_fp )
            {
                fclose(m_fp);
                m_fp = NULL;
            }
        }
        StruGSPCmdCtrl ctrl;
        bzero(&ctrl, sizeof( ctrl));
        ctrl.iCtrlID = GSP_CTRL_PLAY;
        if( m_pChannel->Ctrl(ctrl) )
        {
            MY_PRINTF("%s Play  OK.\n", m_pChannel->GetURI() );
           
        }
        else
        {
             MY_PRINTF("%s Play Trans  Fail.\n", m_pChannel->GetURI() );
        }
    }

    void Stop(void)
    {
        FILE *fp;
        fp = m_fp;
        m_fp = NULL;
        if( fp )
        {
            fclose(fp);
        }
        StruGSPCmdCtrl ctrl;
        bzero(&ctrl, sizeof( ctrl));
        ctrl.iCtrlID = GSP_CTRL_STOP;
        if( m_pChannel->Ctrl(ctrl) )
        {
             MY_PRINTF("%s Stop  OK.\n", m_pChannel->GetURI() );
           
        }
        else
        {
            MY_PRINTF("%s Stop Trans  Fail.\n", m_pChannel->GetURI() );
        } 
    }

    void OnRcvFrame( CFrame *pFrame)
    {
        m_iLastRcvTV = (INT)time(NULL);
		m_iRcvTotal += pFrame->GetFrameSize();


        INT  w;
		CPAT::CRefBuffer csBuffer;
        pFrame->GetFrameData(csBuffer);
        w = csBuffer.GetDataSize();

		GS_ASSERT_RET(w>0);
// 
//         if( pFrame->GetFrameSize() !=w )
//         {
//             GSP_ASSERT(0);
//         }
//         if(  pFrame->GetFrameSize() != _BUFFER_SIZE )
//         {
//             GSP_ASSERT(0);
//         }
     

        UINT32 iIndex;
        unsigned char *pTemp;
        pTemp = (unsigned char *)csBuffer.GetData();

        memcpy(&iIndex, pTemp, sizeof(UINT32) );



        if( m_iLastIndex != MAX_UINT32 )
        {
          
            if( (m_iLastIndex+1)!=iIndex )
            {
                MY_PRINTF("Index Err: %d != %d+1\n", iIndex, m_iLastIndex );				
				
            }
        }
         m_iLastIndex = iIndex;

         unsigned char md5res[MD5_LEN];
         MD5Sum(pTemp,w-MD5_LEN, md5res);

         if( memcmp( pTemp+(w-MD5_LEN), md5res, MD5_LEN) )
         {

			 CGSString strMd5A = MD5toString(md5res);
			  CGSString strMd5B = MD5toString(pTemp+(w-MD5_LEN));
			  MY_PRINTF( "MD5 %s != %s FAIL.\n",strMd5A.c_str(), strMd5B.c_str() );
         }
        

        if( m_bRecord && m_fp )
        {
            INT s; 
             s = fwrite( csBuffer.GetData(), 1, w, m_fp);
             if( s!=w )
             {
                 GS_ASSERT(0);
             }
        }
    }
};

static INT OnClientSectionEvent(CIClientSection *pcsClient,CIClientChannel *pChannel, 
                                EnumGSPClientEventType eEvtType, 
                                void *pEventData,  INT iEvtDataLen, void *pUserData )
{
CMyChannel *pMyChannel = (CMyChannel*) pChannel->GetUserData();
    if( !pMyChannel )
    {
        MY_WARNING("%s User data is NULL.\n", pChannel->GetURI() );
    }
    //客户端事件回调
    switch( eEvtType)
    {
    case GSP_EVT_CLI_FRAME : 
        {
            if( pMyChannel )
            {
                pMyChannel->OnRcvFrame((CFrame*)pEventData );
            }
            // MY_PRINTF("Event %d(frame) %d.\n", (INT)eEvtType, pFrame->GetFrameSize());
        }
        break;
    case GSP_EVT_CLI_DESTROY:
		if( pMyChannel )
		{
			pMyChannel->m_pChannel = NULL;
			delete pMyChannel;
		}
	break;
    case GSP_EVT_CLI_ASSERT_CLOSE :
    case GSP_EVT_CLI_COMPLETE :
        if( pMyChannel )
        {

            delete pMyChannel;
        }
        else
        {

            pChannel->Release();
        }
        break;
    case  GSP_EVT_CLI_RETREQUEST:
        {

            BOOL bRet = (BOOL)pEventData;

			MY_PRINTF("Test Request:'%s' 结果：%d\n",pChannel->GetURI(), bRet );
           
            if(!bRet &&  pChannel )
            {

                pChannel->Close();
                pChannel->Open(GSP_TRAN_RTPLAY, 0);
            }            
        }
    break;

	case  GSP_EVT_CLI_CTRL_OK:
	case  GSP_EVT_CLI_CTRL_FAIL :
		{

			StruGSPCmdCtrl *pCtrl = (StruGSPCmdCtrl*)pEventData;

			MY_PRINTF("Test 控制(0x%x):'%s'  结果：%d\n",
				pCtrl->iCtrlID, pChannel->GetURI(), eEvtType==GSP_EVT_CLI_CTRL_OK );
          
		}
		break;

    default:
        {
            MY_PRINTF("%s Event %d.\n",pChannel->GetURI(),  (INT)eEvtType);
        }
        break;
    }
    return 0;
}



static int _OnInitTest(const char *czCmd,const char *args)
{
    //建立客户端 Section
    if( s_pCliSection )
    {
        MY_PRINTF("Client is was started.\n");
        return 0;

    }
    s_pCliSection = CreateGSPClientSectionInterface();
    GS_ASSERT_RET_VAL(s_pCliSection!=NULL, 0);

    s_pCliSection->InitLog("./GSPClient");    
     s_pCliSection->SetEventListener(OnClientSectionEvent,NULL);
     s_pCliSection->Init(NULL); 

    MY_PRINTF("Client start...\n");

#ifdef CAT_RTP_PACKET

	s_pRtp = new MYSave();
	if( s_pRtp->Start() )
	{
		printf( "开始介绍 RTP 包!!");
	}
	else
	{
		printf( "开始介绍 RTP 包 失败!!");
	}

#endif


#ifdef SIM_RTP_SEND
	CRtpCache sendRtp;

	CUDPClientSocket *pSocket = CUDPClientSocket::Create();
	GS_ASSERT( 0==pSocket->InitSocket(15000, NULL));

	CGSString strPath = GSGetApplicationPath();
	strPath += "save4rtp.bin";
	GS_ASSERT( sendRtp.Init(strPath.c_str()) );
	StruLenAndSocketAddr strRemote;
	GS_ASSERT(COSSocket::Host2Addr("192.168.15.142", 6000, strRemote));
	struct iovec stIO;
	
	for( UINT i=0; i<sendRtp.m_vFrames.size(); i++ )
	{
		stIO.iov_base = (char*) sendRtp.m_vFrames[i].pBuffer;
		stIO.iov_len =sendRtp.m_vFrames[i].iDataSize;	
		pSocket->OSSendTo( &stIO, 1, &strRemote);
		printf("send rtp %d\n", i);
		MSLEEP(10);
	}
#endif


	
    return 0;
}

static int _OnUninitTest(const char *czCmd,const char *args)
{
    //清理资源
    if( s_pCliSection )
    {
        s_pCliSection->Release();
        s_pCliSection = NULL;
    }
    return 0;
}



static CIClientChannel * _AddChannel(CIClientSection *pSection,const char *czKey, const char *czServerHost, int iPort, BOOL bSip )
{
    //增加通道
    CIClientChannel *pCliChn;

	char *szSdp = 
	"v=0\r\n"
	"o=34020000002020000001 0 0 IN IP4 192.168.15.142\r\n"
	"s=Play\r\n"
	"c=IN IP4 192.168.27.215\r\n"
	"t=0 0\r\n"
	"m=video 6000 RTP/AVP 96 98 97\r\n"
	"a=recvonly\r\n"
	"a=rtpmap:96 PS/90000\r\n"
	"a=rtpmap:98 H264/90000\r\n"
	"a=rtpmap:97 MPEG4/90000\r\n";



    pCliChn = pSection->CreateChannel();
    if( !pCliChn )
    {
        MY_PRINTF("Create Channel fail.\n");
    }
    else
    {

        CUri csURI;
        CMyChannel *pMyChannel;
        pMyChannel = new CMyChannel(pCliChn);

        pCliChn->SetUserData(pMyChannel);

        csURI.SetHost(czServerHost);  //46
        csURI.SetKey(czKey);
        csURI.SetPortArgs(iPort);
		if( bSip )
		{
			csURI.SetScheme( "sip" );
		}
		else
		{
			csURI.SetScheme( "gsp" );
			csURI.AddAttr("pro", "tcp");
		}
        
        csURI.AddAttr("S", "35d3ed223");
		if(  !bSip )
		{
			if( !pCliChn->SetURI( csURI.GetURI() ))
			{    
				MY_PRINTF("Set URI:%s Fail.\n",csURI.GetURI() );
				delete pMyChannel;
				pCliChn = NULL;
				return NULL;
			}
		}
		else
		{
			if( !pCliChn->SetURIOfSip( csURI.GetURI(), NULL ))
			{    
				MY_PRINTF("Set URI:%s Fail.\n",csURI.GetURI() );
				delete pMyChannel;
				pCliChn = NULL;
				return NULL;
			}
		}

        if( ! pCliChn->Open(GSP_TRAN_REPLAY, 50000 )  )
        {
            MY_PRINTF("Open URI:%s Fail.\n",csURI.GetURI() );
           delete pMyChannel;
           pCliChn = NULL;
        }
        else
        {
			
            MY_PRINTF("Open URI:%s OK.\n",csURI.GetURI());
			if( bSip )
			{
				const char *czRSdp = pCliChn->GetSdp();
				if( czRSdp )
				{
					MY_PRINTF("**open rcv sdp\r\n%s\n",czRSdp);
				}
			}

        }
    }
    return pCliChn;
}


static BOOL _OnFetchChannelEvent( const CIClientChannel*pChannel,
								void *pUserParam)
{
	std::list<CIClientChannel*> *p = (std::list<CIClientChannel*> *)pUserParam;
	p->push_back((CIClientChannel*)pChannel);
	return TRUE;
}




static int _OnPlayChannelTest(const char *czCmd,const char *args)
{
	if( s_pCliSection==NULL)
	{
		MY_PRINTF("Section not init.\n");
		return -1;
	}
	BOOL bRecord = FALSE;
	char *pParser = ArgsGetParser(args,NULL);
	if( pParser )
	{
		char *czKey =NULL, *czValue;
		int ret;
		czKey =NULL;
		czValue =  NULL;
		while( (ret=ParserFetch(&pParser, &czKey, &czValue ) ) > 0 )
		{
			if( 0==strcmp(czKey, "rcd") )
			{
				if( !czValue )
				{
					return -1;
				}
				bRecord = atoi( czValue);
			}
		}
	}

	std::list<CIClientChannel*> lChannels;
	s_pCliSection->FetchClientChannel(_OnFetchChannelEvent, &lChannels ); 
	CIClientChannel *p;
	CMyChannel *pMyChannel;   
	for(std::list<CIClientChannel*>::iterator csIt=lChannels.begin(); 
		csIt != lChannels.end();
		++csIt )
	{
		p = *csIt;
		pMyChannel = (CMyChannel*)p->GetUserData();
		if( pMyChannel )
		{
			pMyChannel->Play(bRecord);
		}
		else
		{

			MY_WARNING("%s user data is NULL.\n", p->GetURI());
		}
	}
	return 0;


	return 0;
}    


static int _OnStopPlayChannelTest(const char *czCmd,const char *args)
{
    if( s_pCliSection==NULL)
    {
        MY_PRINTF("Section not init.\n");
        return -1;
    }
	std::list<CIClientChannel*> lChannels;
	s_pCliSection->FetchClientChannel(_OnFetchChannelEvent, &lChannels ); 
	CIClientChannel *p;
	CMyChannel *pMyChannel;   
	for(std::list<CIClientChannel*>::iterator csIt=lChannels.begin(); 
		csIt != lChannels.end();
		++csIt )
	{
		p = *csIt;
		pMyChannel = (CMyChannel*)p->GetUserData();
		if( pMyChannel )
		{
			pMyChannel->Stop();
		}
		else
		{
			
			MY_WARNING("%s user data is NULL.\n", p->GetURI());
		}
	}
    return 0;
}

static int _OnCloseChannelTest(const char *czCmd,const char *args)
{
    if( s_pCliSection==NULL)
    {
        MY_PRINTF("Section not init.\n");
        return -1;
    }
   std::list<CIClientChannel*> lChannels;
    s_pCliSection->FetchClientChannel(_OnFetchChannelEvent, &lChannels ); 
    CIClientChannel *p;
    CMyChannel *pMyChannel;   
	for(std::list<CIClientChannel*>::iterator csIt=lChannels.begin(); 
		 csIt != lChannels.end();
		 ++csIt )
    {
        p = *csIt;
        pMyChannel = (CMyChannel*)p->GetUserData();
        if( pMyChannel )
        {
            delete pMyChannel;
        }
        else
        {
            p->Release();
            MY_WARNING("%s user data is NULL.\n", p->GetURI());
        }
    }
    return 0;
}


static int _OnAddChannelTest(const char *czCmd,const char *args)
{

	int iPort = 12345;
	char *czServerIP = "localhost";
	char *czSrcKey = NULL;
	int iCounts=1;
	int iBegin = 0;

	if( !s_pCliSection )
	{
		MY_PRINTF("Add channel fail. Client Section not start.\n");
		return 0;
	}

	char *pParser = ArgsGetParser(args,NULL);
	if( pParser )
	{
		char *czKey =NULL, *czValue;
		int ret;
		czKey =NULL;
		czValue =  NULL;
		while( (ret=ParserFetch(&pParser, &czKey, &czValue ) ) > 0 )
		{

			if( 0==strcmp(czKey,"s") )
			{
				czServerIP = czValue;
			}
			else if( 0==strcmp(czKey,"p") )
			{
				if( !czValue )
				{
					return -1;
				}
				iPort = atoi( czValue);
			}
			else if(  0==strcmp(czKey,"c") )
			{
				if( !czValue )
				{
					return -1;
				}
				iCounts = atoi(czValue);
			}
			else if( 0==strcmp(czKey,"k" ) )
			{
				czSrcKey = czValue;
			}
			else if( 0==strcmp(czKey, "b") )
			{
				if( !czValue )
				{
					return -1;
				}
				iBegin = atoi( czValue);
			}
		}
	}
	int i = 0;
	CIClientChannel *pChn;

	if( czSrcKey )
	{    
		MY_PRINTF("Add channel. Server:%s:%d  SourceKey:%s  Counts:%d.\n", czServerIP, iPort, czSrcKey, iCounts);
		for(i = 0; i<iCounts; i++ )
		{
			pChn = _AddChannel(s_pCliSection, czSrcKey, czServerIP, iPort, FALSE);
		}
	}
	else
	{
		char czTemp[65];
		MY_PRINTF("Addchannel .Server:%s:%d  Source test_%d --> test_%d.\n", czServerIP, iPort, iBegin, iBegin+iCounts);

		for(i =  iBegin; i<(iCounts+iBegin); i++ )
		{
			sprintf_s(czTemp, 64, "test_%d", i );
			pChn = _AddChannel(s_pCliSection, czTemp, czServerIP, iPort, FALSE);
		}
	}



	return 0;
}



static int _OnAddSipChannelTest(const char *czCmd,const char *args)
{

int iPort = 5060;
char *czServerIP = "192.168.27.242";
char *czSrcKey = NULL;
int iCounts=1;
int iBegin = 0;

    if( !s_pCliSection )
    {
        MY_PRINTF("Add channel fail. Client Section not start.\n");
        return 0;
    }

char *pParser = ArgsGetParser(args,NULL);
    if( pParser )
    {
        char *czKey =NULL, *czValue;
        int ret;
        czKey =NULL;
        czValue =  NULL;
        while( (ret=ParserFetch(&pParser, &czKey, &czValue ) ) > 0 )
        {
 
            if( 0==strcmp(czKey,"s") )
            {
               czServerIP = czValue;
            }
            else if( 0==strcmp(czKey,"p") )
            {
                 if( !czValue )
                 {
                     return -1;
                 }
                iPort = atoi( czValue);
            }
            else if(  0==strcmp(czKey,"c") )
            {
                if( !czValue )
                {
                    return -1;
                }
                iCounts = atoi(czValue);
            }
            else if( 0==strcmp(czKey,"k" ) )
            {
                czSrcKey = czValue;
            }
            else if( 0==strcmp(czKey, "b") )
            {
                if( !czValue )
                {
                    return -1;
                }
                iBegin = atoi( czValue);
            }
        }
    }
int i = 0;
CIClientChannel *pChn;

    if( czSrcKey )
    {    
        MY_PRINTF("Add channel. Server:%s:%d  SourceKey:%s  Counts:%d.\n", czServerIP, iPort, czSrcKey, iCounts);
        for(i = 0; i<iCounts; i++ )
        {
            pChn = _AddChannel(s_pCliSection, czSrcKey, czServerIP, iPort, TRUE);
        }
    }
    else
    {
        char czTemp[65];
        MY_PRINTF("Addchannel .Server:%s:%d  Source test_%d --> test_%d.\n", czServerIP, iPort, iBegin, iBegin+iCounts);

        for(i =  iBegin; i<(iCounts+iBegin); i++ )
        {
            sprintf_s(czTemp, 64, "test_%d", i );
            pChn = _AddChannel(s_pCliSection, czTemp, czServerIP, iPort, TRUE);
        }
    }


   
    return 0;
}

static BOOL _OnFetchChannelPrintInfoEvent( const CIClientChannel*pChannel,
								 void *pUserParam)
{
	 CMyChannel* pMyChannel = (CMyChannel*)pChannel->GetUserData();
	 if( pMyChannel )
	 {
		 pMyChannel->PrintStatus();
	 }
	 return 1;
}

static int _OnPrintInfo(const char *czCmd,const char *args)
{
    if( s_pCliSection == NULL)
    {
        MY_PRINTF("Section not init.\n");
        return -1;
    }
	s_pCliSection->FetchClientChannel(_OnFetchChannelPrintInfoEvent, NULL);
    return 0;
}

static StruOptionLine _Options[]=
{
    {
        "-pf",
            "-print",
            "Print Client test.",
            _OnPrintInfo
    },
    {
        "-i",
            "-init",
            "Init Client test.",
            _OnInitTest
    },
    {
        "-u",
            "-uninit",
            "Uninit Client test.",
            _OnUninitTest
        },

    {
        "-a",
            "-add",
            "Add channel to server.\n \
            -s:hostname server ip default 127.0.0.1\n \
            -k:key  link source key default test\n \
            -p:port server listen port default 8443\n \
            -c:counts add channels default 1 \n \
            -b:number Add test_index start \n \
            IF Input -k -b is invalid.\n",
            _OnAddChannelTest
        },
        {
            "-p",
                "-play",
                "Start channel play, Args:[ -rcd:[0|1] ]  Enable record, Default is FALSE.\n",
                _OnPlayChannelTest
        },
        {
            "-s",
                "-stop",
                "Stop channel play.\n",
                _OnStopPlayChannelTest
            },
            {
                "-c",
                    "-close",
                    "Close channel play.\n",
                    _OnCloseChannelTest
            },
			{
				"-y",
					"-sip",
					"Add sip channel to server.\n \
					-s:hostname server ip default 127.0.0.1\n \
					-k:key  link source key default test\n \
					-p:port server listen port default 8443\n \
					-c:counts add channels default 1 \n \
					-b:number Add test_index start \n \
					IF Input -k -b is invalid.\n",
					_OnAddSipChannelTest

			},

    {
        NULL,
            NULL,
            NULL,
            NULL,
        }
};


#ifdef SIM_RTP_SEND
class CPSParser
{
public :
	CRtpCache m_csInCache;
	FILE *m_pfOut;
CPSParser(void)
{
	
	m_pfOut = NULL;

}

~CPSParser(void)
{

	if( m_pfOut )
	{
		fclose(m_pfOut);
		m_pfOut = NULL;
	}
}

	void Convert(void)
	{
		CGSString strPath = GSGetApplicationPath();
		CGSString strFilename  = strPath+"save4rtp.bin";
		
		BOOL bRet = m_csInCache.Init(strFilename.c_str());
		GS_ASSERT_RET(bRet);
		strFilename = strPath+"save4rtp_dh.h264";

		m_pfOut = fopen( strFilename.c_str(), "wb+");
		GS_ASSERT_RET(m_pfOut);
		unsigned char *p;
		UINT32 code;
		INT32 s;
		UINT16 pksize = 0;
		INT32 iRet;
		int j = 0;
		INT32 iFramesize = 0;
		INT32 iFCur;
		INT32 iTotals = 0, iTotalsa = 0;
		for(UINT k = 0; k<m_csInCache.m_vFrames.size(); k++ )
		{
			s = m_csInCache.m_vFrames[k].iDataSize;
			p = m_csInCache.m_vFrames[k].pBuffer;
			code = 0;
			iFramesize = 0;

			iTotalsa += s;

			iFCur = ftell(m_pfOut);
			iRet = fwrite(&iFramesize, 1, 4, m_pfOut);
			GS_ASSERT_RET(iRet==4);

			for( j = 0; j<s; j++ )
			{
				code = (code<<8) + p[j];
				if ((code & 0xffffffff) == 0x1E0)
				{
					s -= (j+1);
					p += (j+1);
					memcpy( &pksize, p, sizeof(pksize));
					pksize = ntohs(pksize);
					pksize -= 8;
					p += 10;
					s -= 10;

					if(s<pksize)
					{
					//	pksize = s;
// 						if( iFramesize > 0)
// 						{						
// 							fseek(m_pfOut, iFCur, SEEK_SET);
// 							iRet = fwrite(&iFramesize, 1, 4, m_pfOut);
// 							GS_ASSERT_RET(iRet==4);
// 							fseek(m_pfOut, 0, SEEK_END);		
// 							iFCur = ftell(m_pfOut);
// 							iRet = fwrite(&iFramesize, 1, 4, m_pfOut);
// 							GS_ASSERT_RET(iRet==4);
// 							iFramesize = 0;
// 						}
// 						else
						{
							j = 5;
							s = 0;
							break;
						}
					}
					iRet = fwrite(p, 1, pksize, m_pfOut);
					GS_ASSERT_RET(iRet==pksize);		
					iFramesize += pksize;
					s -= pksize;
					p += pksize;
					code = 0;
					j = -1;
				}
				
			}
			fseek(m_pfOut, iFCur, SEEK_SET);
			iTotals += iFramesize;
			iRet = fwrite(&iFramesize, 1, 4, m_pfOut);
			GS_ASSERT_RET(iRet==4);
			fseek(m_pfOut, 0, SEEK_END);	
					
		}

		printf( "totals: %d,, %d\n", iTotals, iTotalsa);

	}

};
#endif

int TestGSPClient(const char *czCmd,const char *args)
{
    MY_PRINTF("Test GSP Client Test enter..\n");

#ifdef SIM_RTP_SEND
#if 1
	CPSParser csParser;
	csParser.Convert();
	return 0;
#endif
#endif

    OptionsEntery(_Options);
    MY_PRINTF("Test GSP Client Test  leave..\n");
// 	if( s_pRtp )
// 	{
// 
// 	s_pRtp->Disconnect();
// 	s_pRtp->UnrefObject();
// 	s_pRtp = NULL;
// 	}

	if( s_pCliSection )
	{
		s_pCliSection->Release();
		s_pCliSection = NULL;

	}
    return 0;


}



