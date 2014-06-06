#include "ES2PS.h"
#include "../ISocket.h"

namespace GSP
{

namespace SIP
{




#ifndef AV_WB32
#   define AV_WB32(p, d) do {               \
	((BYTE*)(p))[3] = (d);               \
	((BYTE*)(p))[2] = (d)>>8;            \
	((BYTE*)(p))[1] = (d)>>16;           \
	((BYTE*)(p))[0] = (d)>>24;           \
} while(0)
#endif

CES2PS::CES2PS(void)
{

	m_iPSIndex = 0;
	m_uiFrameRate = 25;
	m_stream_ID = VIDEO_STREAM_ID;
	m_stream_type = stream_type_none;
	m_stream_type_audio = stream_type_audio_g711;
	m_stream_ID_audio = AUDIO_STREAM_ID;
	 m_blFirstPES = true;
#ifdef __WRITE_FILE__
	m_pFile = fopen("PSFile.mp4","w+b");
	assert(m_pFile != NULL);
#endif // __WRITE_FILE__
}

CES2PS::~CES2PS(void)
{
	
#ifdef __WRITE_FILE__
	if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
#endif // __WRITE_FILE__
}

/**********************************************************
 函数功能：给数据流加PS头
 函数变量：BYTE *_pHeader 存放转换后的头变量
 UINT _iHeaderLen, 头的长度
 UINT _iFrameIndex,帧序号
 UINT _iFrameRate,帧速率
 DWORD _ulTimeStamp，时间戳
***********************************************************/
int CES2PS::make_ps_packet_header(BYTE *_pHeader,UINT _iFrameIndex,UINT _iFrameRate)
{

	//char pss[]={0x00,0x00,0x01,0xba,0x44,0x0a,0xc7,  0x22,0xc4,0x01,0x00,0x3a,0x9b,0xf8};
	//memcpy(_pHeader,pss,14);
	//return 14;
////////////////////////////////////////
	PS_HEADER_tag ePSHeader;
	
	float fInterval = 0;
	UINT64 ui64SCR = 0;
	fInterval =(float) (90000.0/_iFrameRate);
	ui64SCR = (UINT64)(fInterval * _iFrameIndex);
	//	ui64SCR=90*9+90000*2+90000*60*3;
	ePSHeader.setSystem_clock_reference_base(ui64SCR);

	ePSHeader.setProgram_mux_rate(6106); //(0xEA6); //6106
	ePSHeader.pack_stuffing_length = 0;

	memcpy(_pHeader,&ePSHeader,sizeof(PS_HEADER_tag));

	return sizeof(PS_HEADER_tag);
}

int CES2PS::make_pes_packet_header(BYTE *_pHeader,UINT _iFrameDataLen,UINT _iFrameIndex,UINT _iFrameRate,int stream_id, bool bFirst )
{
	PES_HEADER_tag ePESHeader;

	ePESHeader.packet_start_code_prefix[0] = 0x00;
	ePESHeader.packet_start_code_prefix[1] = 0x00;
	ePESHeader.packet_start_code_prefix[2] = 0x01;
	ePESHeader.stream_id = stream_id;//标志是视频还是音频

	ePESHeader.PES_packet_length[0] = (_iFrameDataLen+8) >> 8;
	ePESHeader.PES_packet_length[1] = (_iFrameDataLen+8) & 0xFF;

	ePESHeader.PES_scrambling_control = 0;
	ePESHeader.PES_priority = 0;
	
	ePESHeader.copyright = 0;
	ePESHeader.original_or_copy = 0;
	ePESHeader.data_alignment_indicator = 0;
	if( bFirst )
	{
		//ePESHeader.data_alignment_indicator = 0;
		ePESHeader.PTS_DTS_flags = 2; // 3 = 10
	}
	else
	{	
		ePESHeader.PTS_DTS_flags = 0;
	}
	ePESHeader.ESCR_flag = 0;
	ePESHeader.ES_rate_flag = 0;
	ePESHeader.DSM_trick_mode_flag = 0;
	ePESHeader.additional_copy_info_flag = 0;
	ePESHeader.PES_CRC_flag = 0;
	ePESHeader.PES_extension_flag = 0;
	
	ePESHeader.PES_header_data_length = 5;
	
	
	memcpy(_pHeader,&ePESHeader,sizeof(PES_HEADER_tag));

	PTS_tag ePTS;
	
	ePTS.fix_bit = 0x2;
//	ePTS.fix_bit2 = 0x2;
	float fInterval = 0;
	UINT64 ui64SCR = 0;
	fInterval = (float)( 90000.0/_iFrameRate);//33.3333333333;
	ui64SCR = (UINT64)(fInterval * _iFrameIndex);
   //填写PTS，DTS
	ePTS.setPTS(ui64SCR,ui64SCR); //+3600

	memcpy(_pHeader + sizeof(PES_HEADER_tag),&ePTS, 5); //sizeof(PTS_tag));


	return sizeof(PES_HEADER_tag)+5; //sizeof(PTS_tag);//19字节
	
}

#if 0
static const BYTE reserveddata[]={0x00,0x00,0x01,0xbc,0x00,0x18,0xe1,0xff,0x00,0x00,
								0x00,0x08,
								0x1b,0xe0,0x00,0x06,0x0a,0x04,0x65,0x6e,0x67,0x00,
								0x90,0xc0,0x00,0x00,
								0x2e,0xb9,0x0f,0x3d};
#endif

//添加系统头
int CES2PS::make_sys_packet_header(BYTE *_pHeader,UINT _iDataLen,UINT _iFrameIndex,UINT _iFrameRate,int stream_id)
{
	PS_SYSTEM_HEADER_tag ePS_Sys_Header;

	ePS_Sys_Header.header_length[0] = (12>>8)&0xff;
	ePS_Sys_Header.header_length[1] = 12&0xff;

	ePS_Sys_Header.rate_bound1 = (0x667A>>15)&0x7f;  //0x667A
	ePS_Sys_Header.rate_bound2 = (0x667A>>7)&0xff;
	ePS_Sys_Header.rate_bound3 = (0x667A)&0x7f;
    memcpy(_pHeader,&ePS_Sys_Header,sizeof(PS_SYSTEM_HEADER_tag));
	return sizeof(PS_SYSTEM_HEADER_tag);//12字节
}


/*
int CES2PS::ESconvertToPS(BYTE * buffer,int len,BYTE *pOutBuf,int iOutBufSize, int *iOutLen,DWORD uiFrameRate,int stream_ID,int stream_type)
{
	int iRet = 0, iWiteLen = 0, iCurFrameSize = 0, iPESHeaderLen = 0, iBuffOffset = 0;

	bool blFirstPES = true;
	PSM_tag psm;
	BYTE *pPSHeader = new BYTE[sizeof(PS_HEADER_tag)];
	BYTE PESBuf[40];


	//给对应的视频或者音频填写stream_type和stream_id
	//依次给视频数据添加PES头，SYS头，PS头
	psm.stream_type = stream_type;

	if ((iWiteLen = make_ps_packet_header(pPSHeader,sizeof(PS_HEADER_tag),iPSIndex,uiFrameRate)) > 0)
	{
		if ((iRet+iWiteLen)<iOutBufSize)
		{
			memcpy(pOutBuf,pPSHeader,iWiteLen);
			iRet = iWiteLen;
		}else
			return -1;
	}

	if((buffer[4] & 0x1F) == 0x07)//SPS
	{
		if ((iRet + sizeof(PSM_tag))<iOutBufSize)
		{
			memcpy(pOutBuf+iRet,&psm,sizeof(PSM_tag));
			iRet += sizeof(PSM_tag);
		} else
			return -1;
	}

	blFirstPES = true;
	iPESHeaderLen = sizeof(PES_HEADER_tag) + sizeof(PTS_tag);

	if ((iWiteLen = make_pes_packet_header(PESBuf,sizeof(PES_HEADER_tag) + sizeof(PTS_tag),len,iPSIndex,blFirstPES,uiFrameRate,0,stream_ID)) > 0)
	{
		if ((iRet+iWiteLen)<iOutBufSize)
		{
			memcpy(pOutBuf+iRet,PESBuf,iWiteLen);
			iRet += iWiteLen;
		}
		else
			return -1;
	}
	memcpy(pOutBuf+iRet,buffer ,len);
	iRet += len;


	iPSIndex ++;	
	if (pPSHeader != NULL)
	{
		delete[] pPSHeader;
		pPSHeader = NULL;
	}
	if (iRet>iOutBufSize)
		return -1;
	*iOutLen = iRet;
	return 0;
}
*/

int CES2PS::ESConvertToPs(const BYTE **pInpuf, int *iolen, int iframelen, BYTE *pOutHeaderBuf, int iOutBufSize )
{
	int iRet = 0, iWiteLen = 0;
	//iCurFrameSize = 0, iPESHeaderLen = 0, iBuffOffset = 0;

	const BYTE *pSrc = *pInpuf;
	int len = *iolen;
	
	PS_HEADER_tag psHeader;	
	BYTE PESBuf[64];
	BYTE *pPSHeader = (BYTE *)&psHeader;

	//给对应的视频或者音频填写stream_type和stream_id
	//依次给视频数据添加PES头，SYS头，PS头
	

	if ((iWiteLen = make_ps_packet_header(pPSHeader,m_iPSIndex,m_uiFrameRate)) > 0)
	{
		if ((iRet+iWiteLen)<iOutBufSize)
		{
			memcpy(pOutHeaderBuf,pPSHeader,iWiteLen);
			iRet = iWiteLen;
		}else
			return -1;
	}
	bool bFirst = true;
	if( (m_stream_type == stream_type_h264 && h264_probe_is_sps(pSrc, len) ) 
		|| (m_stream_type == stream_type_mpeg4 && mpeg4video_probe_is_iframe(pSrc, len) )  )
	{
		
		
		PS_SYSTEM_HEADER_tag stSysHeader;

		if ((iWiteLen = make_sys_packet_header((BYTE*)&stSysHeader,iframelen,
							m_iPSIndex,m_uiFrameRate,  m_stream_ID)) > 0)
		{
			if ((iRet+iWiteLen)<iOutBufSize)
			{
				memcpy(pOutHeaderBuf+iRet,&stSysHeader,iWiteLen);
				iRet += iWiteLen;
			}else
				return -1;
		}
		
		PSM_tag psm;
		psm.stream_type_video = m_stream_type;
		psm.stream_type_audio = m_stream_type_audio;

// #if 1
// 		static const BYTE psmdata[]=
// 			{ 0x00, 0x00, 0x01, 0xbc, 0x00, 0x1e, 0xe1, 0xff,  0x00, 0x00, 
// 			  0x00, 0x18, 
// 			  0x1b, 0xe0, 0x00, 0x0c, 0x2a, 0x0a, 0x7f, 0xff, 0x00, 0x00, 0x07, 0x08,
// 			  0x1f, 0xfe, 0xa0, 0x5a, 
// 			  0x90, 0xc0, 0x00, 0x00,
// 		      0x00, 0x00, 0x00, 0x00}; //36
// #else
// 		static const BYTE psmdata[]={0x00,0x00,0x01,0xbc,0x00,0x18,0xe1,0xff,0x00,0x00,
// 								0x00,0x08,
// 								0x1b,0xe0,0x00,0x06, 0x0a,0x04,0x65,0x6e,0x67,0x00,
// 								0x90,0xc0,0x00,0x00,
// 								0x0,0x0,0x0,0x0};
// #endif
// 		if(0 && m_stream_type == stream_type_h264 )
// 		{
// 			if (  (int) (iRet + sizeof(psmdata)) < iOutBufSize)
// 			{
// 				memcpy(pOutHeaderBuf+iRet,psmdata,sizeof(psmdata));
// 				iRet += sizeof(psmdata);
// 			} else
// 				return -1;
// 		}
// 		else
		{
			if (  (int) (iRet + sizeof(psm)) < iOutBufSize)
			{
				memcpy(pOutHeaderBuf+iRet,&psm,sizeof(psm));
				iRet += sizeof(psm);
			} else
				return -1;
		}
/*
		if(0 &&  m_stream_type == stream_type_h264 )
		{			
			//sps
			iframelen -= 40;
			if ((iWiteLen = make_pes_packet_header(PESBuf,40,m_iPSIndex,
				m_uiFrameRate,m_stream_ID, bFirst)) > 0)
			{
				if ((iRet+iWiteLen)<iOutBufSize)
				{
					memcpy(pOutHeaderBuf+iRet,PESBuf,iWiteLen);
					iRet += iWiteLen;
				}
				else
					return -1;
			}

			bFirst = false;		
			iWiteLen = 40;
			if ((iRet+iWiteLen)<iOutBufSize)
				{
					memcpy(pOutHeaderBuf+iRet,pSrc,iWiteLen);
					iRet += iWiteLen;
				}
				else
					return -1;

			len -= 40; 
			pSrc += 40;


			//pps---
			iframelen -= 8;
			if ((iWiteLen = make_pes_packet_header(PESBuf,8,m_iPSIndex,
				m_uiFrameRate,m_stream_ID, bFirst)) > 0)
			{
				if ((iRet+iWiteLen)<iOutBufSize)
				{
					memcpy(pOutHeaderBuf+iRet,PESBuf,iWiteLen);
					iRet += iWiteLen;
				}
				else
					return -1;
			}
			
			iWiteLen = 8;
			if ((iRet+iWiteLen)<iOutBufSize)
			{
				memcpy(pOutHeaderBuf+iRet,pSrc,iWiteLen);
				iRet += iWiteLen;
			}
			else
				return -1;

			pSrc += 8;
			len -= 8; 

			*pInpuf = pSrc;
			*iolen = len;
		}
*/	
	}
	


	if ((iWiteLen = make_pes_packet_header(PESBuf,iframelen,m_iPSIndex,
											m_uiFrameRate,m_stream_ID, bFirst)) > 0)
	{
		if ((iRet+iWiteLen)<iOutBufSize)
		{
			memcpy(pOutHeaderBuf+iRet,PESBuf,iWiteLen);
			iRet += iWiteLen;
		}
		else
			return -1;
	}
	m_blFirstPES = false;
	m_iPSIndex ++;	
	GS_ASSERT(iRet<=iOutBufSize);
	return iRet;
}

int CES2PS::ESConvertMakePES(int iframelen, BYTE *pOutHeaderBuf, int iOutBufSize, bool bAudio)
{
	int iWriteLen = -1;

	if( iOutBufSize <(sizeof(PES_HEADER_tag) + sizeof(PTS_tag) ) )
	{
		GS_ASSERT(0);
		return -1;
	}

	if( bAudio )
	{

		if ((iWriteLen = make_pes_packet_header(pOutHeaderBuf,iframelen,m_iPSIndex,
			m_uiFrameRate,m_stream_ID_audio,true)) > 0)
		{

		}	
	} 
	else 
	{
		if ((iWriteLen = make_pes_packet_header(pOutHeaderBuf,iframelen,m_iPSIndex,
			m_uiFrameRate,m_stream_ID,false)) > 0)
		{

		}	
	}

	return iWriteLen;
}

//检测是否为H264 码流
#define AVPROBE_SCORE_MAX 30


BOOL CES2PS::h264_probe_is_video(const BYTE *buf, int buf_size, const BYTE **ppStart)
{
	UINT32 code= 0xffffffff;
	int iType = 0;
	int i;
	if( buf_size>512 )
		buf_size = 512;

	for(i=0; i<buf_size; i++) {
		code = (code<<8) + buf[i];
		if ((code & 0xffffff00) == 0x100)
		{		
			if( (code&0x60)==0x60)
			{
				*ppStart =  &buf[i-4];
				return TRUE;
			}			
		}
	}
	return FALSE;
}

BOOL CES2PS::h264_probe_is_sps(const BYTE *buf, int buf_size)
{
	UINT32 code= 0xffffffff;
	int sps=0, pps=0, idr=0, res=0, sli=0;
	int i;

	if( buf_size>512 )
		buf_size = 512;

	for(i=0; i<buf_size; i++) {
		code = (code<<8) + buf[i];
		if ((code & 0xffffff00) == 0x100) {
			int ref_idc= (code>>5)&3;
			int type   = code & 0x1F;
			static const UINT8 ref_zero[32]= {
				2, 0, 0, 0, 0,-1, 1,-1,
				-1, 1, 1, 1, 1,-1, 2, 2,
				2, 2, 2, 0, 2, 2, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2
			};

			if(code & 0x80) //forbidden bit  ps header
				continue;
				
			if(ref_zero[type] == 1 && ref_idc)
				return 0;
			if(ref_zero[type] ==-1 && !ref_idc)
				return 0;
			if(ref_zero[type] == 2)
				res++;

			switch(type){
			case     1:   sli++; break;
			case     5:   idr++; break;
			case     7:
				{					

					if(buf[i+2]&0x03)
						return 0;					
					return TRUE;				
				}
				break;
			case     8:   pps++; break;
			}
		}
	}
	return FALSE;
}

int CES2PS::h264_probe(const BYTE *buf, int buf_size )
{
	UINT32 code= 0xffffffff;
	int sps=0, pps=0, idr=0, res=0, sli=0;
	int i;

	for(i=0; i<buf_size; i++){
		code = (code<<8) + buf[i];
		if ((code & 0xffffff00) == 0x100) {

			
			

			int ref_idc= (code>>5)&3;
			int type   = code & 0x1F;
			static const UINT8 ref_zero[32]={
				2, 0, 0, 0, 0,-1, 1,-1,
				-1, 1, 1, 1, 1,-1, 2, 2,
				2, 2, 2, 0, 2, 2, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2
			};

			if(code & 0x80) //forbidden bit, ps header
				continue;
			
			if(ref_zero[type] == 1 && ref_idc)
				return 0;
			if(ref_zero[type] ==-1 && !ref_idc)
				return 0;

			
			if(ref_zero[type] == 2)
				res++;
			
			switch(type){
			case     1:   sli++; break;
			case     5:   idr++; break;
			case     7:
				{					

					if(buf[i+2]&0x03)
						return 0;
					sps++;					
				}
				break;
			case     8:   pps++; break;
			}
		}
	}
	if(sps && pps && (idr||sli>0) && res<(sps+pps+idr))
		return AVPROBE_SCORE_MAX/2+1; // +1 for .mpg
	return 0;
}


//检测是否为MPEMG4 码流
#define MP4_VISUAL_OBJECT_START_CODE       0x000001b5
#define MP4_VOP_START_CODE                 0x000001b6
#define MP4_I_VOP 0

BOOL CES2PS::mpeg4video_probe_is_iframe(const BYTE *buf, int buf_size)
{
	UINT32 temp_buffer = 0xffffffff;
	int VO=0, VOL=0, VOP = 0, VISO = 0, res=0;
	int i;


	for(i=0; i<buf_size; i++){
		temp_buffer = (temp_buffer<<8) + buf[i];
		if ((temp_buffer & 0xffffff00) != 0x100)
			 continue;

		if (temp_buffer == MP4_VOP_START_CODE)  
		{
			int coding_type = (buf[i+1])>>6;			
			return coding_type==MP4_I_VOP ? TRUE : FALSE ;				
		}		
	}
	return FALSE;
}


int CES2PS::mpeg4video_probe(const BYTE *buf, int buf_size)
{
	UINT32 temp_buffer=  0xffffffff;
	int VO=0, VOL=0, VOP = 0, VISO = 0, res=0;
	int i;


	for(i=0; i<buf_size; i++){
		temp_buffer = (temp_buffer<<8) + buf[i];
		if ((temp_buffer & 0xffffff00) != 0x100)
			continue;

		if (temp_buffer == MP4_VOP_START_CODE)  
		{
			VOP++;			
		}
		else if (temp_buffer == MP4_VISUAL_OBJECT_START_CODE)   
		{
			VISO++;			
		}
		else if (temp_buffer < 0x120)                              VO++;
		else if (temp_buffer < 0x130)                              VOL++;
		else if (   !(0x1AF < temp_buffer && temp_buffer < 0x1B7)
			&& !(0x1B9 < temp_buffer && temp_buffer < 0x1C4)) res++;
	}

	if (VOP >= VISO && VOP >= VOL && VO >= VOL && VOL > 0 && res==0)
		return VOP+VO > 3 ? AVPROBE_SCORE_MAX/2 : AVPROBE_SCORE_MAX/4;
	return 0;
}

//检测是已经是PS 流
#define PS_SYSTEM_HEADER_START_CODE    ((unsigned int)0x000001bb)
#define PS_PACK_START_CODE             ((unsigned int)0x000001ba)
#define PS_AUDIO_ID 0xc0
#define PS_VIDEO_ID 0xe0
#define PS_PROGRAM_STREAM_MAP 0x1bc
#define PS_PRIVATE_STREAM_1   0x1bd
#define PS_PADDING_STREAM     0x1be
#define PS_PRIVATE_STREAM_2   0x1bf
static int check_pes(const BYTE *p,const BYTE *end){
	int pes1;
	int pes2=      (p[3] & 0xC0) == 0x80
		&& (p[4] & 0xC0) != 0x40
		&&((p[4] & 0xC0) == 0x00 || (p[4]&0xC0)>>2 == (p[6]&0xF0));

	for(p+=3; p<end && *p == 0xFF; p++);
	if((*p&0xC0) == 0x40) p+=2;
	if((*p&0xF0) == 0x20){
		pes1= p[0]&p[2]&p[4]&1;
	}else if((*p&0xF0) == 0x30){
		pes1= p[0]&p[2]&p[4]&p[5]&p[7]&p[9]&1;
	}else
		pes1 = *p == 0x0F;

	return pes1||pes2;
}

INT CES2PS::TestPSStream( const BYTE *p, int iSize, const BYTE **ppStart, INT &iDataSize)
{
	UINT32 code= -1;
	int sys=0, pspack=0, priv1=0, vid=0, audio=0, invalid=0;
	int i;
	int score=0;

	for(i=0; i<iSize; i++){
		code = (code<<8) + p[i];
		if ((code & 0xffffff00) == 0x100) {
			int len= p[i+1] << 8 | p[i+2];
			
			if(code == PS_PACK_START_CODE)
			{
				*ppStart = &p[i-3];
				iDataSize=sizeof(PS_HEADER_tag);
				PS_HEADER_tag tag;
				memcpy(&tag, *ppStart, sizeof(tag));
				iDataSize += tag.pack_stuffing_length;
				return PS_HEADER_PK_START;
			} 
			else if(code == PS_SYSTEM_HEADER_START_CODE) 
			{
				*ppStart = &p[i-3];
				UINT16 l;
				memcpy(&l, &p[i+1], sizeof(l)); //拷贝长度

				l = COSSocket::Int16N2H(l);
				iDataSize = 6+l;
				return PS_HEADER_SMH;
			}
			else if( code == PS_PROGRAM_STREAM_MAP )
			{
				*ppStart = &p[i-3];
				UINT16 l;
				memcpy(&l, &p[i+1], sizeof(l)); //拷贝长度

				l = COSSocket::Int16N2H(l);
				iDataSize = 6+l;
				return PS_HEADER_PSM;
			}
			else if( 0==(code&0x0f) && (code & 0xf0)  )
			{
				*ppStart = &p[i-3];
				UINT16 l;
				l = p[i+5];
				iDataSize = 9+l;
				return PS_HEADER_PES;
			}
		}
	}
	return PS_HEADER_NONE;
}

BOOL CES2PS::ParserPSM( const BYTE *p, int iSize, std::vector<StruPSStreamInfo> &vStreamInfo)
{

	PSM_tag tag;
	
	
	INT16 l;
	memcpy(&l, p+4, 2); //拷贝长度
	l = COSSocket::Int16N2H(l);

	INT16 iInfoLen;

	p += 8;
	iSize -= 8;
	l -= 2;
	GS_ASSERT_RET_VAL(iSize>=2 && l>=2 , FALSE);

	memcpy(&iInfoLen, p, sizeof(iInfoLen)); //拷贝 program_stream_info 信息长度
	iInfoLen = COSSocket::Int16N2H(iInfoLen);
	p += 2;
	l -= 2;
	iSize -= 2;
	GS_ASSERT_RET_VAL( iSize>=iInfoLen && l>=iInfoLen, FALSE );

	//跳过 program_stream_info
	p += iInfoLen;
	l -= iInfoLen;
	iSize -= iInfoLen;

	GS_ASSERT_RET_VAL(iSize>=2 && l>=2 , FALSE);


	memcpy(&iInfoLen, p, sizeof(iInfoLen)); //拷贝 elementary_stream_map_length 信息长度
	iInfoLen = COSSocket::Int16N2H(iInfoLen);
	p += 2;
	l -= 2;
	iSize -= 2;
	GS_ASSERT_RET_VAL( iSize>=iInfoLen && l>=iInfoLen, FALSE );

	INT16 stream_info_length;
	StruPSStreamInfo stInfo;

	GS_ASSERT_RET_VAL( (iInfoLen!=0 && iInfoLen>=4),  FALSE );

	for( ;/*iInfoLen>=4 && */l>=8;  )
	{
		stInfo.iStreamType = *p;
		p++;		
		 stInfo.iStreamId = *p;
		p++;
		memcpy(&stream_info_length, p, 2); //拷贝 stream_info_length 信息长度
		stream_info_length = COSSocket::Int16N2H(stream_info_length);
		p += 2;
		l -= 4;
		iSize -= 4;
		iInfoLen -= 4;

		//跳过 program_stream_info
		p += stream_info_length;
		l -= stream_info_length;
		iSize -= stream_info_length;
		iInfoLen -= stream_info_length;	

		GS_ASSERT( l>=4 /*l>=iInfoLen && (iInfoLen==0 || iInfoLen>=4) */);
		vStreamInfo.push_back(stInfo);
	}	
	return TRUE;
}

BOOL CES2PS::PSStreamGetPES( const BYTE *pPS,int iSize,
							const BYTE **pDataStart, INT &iDataSize, INT &iStreamId,
							UINT64 &iPTS, UINT64 &iDTS )
{
	

	PES_HEADER_tag tag;	

	int iInputSize = iSize;

	GS_ASSERT_RET_VAL(iSize>=sizeof(PES_HEADER_tag), FALSE);

	memcpy( &tag, pPS, sizeof(tag) );
	iStreamId = tag.stream_id;

	pPS += sizeof(tag);
	iSize -= sizeof(tag);

	UINT16 iPktLen = 0; //包的长度
	memcpy(&iPktLen, tag.PES_packet_length, sizeof(iPktLen)); //包的长度
	iPktLen = COSSocket::Int16N2H(iPktLen);


//	if( tag.stream_id== 0xc0)
//	{
//		GS_ASSERT(0);
//	}

	if( iPktLen>iInputSize || tag.PES_header_data_length>iPktLen  )
	{
		//GS_ASSERT(0);
		return FALSE;
	}


	iPktLen -= (3+tag.PES_header_data_length);
	GS_ASSERT_RET_VAL(iSize>=tag.PES_header_data_length, FALSE);
	iPTS = MAX_UINT64;
	iDTS = MAX_UINT64;
	if( tag.PTS_DTS_flags && tag.PES_header_data_length>=5 )
	{
		//有DTS, 或PTS
		PTS_tag ptsTag;
		bzero(&ptsTag, sizeof(ptsTag));
		GS_ASSERT_RET_VAL( 5<=tag.PES_header_data_length, FALSE); //最少有一个时间
		int iTemp = MIN(tag.PES_header_data_length, sizeof(ptsTag));
		memcpy(&ptsTag, pPS, iTemp);

		pPS += tag.PES_header_data_length;
		iSize -= tag.PES_header_data_length;

		if( ptsTag.fix_bit == 0x2 || ptsTag.fix_bit == 0x3 )
		{
			// PTS
			 ptsTag.getPTS(iPTS);		
			if( ptsTag.fix_bit == 0x3 && ptsTag.fix_bit2 == 0x01)
			{
				//DTS
				GS_ASSERT_RET_VAL( 10<=tag.PES_header_data_length, FALSE); 
				ptsTag.getDTS(iDTS);
			}
		}		
	}
	else
	{
		pPS += tag.PES_header_data_length;
		iSize -= tag.PES_header_data_length;
	}
	*pDataStart = pPS;
	iDataSize = iPktLen;
	return TRUE;
	
	
}

int CES2PS::mpegps_probe(const BYTE *buf, int buf_size)
{
	UINT32 code= 0xffffffff;
	int sys=0, pspack=0, priv1=0, vid=0, audio=0, invalid=0;
	int i;
	int score=0;

	for(i=0; i<buf_size; i++){
		code = (code<<8) + buf[i];
		if ((code & 0xffffff00) == 0x100) {
			/*code= 0xffffffff;*/
			int len= buf[i+1] << 8 | buf[i+2];
			int pes= check_pes(buf+i, buf+buf_size);

			if(code == PS_SYSTEM_HEADER_START_CODE) sys++;
			else if(code == PS_PACK_START_CODE)     pspack++;
			else if((code & 0xf0) == PS_VIDEO_ID &&  pes) vid++;
			// skip pes payload to avoid start code emulation for private
			// and audio streams
			else if((code & 0xe0) == PS_AUDIO_ID &&  pes) {audio++; i+=len;}
			else if(code == PS_PRIVATE_STREAM_1  &&  pes) {priv1++; i+=len;}
			else if(code == 0x1fd             &&  pes) vid++; //VC1

			else if((code & 0xf0) == PS_VIDEO_ID && !pes) invalid++;
			else if((code & 0xe0) == PS_AUDIO_ID && !pes) invalid++;
			else if(code == PS_PRIVATE_STREAM_1  && !pes) invalid++;
		}
	}

	if(vid+audio > invalid+1)     /* invalid VDR files nd short PES streams */
		score= AVPROBE_SCORE_MAX/4;

	//av_log(NULL, AV_LOG_ERROR, "%d %d %d %d %d %d len:%d\n", sys, priv1, pspack,vid, audio, invalid, p->buf_size);
	if(sys>invalid && sys*9 <= pspack*10)
		return pspack > 2 ? AVPROBE_SCORE_MAX/2+2 : AVPROBE_SCORE_MAX/4; // +1 for .mpg
	if(pspack > invalid && (priv1+vid+audio)*10 >= pspack*9)
		return pspack > 2 ? AVPROBE_SCORE_MAX/2+2 : AVPROBE_SCORE_MAX/4; // +1 for .mpg
	if((!!vid ^ !!audio) && (audio > 4 || vid > 1) && !sys && !pspack && buf_size>2048 && vid + audio > invalid) /* PES stream */
		return (audio > 12 || vid > 3) ? AVPROBE_SCORE_MAX/2+2 : AVPROBE_SCORE_MAX/4;

	//02-Penguin.flac has sys:0 priv1:0 pspack:0 vid:0 audio:1
	//mp3_misidentified_2.mp3 has sys:0 priv1:0 pspack:0 vid:0 audio:6
	return score;
}

BOOL CES2PS::IsExistStreamHeader(const BYTE *pSrc, int len)
{
	UINT32 code= 0xffffffff;
	int iType = 0;
	int i;

	for(i=0; i<len; i++) {
		code = (code<<8) + pSrc[i];
		if ((code & 0xffffff00) == 0x100)
		{			
			if( (code&0x0F) )
			{			
				return TRUE;
			}			
		}
	}
	return FALSE;
}

BOOL CES2PS::IsTestStreamType(const BYTE *pSrc, int len, int istream_type, const BYTE **ppStart)
{
	*ppStart = pSrc;
	if( istream_type == stream_type_h264 )
	{
		if( h264_probe_is_video(pSrc, len ,ppStart) > 0 )
		{
			return TRUE;
		}
		return FALSE;
	}
	else if(  istream_type == stream_type_mpeg4)
	{
		
		if( mpeg4video_probe(pSrc, len) > 0)
		{
			return TRUE;
		}
		return FALSE;
	}
	else if(  istream_type == stream_type_ps)
	{
		if( mpegps_probe(pSrc, len) > 0)
		{
			return TRUE;
		}
		return FALSE;
	}
	GS_ASSERT(0);
	return FALSE;
}

int CES2PS::TestStreamType(const BYTE *pSrc, int len)
{
	int iPs = 0, iH264 = 0, iMpeg4 = 0;
	iPs = mpegps_probe(pSrc, len);
	iH264 = h264_probe(pSrc, len);
	iMpeg4 = mpeg4video_probe(pSrc, len);
	if( iPs>1 )
	{
		return stream_type_ps;
	}
	if( iH264>iMpeg4 )
	{
		return stream_type_h264;
	}
	if( iMpeg4>1 )
	{
		return stream_type_mpeg4;
	}
	return stream_type_none;	
}








#if 0






CPSStreamDecoder::CPSStreamDecoder(void)
{
	m_pCache = NULL;
	m_iCacheSize = (1L<<10)*128; // 16 KBytes
	m_iR = 0;
	m_iW = 0;
	bzero(&m_stAudioStreaInfo, sizeof(m_stAudioStreaInfo));
	bzero(&m_stVideoStreaInfo, sizeof(m_stVideoStreaInfo));
}

CPSStreamDecoder::~CPSStreamDecoder(void)
{
	if( m_pCache )	
	{
		delete[] m_pCache;
	}	
}

bool CPSStreamDecoder::Init(void)
{
	GS_ASSERT(m_pCache==NULL);
	m_pCache = new BYTE[m_iCacheSize];
	if( m_pCache == NULL )
	{
		return false;
	}
	return true;
}

bool CPSStreamDecoder::Decode(const BYTE *pData, int iSize )
{
	GS_ASSERT(m_iW==0);
	if( iSize> m_iCacheSize )
	{
		BYTE *pNewCach = new BYTE[iSize+1024];
		if( pNewCach == NULL )
		{
			return false;
		}
		delete[] m_pCache;
		m_pCache = pNewCach;
		m_iCacheSize = iSize+1024;
	}
	memcpy(m_pCache,pData, iSize);
	m_iR = 0;
	m_iW = iSize;
	return true;
}

int CPSStreamDecoder::GetMediaFrame(CPSStreamDecoder::StruFramePktInfo &stInfo)
{
	bzero(&stInfo, sizeof(stInfo));
	if( m_iW < 4 )
	{
		m_iR = 0;
		m_iW = 0;
		return 1;
	}

	INT iT;
	const BYTE *p = m_pCache+m_iW;
	const BYTE *pp = p;
	INT iDataSize = m_iW-m_iR;
	INT iS = iDataSize;
	while( iS>6 )
	{

		iT = TestPSStream(p, iS, &pp,  iDataSize );

		if( iT = PS_HEADER_PK_START )
		{
			stInfo.pPSData = pp;

			//新的数据帧
			//跳过 BA 包头 
			iS -= (pp-p);
			p = pp;
			p += iDataSize;
			iS -= iDataSize;
			iT = TestPSStream(p, iS, &pp,  iDataSize );
			m_iR = p-m_pCache;

			if( iT == PS_HEADER_NONE )
			{
				//出错
				GS_ASSERT(0);
				m_iR = 0;
				m_iW = 0;
				return -1;
			}
			if( iT == PS_HEADER_PES )
			{
				//数据帧
				break;
			}

			if( iT == PS_HEADER_PSM )
			{
				//系统头
				stInfo.bKey = TRUE;
			}
			else if( iT==  PS_HEADER_SMH )
			{
				//系统头， 是I帧
				stInfo.bKey = TRUE;
				m_bPSStreamLosePkt = false;

				iT = ParserPSM(p, iS, m_vPSStreamInfo);
				if( !iT  || m_vPSStreamInfo.empty() )
				{
					GS_ASSERT(0);
					m_iR = 0;
					m_iW = 0;
					m_bPSStreamLosePkt = TRUE;
					return -1;
				}
				for( UINT i = 0; i<m_vPSStreamInfo.size(); i++ )
				{
					if( m_vPSStreamInfo[i].eMediaType == GS_MEDIA_TYPE_VIDEO )
					{
						m_stVideoStreaInfo = m_vPSStreamInfo[i];
					}
					else if( m_vPSStreamInfo[i].eMediaType == GS_MEDIA_TYPE_AUDIO )
					{
						m_stVideoStreaInfo = m_vPSStreamInfo[i];
					}
					//其他不理会
				}
			}
			else 
			{
				GS_ASSERT(0);
				m_iR = 0;
				m_iW = 0;
				return -1;
			}
			//跳过
			iS -= (pp-p);
			p = pp;

			p += iDataSize;
			iS -= iDataSize;
			m_iR = pp-m_pCache;
		} // end if
		else if( iT == PS_HEADER_PES )
		{
			if( m_bPSStreamLosePkt || !m_vPSStreamInfo.empty() )
			{
				pp = p;
				SkipPES(&pp, iS);
				iS -= (pp-p);
				m_iR = pp-m_pCache;
				p = pp;	
				iT = PS_HEADER_NONE;
				continue;
			}
			else
			{ 
				break;
			}
		}
		else
		{
			m_bPSStreamLosePkt = TRUE;
			m_iR = 0;
			m_iW = 0;
			return -1;
		}

	} //end while(1)

	

	if( iT != PS_HEADER_PES )
	{
		GS_ASSERT(0);
		m_iR = 0;
		m_iW = 0;
		return -1;
	}


	INT iStreamId = -1;
	UINT64 iPTS = MAX_UINT64; 
	UINT64 iDTS = MAX_UINT64;

	//读取PES 的数据
	iT =  PSStreamGetPES(p, iS,&pp, iDataSize, iStreamId, iPTS, iDTS);
	if( !iT )
	{
		GS_ASSERT(0);
		m_iR = 0;
		m_iW = 0;
		m_bPSStreamLosePkt = TRUE;
		return -1;
	}
	StruPSPktInfo *pInfo;

	m_iR = (pp-m_pCache)+iDataSize;

	GS_ASSERT(m_iR<=m_iW);

	

	if( iStreamId == m_stVideoStreaInfo.iStreamId )
	{
		//视频
		pInfo =  &m_stVideoStreaInfo;		
	}
	else if( iStreamId == m_stAudioStreaInfo.iStreamId )
	{
		//音频
		pInfo =  &m_stAudioStreaInfo;		
	}

	if( pInfo )
	{
		stInfo.eCodeId = pInfo->eCodeId;
		stInfo.eMediaType = pInfo->eMediaType;
		stInfo.iBodyDataSize = iDataSize;
		stInfo.pBodyData  = pp;
		stInfo.iPSDataSize = pp-stInfo.pPSData+iDataSize;
	}
	return 0;

}


INT CPSStreamDecoder::TestPSStream( const BYTE *p, int iSize, const BYTE **ppStart, INT &iDataSize)
{
	UINT32 code= -1;
	int sys=0, pspack=0, priv1=0, vid=0, audio=0, invalid=0;
	int i;
	int score=0;

	for(i=0; i<iSize; i++){
		code = (code<<8) + p[i];
		if ((code & 0xffffff00) == 0x100) {
			int len= p[i+1] << 8 | p[i+2];

			if(code == PS_PACK_START_CODE)
			{
				*ppStart = &p[i-3];
				iDataSize=sizeof(PS_HEADER_tag);
				return PS_HEADER_PK_START;
			} 
			else if(code == PS_SYSTEM_HEADER_START_CODE) 
			{
				*ppStart = &p[i-3];
				UINT16 l;
				memcpy(&l, &p[i+1], sizeof(l)); //拷贝长度

				l = ntohs(l);
				iDataSize = 6+l;
				return PS_HEADER_SMH;
			}
			else if( code == PS_PROGRAM_STREAM_MAP )
			{
				*ppStart = &p[i-3];
				UINT16 l;
				memcpy(&l, &p[i+1], sizeof(l)); //拷贝长度

				l = ntohs(l);
				iDataSize = 6+l;
				return PS_HEADER_PSM;
			}
			else if( 0==(code&0x0f) && (code & 0xf0)  )
			{
				*ppStart = &p[i-3];
				UINT16 l;
				l = p[i+5];
				iDataSize = 9+l;
				return PS_HEADER_PES;
			}
		}
	}
	return PS_HEADER_NONE;
}

#endif

} //end namespace RTP

} //end namespace GSP
