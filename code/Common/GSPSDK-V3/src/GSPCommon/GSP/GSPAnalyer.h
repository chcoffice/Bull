

#ifndef GSS_GSPANALYER_DEF_H
#define GSS_GSPANALYER_DEF_H

/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : ANALYER.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/6/13 8:43
Description: 进行协议分析
********************************************
*/

#include "../GSPObject.h"
#include "../List.h"
#include "../GSPProDebug.h"
//#include "../MediaInfo.h"
#include "../md5.h"
#include "../GSPMemory.h"
#include "../MediaInfo.h"


namespace GSP
{

#define GSP_MAX_CHK_CRC_LEN (GSP_PACKET_SIZE_OFFSET_HEADER-GSP_PACKET_HEADER_LEN)  //效验数据的最长长的

	
	class CGspProPacket : public CProPacket
	{
	private :
		union
		{
			BYTE bBuffer[sizeof(StruGSPPacketHeader)]; // header buffer
			StruGSPPacketHeader stHeader;
		}m_unHeader;
		BYTE m_bTBuffer[GSP_MAX_CHK_CRC_LEN]; // tailer buffer
		BYTE *m_pPBuffer; // playload buffer
		UINT m_iPBufMaxSize;  // m_pPBuffer max size

	
		UINT m_iWholeSize;

		INT m_iAnalyseErrno;

		CRefObject *m_pPasteBuf;
	public :
		static CGspProPacket *Create( UINT iMaxWholeSize );

		static CGspProPacket *Create(CRefObject *pPasteBuf, 
							const BYTE **ppPlayload, UINT &iSize, INT iVersion);

		/*
		 *********************************************
		 Function : Analyse
		 Version : 1.0.0.1
		 Author : 邹阳星
		 DateTime : 2013/4/12 9:49
		 Description :  分析数据
		 Input :  ppData 被分析的数据， 出入参数， 返回使用后的指针
		 Input :  iSize 数据长度 ， 出入参数， 返回使用后的大小
		 Output : 
		 Return : 返回 				
		 Note :   
		 *********************************************
		 */
		EnumErrno Analyse( const BYTE **ppData, UINT &iSize );

		/*
		 *********************************************
		 Function : AnalyseResult
		 Version : 1.0.0.1
		 Author : 邹阳星
		 DateTime : 2013/4/12 9:53
		 Description :  分析结果
		 Input :  
		 Output : 
		 Return :  0 分析完成，已经是完整包,
				-1 没有数据
				-2 没有完整头
				-3 数据版本不对
				-4 效验不对
				-5 数据长度不对
				> 0 还需要的数据长度
		 Note :   
		 *********************************************
		 */
		INT AnalyseResult(void);


		/*
		 *********************************************
		 Function : AnalyseReset
		 Version : 1.0.0.1
		 Author : 邹阳星
		 DateTime : 2013/4/12 9:56
		 Description :  重置
		 Input :  
		 Output : 
		 Return : 
		 Note :   
		 *********************************************
		 */
		void Reset(void);

		StruGSPPacketHeader &GetHeader(void)
		{
			return m_unHeader.stHeader;
		}

		INLINE BOOL IsCommand(void) const
		{
			return m_unHeader.stHeader.iDataType == GSP_PACKET_TYPE_CMD;
		}

		static INLINE BOOL IsCommand(const StruGSPPacketHeader &stH) 
		{
			return (stH.iDataType == GSP_PACKET_TYPE_CMD);
		}



		//打包， 完成
		EnumErrno Packet(const StruGSPPacketHeader &stHeader );

		EnumErrno AppendPlayload( const BYTE *pData, UINT iSize );

		//尽可能添加数据
		EnumErrno AppendPlayloadInMaxC( const BYTE **pData, UINT &iSize );

		UINT GetFreeBuffSize(void) const;

		virtual CRefObject *Clone(void) const
		{
			return NULL;
		}

	protected :

	private :
		CGspProPacket(void);
		~CGspProPacket(void);

		BOOL InitOfPaste(CRefObject *pPasteBuf, 
			const BYTE **ppPlayload, UINT &iSize, INT iVersion);
	
		BOOL CheckCRC(void);
		void MakeCRC(void);

	};


	class CGspProFrame : public CProFrame
	{
	public :
		INT m_iGspVersion;   //GSP 版本号
		INT m_iGspDataType;  //数据类型 （参考 GSP 包数据类型定义)
		UINT16 m_iGspSeq;            //数据包的索引号	
		UINT8  m_iGspSubChn;        // 会话的子通道号,有1开始，命令子通道号指定为1, 非法通道
		UINT8 m_iGspExtraVal;
	public :
		INLINE BOOL IsCommand(void) const
		{
			return m_iGspDataType == GSP_PACKET_TYPE_CMD;
		}

		static CGspProFrame *Create(void)
		{
			return new CGspProFrame();
		}

	

		//检测是否有效
		BOOL CheckValid(void);

		virtual CRefObject *Clone(void) const
		{
			return NULL;
		}
	protected :
		CGspProFrame(void);
		virtual ~CGspProFrame(void);
	};




	class CGspCommand : public CGSPObject
	{
	private :
		union
		{
			StruGSPCommand stCmd;
			BYTE bBuffer[MAX_GSP_COMMAND_LEN];
		}m_unData;
		UINT m_iPlayloadSize;
		UINT m_iWholeSize;
	public :
		INT m_iGspVersion;   //GSP 版本号
		INT m_iGspDataType;  //数据类型 （参考 GSP 包数据类型定义)
		UINT16 m_iGspSeq;            //数据包的索引号	
		UINT8  m_iGspSubChn;        // 会话的子通道号,有1开始，命令子通道号指定为1, 非法通道
		UINT8 m_iGspExtraVal;

	public :
// 		INLINE static CGspCommand *Create(void)
// 		{
// 			return new CGspCommand();
// 		}

		

		StruGSPCommand &GetCommand(void)
		{
			return m_unData.stCmd;
		}

		INLINE BYTE *CommandPlayload(void)
		{
			return m_unData.stCmd.cPlayload;
		}

		INLINE UINT CommandPlayloadSize(void) const
		{
			return m_iPlayloadSize;
		}

		INLINE BYTE *GetWholeData(void)
		{
			return m_unData.bBuffer;
		}

		INLINE  UINT GetWholeDataSize(void)
		{
			return m_iWholeSize;
		}

		EnumErrno Parser( const char *pData, UINT iSize );
		EnumErrno Parser(CGspProFrame *pProFrame);
		void Reset(void);

	

		EnumErrno AddCommandPlayload(const void *pData, UINT iSize );


		template< typename playloadtype >
		static playloadtype &CastSubCommand(CGspCommand &csCmd )
		{
			playloadtype *pRet = (playloadtype *)csCmd.CommandPlayload(); 
			return *pRet;
		}

		virtual ~CGspCommand(void);
		CGspCommand(void);
		
	
		
	};

	
	

	
	
	/*
	*********************************************************************
	*
	*@brief : CGspTcpDecoder GSP TCP 协议解析器
	*
	*********************************************************************
	*/


	
	class CGspTcpDecoder : public CGSPObject
	{
	
	private :
		CGspProPacket *m_pCurPacket;
		CGspProFrame *m_pCurFrame; //当前包内容		
		CList m_csFinishList; //完成数据队列, 对象为  CGspProFrame *
		UINT m_iMaxPacketSize;
		INT m_iVersion;
		EnumErrno m_eAssert; //是否异常
	public :
		CGspTcpDecoder(void);
		virtual ~CGspTcpDecoder(void);

		EnumErrno Decode( CGSPBuffer *pInBuffer );
		CGspProFrame *Pop( void );
		void Reset(void);	
	};

	/*
	*********************************************************************
	*
	*@brief : CGspTcpEncoder GSP TCP 协议打包
	*
	*********************************************************************
	*/
	class CGspTcpEncoder : public CGSPObject
	{
	public :
		CGspTcpEncoder(void);
		virtual ~CGspTcpEncoder(void);
		static CGspProFrame * Encode( CGSPBuffer *pInBuffer, 
									StruGSPPacketHeader &stHeader );
		static CGspProFrame * Encode( CFrameCache *pSliceFrame, 
									StruGSPPacketHeader &stHeader );
		static CGspProFrame * Encode( const StruBaseBuf *vBuf, INT iBufNums, 
									StruGSPPacketHeader &stHeader );
		

	};



	

	
		/*
		*********************************************************************
		*
		*@brief : StruMediaInfoTable 的格式化函数
		*
		*********************************************************************
		*/
		class CGspMediaFormat : public CGSPObject
		{
			
		public : 
			static BOOL StructToInfo( const StruMediaInfoTable *pTable, CMediaInfo &csResult );

			static BOOL InfoToStruct( const CMediaInfo &csInfo,  StruMediaInfoTable *pResBuf, INT iBufSize  );
		protected :
			static EnumGSMediaType FormatSection( const   StruMediaInfoTable *pMediaInfo,
				StruGSMediaDescri &stDescri, INT &iChn, UINT iIdxStart, UINT iIdxEnd);
			static void FormatVideo( const StruMediaInfoTable *pMediaInfo,StruVideoDescri &stVideo, 
				UINT iIdxStart, UINT iIdxEnd);
			static void FormatAudio( const StruMediaInfoTable *pMediaInfo,StruAudioDescri &stAudio, 
				UINT iIdxStart, UINT iIdxEnd);
			static void FormatPicture( const StruMediaInfoTable *pMediaInfo,StruPictureDescri &stPicture, 
				UINT iIdxStart, UINT iIdxEnd);
			static void FormatOSD( const StruMediaInfoTable *pMediaInfo,StruOSDDescri &stOSD, 
				UINT iIdxStart, UINT iIdxEnd);
			static void FormatBin( const StruMediaInfoTable *pMediaInfo,StruBinaryDescri &stBin, 
				UINT iIdxStart, UINT iIdxEnd);
			static void FormatSysHeader( const StruMediaInfoTable *pMediaInfo,StruSysHeaderDescri &stSys, 
				UINT iIdxStart, UINT iIdxEnd);
		};




		
		//把本地错误码 转换为 GSP 错误号
		INT32 ErrnoLocal2Gsp(EnumErrno eGSSRet );

		//把GSP 错误号 转换为  本地错误码 
		EnumErrno ErrnoGsp2Local(INT32 iGspRet );



} //end namespace GSP

#endif


