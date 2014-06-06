/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : ES2PS.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/11/28 8:47
Description: PS 流封装函数
********************************************
*/

#ifndef _GS_H_ES2PS_H_
#define _GS_H_ES2PS_H_
#include "../GSPObject.h"
#include "GSMediaDefs.h"

namespace GSP
{

namespace SIP
{





#define AUDIO_STREAM_ID 0xC0
#define VIDEO_STREAM_ID 0xE0

#define  stream_type_none 0
#define  stream_type_ps  0xFFFF

#define  stream_type_h264   0x1B
#define  stream_type_mpeg4  0x10
#define  stream_type_svac   0x80

#define  stream_type_audio_g711 0x90
#define  stream_type_audio_g722	0x92
#define  stream_type_audio_g723 0x93
#define  stream_type_audio_g729 0x99
#define  stream_type_audio_svac 0x9B

typedef struct _StruPSStreamInfo
{
	INT iStreamId;
	INT iStreamType;
}StruPSStreamInfo;

#ifdef _WIN32 
#pragma pack( push,1 )
#endif



typedef struct PS_HEADER_tag//size =14
{
	BYTE pack_start_code[4];		//'0x000001BA'

	BYTE system_clock_reference_base21:2;	
	BYTE marker_bit1:1;	
	BYTE system_clock_reference_base1:3;
	BYTE fix_bit:2;				//'01'

	BYTE system_clock_reference_base22;

	BYTE system_clock_reference_base31:2;
	BYTE marker_bit2:1;
	BYTE system_clock_reference_base23:5;


	BYTE system_clock_reference_base32;

	BYTE system_clock_reference_extension1:2;
	BYTE marker_bit:1;
	BYTE system_clock_reference_base33:5;	//system_clock_reference_base 33bit


	BYTE marker_bit3:1;
	BYTE system_clock_reference_extension2:7; //system_clock_reference_extension 9bit

	BYTE program_mux_rate1;

	BYTE program_mux_rate2;

	BYTE marker_bit5:1;
	BYTE marker_bit4:1;
	BYTE program_mux_rate3:6;

	BYTE pack_stuffing_length:3;
	BYTE reserved:5;

	
	/*WORD dwRerved;
	UINT iFrameIndex;*/

	PS_HEADER_tag()
	{
		pack_start_code[0] = 0x00;
		pack_start_code[1] = 0x00;
		pack_start_code[2] = 0x01;
		pack_start_code[3] = 0xBA;
		fix_bit = 0x01;	
		marker_bit = 0x01;
		marker_bit1 = 0x01;
		marker_bit2 = 0x01;
		marker_bit3 = 0x01;
		marker_bit4 = 0x01;
		marker_bit5 = 0x01;
		reserved = 0x1F;
		pack_stuffing_length = 0x00;
		system_clock_reference_extension1 = 0;
		system_clock_reference_extension2 = 0;
	}

	void getSystem_clock_reference_base(UINT64 &_ui64SCR)
	{
		_ui64SCR = (system_clock_reference_base1 << 30) | (system_clock_reference_base21 << 28)
			| (system_clock_reference_base22 << 20) | (system_clock_reference_base23 << 15)
			| (system_clock_reference_base31 << 13) | (system_clock_reference_base32 << 5)
			| (system_clock_reference_base33);

	}

	void setSystem_clock_reference_base(UINT64 _ui64SCR)
	{
		system_clock_reference_base1 = (_ui64SCR >> 30) & 0x07;
		system_clock_reference_base21 = (_ui64SCR >> 28) & 0x03;
		system_clock_reference_base22 = (_ui64SCR >> 20) & 0xFF;
		system_clock_reference_base23 = (_ui64SCR >> 15) & 0x1F;
		system_clock_reference_base31 = (_ui64SCR >> 13) & 0x03;
		system_clock_reference_base32 = (_ui64SCR >> 5) & 0xFF;
		system_clock_reference_base33 = _ui64SCR & 0x1F;
	}

	void getProgram_mux_rate(UINT &_uiMux_rate)
	{
		_uiMux_rate = (program_mux_rate1 << 14) | (program_mux_rate2 << 6) | program_mux_rate3;
	}

	void setProgram_mux_rate(UINT _uiMux_rate)
	{
		program_mux_rate1 = (_uiMux_rate >> 14) & 0xFF;
		program_mux_rate2 = (_uiMux_rate >> 6) & 0xFF;
		program_mux_rate3 = _uiMux_rate & 0x3F;
	}
} GS_MEDIA_ATTRIBUTE_PACKED  PS_HEADER_tag;

typedef PS_HEADER_tag* pPS_HEADER_tag;

typedef struct PS_SYSTEM_HEADER_tag//12
{
	BYTE system_header_start_code[4];
	BYTE header_length[2];

	BYTE rate_bound1:7; 
	BYTE marker_bit1:1;

	BYTE rate_bound2; 

	BYTE	marker_bit2:1;
	BYTE rate_bound3:7; 

	BYTE	CSPS_flag:1;
	BYTE	fixed_flag:1;
	BYTE	audio_bound:6;

	BYTE	video_bound:5;
	BYTE	marker_bit3:1;
	BYTE	system_video_lock_flag:1;
	BYTE	system_audio_lock_flag:1;

	BYTE	reserved_bits:7;
	BYTE	packet_rate_restriction_flag:1;
////////////
	BYTE stream_id1;

	BYTE P_STD_buffer_size_bound1:5;
	BYTE P_STD_buffer_bound_scale:1;
	BYTE fix_bit:2;

	BYTE P_STD_buffer_size_bound;
/////
	BYTE stream_id2;

	BYTE aP_STD_buffer_size_bound1:5;
	BYTE aP_STD_buffer_bound_scale:1;
	BYTE afix_bit:2;

	BYTE aP_STD_buffer_size_bound;
//////////
	//BYTE reserved[30];

	PS_SYSTEM_HEADER_tag()
	{
		system_header_start_code[0] = 0x00;
		system_header_start_code[1] = 0x00;
		system_header_start_code[2] = 0x01;
		system_header_start_code[3] = 0xBB;

		header_length[1]=12;


		//program_mux_rate1 = (_uiMux_rate >> 14) & 0xFF;
		//program_mux_rate2 = (_uiMux_rate >> 6) & 0xFF;
		//program_mux_rate3 = _uiMux_rate & 0x3F;

		rate_bound1= (0x667A >> 15)&0x7F; //667A
		marker_bit1=1;

		rate_bound2=(0x667A >> 7)&0xFF; 

		marker_bit2=1;
		rate_bound3=(0x667A)&0x7F; 

		CSPS_flag=0;
		fixed_flag=0; // 1;
		audio_bound=1; //63;

		video_bound=1;
		marker_bit3=1;
		system_video_lock_flag=1;
		system_audio_lock_flag=1;

		reserved_bits=127;
		packet_rate_restriction_flag=0;
//////
		stream_id1=0xe0;
		P_STD_buffer_size_bound1=1;
		P_STD_buffer_bound_scale=1;
		fix_bit=3;
		P_STD_buffer_size_bound=254; //

		stream_id2=0xc0;
		aP_STD_buffer_size_bound1=0;
		aP_STD_buffer_bound_scale=0;
		afix_bit=3;
		aP_STD_buffer_size_bound=32;
	}
} GS_MEDIA_ATTRIBUTE_PACKED PS_SYSTEM_HEADER_tag;

typedef PS_SYSTEM_HEADER_tag* pPS_SYSTEM_HEADER_tag;

typedef struct PES_HEADER_tag//size =9
{
	BYTE	packet_start_code_prefix[3];
	BYTE	stream_id;
	BYTE	PES_packet_length[2];

	BYTE	original_or_copy:1;
	BYTE	copyright:1;
	BYTE	data_alignment_indicator:1;
	BYTE	PES_priority:1;
	BYTE	PES_scrambling_control:2;
	BYTE	fix_bit:2;

	BYTE	PES_extension_flag:1;
	BYTE	PES_CRC_flag:1;
	BYTE	additional_copy_info_flag:1;
	BYTE	DSM_trick_mode_flag:1;
	BYTE	ES_rate_flag:1;
	BYTE	ESCR_flag:1;
	BYTE	PTS_DTS_flags:2;

	BYTE	PES_header_data_length;

	PES_HEADER_tag()
	{
		packet_start_code_prefix[0] = 0x00;
		packet_start_code_prefix[1] = 0x00;
		packet_start_code_prefix[2] = 0x01;

		PES_packet_length[0] = 0x00;
		PES_packet_length[1] = 0x00;

		stream_id = 0xE0;
		fix_bit = 0x02;
	}

}GS_MEDIA_ATTRIBUTE_PACKED PES_HEADER_tag;

typedef PES_HEADER_tag *pPES_HEADER_tag;


typedef struct PTS_tag//size = 10
{
	BYTE marker_bit:1;
	BYTE PTS1:3;
	BYTE fix_bit:4;

	BYTE PTS21;

	BYTE marker_bit1:1;
	BYTE PTS22:7;

	BYTE PTS31; 

	BYTE marker_bit2:1;
	BYTE PTS32:7;

/////////////////
	BYTE marker_bit3:1;
	BYTE DTS1:3;
	BYTE fix_bit2:4;

	BYTE  DTS21;

	BYTE marker_bit4:1;
	BYTE DTS22:7;

	BYTE  DTS31; 

	BYTE marker_bit5:1;
	BYTE  DTS32:7;

	PTS_tag()
	{
		fix_bit = 0x03;
		marker_bit = 0x01;
		marker_bit1 = 0x01;
		marker_bit2 = 0x01;

		fix_bit2 = 0x01;
		marker_bit3 = 0x01;
		marker_bit4 = 0x01;
		marker_bit5 = 0x01;
	}

	void getPTS(UINT64 &_ui64PTS)
	{
		_ui64PTS = (PTS1 << 30) | (PTS21 << 22)
			| (PTS22 << 15) | (PTS31 << 7) | (PTS32);
	}

	 void getDTS(UINT64 &_ui64DTS)
	{
		_ui64DTS = (DTS1 << 30) | (DTS21 << 22)
			| (DTS22 << 15) | (DTS31 << 7) | (DTS32);
	}

	static UINT64 DTS2MSec(UINT64 iValue, int iRate )
	{
		if( iRate==0 )
		{
			GS_ASSERT(0);
			return 0;
		}
		return (UINT64)((1000.0/iRate)*iValue);
	}


	void setPTS(UINT64 _ui64PTS,UINT64 _ui64DTS)
	{
		PTS1 = (_ui64PTS >> 30) & 0x07;
		PTS21 = (_ui64PTS >> 22) & 0xFF;
		PTS22 = (_ui64PTS >> 15) & 0x7F;
		PTS31 = (_ui64PTS >> 7) & 0xFF;
		PTS32 = _ui64PTS & 0x7F;

		DTS1 =  0xFF;  //(_ui64DTS >> 30) & 0x07;
		DTS21 = 0xFF ; //(_ui64DTS >> 22) & 0xFF;
		DTS22 = 0xFF ; // (_ui64DTS >> 15) & 0x7F;
		DTS31 = 0xFF ; //(_ui64DTS >> 7) & 0xFF;
		DTS32 = 0xFF ; //_ui64DTS & 0x7F;
	}
}GS_MEDIA_ATTRIBUTE_PACKED PTS_tag;

typedef PTS_tag *pPTS_tag;

//Program Stream Map
typedef struct PSM_tag
{
	BYTE packet_start_code_prefix[3];
	BYTE map_stream_id;
	BYTE program_stream_map_length[2];  //6

	BYTE program_stream_map_version:5;
	BYTE reserved1:2;
	BYTE current_next_indicator:1;  // 7

	BYTE marker_bit:1;
	BYTE reserved2:7;  // 8

	BYTE program_stream_info_length[2]; // 10

	BYTE elementary_stream_map_length[2];  // 12

	BYTE stream_type_video;  // 13
	BYTE elementary_stream_id_video; // 14 
	BYTE elementary_stream_info_length_video[2];  // 16

	BYTE stream_type_audio;  // 17 
	BYTE elementary_stream_id_audio; // 18 
	BYTE elementary_stream_info_length_audio[2];  // 20

	BYTE CRC_32[4];  // 24

	PSM_tag()
	{
		packet_start_code_prefix[0] = 0x00;
		packet_start_code_prefix[1] = 0x00;
		packet_start_code_prefix[2] = 0x01;

		map_stream_id = 0xBC;
		program_stream_map_length[0] = 0x00;
		program_stream_map_length[1] = 0x12;

		program_stream_map_version = 0x00;
		current_next_indicator = 0x01;
		reserved1 = 0x03;
		program_stream_map_version = 0x00;

		reserved2 = 0x7F;
		marker_bit = 0x01;

		program_stream_info_length[0] = 0x00;
		program_stream_info_length[1] = 0x00;

		elementary_stream_map_length[0] = 0x00;
		elementary_stream_map_length[1] = 0x0C; 

		stream_type_video = 0x1B;
		elementary_stream_id_video = 0xE0;

		elementary_stream_info_length_video[0] = 0x00;
		elementary_stream_info_length_video[1] = 0x00;


		stream_type_audio = stream_type_audio_g711;
		elementary_stream_id_audio = 0xC0;

		elementary_stream_info_length_audio[0] = 0x00;
		elementary_stream_info_length_audio[1] = 0x00;

// 		CRC_32[3] = 0x45;
// 		CRC_32[2] = 0xBD;
// 		CRC_32[1] = 0xDC;
// 		CRC_32[0] = 0xF4;
		CRC_32[3] = 0;
		CRC_32[2] = 0;
		CRC_32[1] = 0;
		CRC_32[0] = 0;
	}
} GS_MEDIA_ATTRIBUTE_PACKED PSM_tag;

typedef PSM_tag *pPSM_tag;


#ifdef _WIN32
#pragma pack( pop )
#endif



const int g_iMaxPESSize = 0x2050;

class CES2PS
{
public  :
	int  m_iPSIndex;
	UINT m_uiFrameRate;
	int  m_stream_ID;
	int  m_stream_type;
	int  m_stream_type_audio;
	int m_stream_ID_audio;

	bool m_blFirstPES;
#ifdef __WRITE_FILE__
	FILE *m_pFile;
#endif

public:

	CES2PS(void);
	virtual ~CES2PS(void);

	void SetConvertParam(DWORD uiFrameRate,int stream_type)
	{		
		m_blFirstPES = true;
		m_uiFrameRate = uiFrameRate;		
		m_stream_type = stream_type;
	}
	void SetAudioType(int stream_type)
	{		
		m_stream_type_audio = stream_type;
	}
	
	int ESConvertToPs(const BYTE **pInpuf, int *ioLen,int iframelen, 
					BYTE *pOutHeaderBuf, int iOutBufSize );

	int ESConvertMakePES(int iframelen, BYTE *pOutHeaderBuf, int iOutBufSize, bool bAudio);

	

	

	BOOL IsTestStreamType(const BYTE *pSrc, int len, int istream_type, const BYTE **ppStart);

	BOOL IsExistStreamHeader(const BYTE *pSrc, int len);


	/*int ESconvertToPS(BYTE * buffer,int len,BYTE *pOutBuf, int iOutBufSize,int *iOutLen,DWORD uiFrameRate,int stream_ID,int stream_type);*/

	int make_ps_packet_header(BYTE *_pHeader,UINT _iFrameIndex,UINT _iFrameRate);
	int make_pes_packet_header(BYTE *_pHeader,UINT _iFrameDataLen,
								UINT _iFrameIndex,UINT _iFrameRate,int stream_id, bool bFirst);

	int make_sys_packet_header(BYTE *_pHeader,UINT _iDataLen,UINT _iFrameIndex,UINT _iFrameRate,int stream_id);

	BOOL mpeg4video_probe_is_iframe(const BYTE *buf, int buf_size);
	BOOL h264_probe_is_sps(const BYTE *buf, int buf_size);


#define PS_HEADER_PK_START  1  // 0xbA
#define PS_HEADER_SMH  2  // 0xbb  SYSTEM_HEADER
#define PS_HEADER_PSM  3  // 0xbc  PROGRAM_STREAM_MAP
#define PS_HEADER_PES  4  // 0x**
#define PS_HEADER_NONE 0  

	//检测 PS 流的包头类型, 当要检测 PES 包时, iStreamType
	static INT TestPSStream( const BYTE *p, int iSize, const BYTE **ppStart, INT &iDataSize);

	static BOOL ParserPSM( const BYTE *p, int iSize, std::vector<StruPSStreamInfo> &vStreamInfo);

	//出错返回 -1， 否则返回 , iPTS|iDTS = MAX_UINT64  表示没有该项目
	static BOOL PSStreamGetPES( const BYTE *pPS,int iSize,
						const BYTE **pDataStart, INT &iDataSize, INT &iStreamId,
						UINT64 &iPTS, UINT64 &iDTS );

	//检测码流类型
	static int TestStreamType(const BYTE *pSrc, int len );

	static int mpeg4video_probe(const BYTE *buf, int buf_size);
	static int h264_probe(const BYTE *buf, int buf_size );

private :
	
	static int mpegps_probe(const BYTE *buf, int buf_size);
	static BOOL h264_probe_is_video(const BYTE *buf, int buf_size, const BYTE **ppStart);
};

#if 0



class CPSStreamDecoder
{
public :
	typedef struct _StruFramePktInfo
	{
		const unsigned char *pPSData;
		int iPSDataSize;

		const unsigned char *pBodyData;
		int iBodyDataSize;

		int bKey;
		EnumGSMediaType eMediaType;
		EnumGSCodeID eCodeId;
	}StruFramePktInfo;

	

private :
	typedef struct _StruPSPktInfo
	{
		INT iStreamId;
		INT iStreamType;
		EnumGSCodeID eCodeId;
		EnumGSMediaType eMediaType;
	}StruPSPktInfo;

	BYTE *m_pCache;
	int m_iCacheSize;
	int m_iR;
	int m_iW;
	BOOL m_bPSStreamLosePkt;
	

	StruPSPktInfo m_stAudioStreaInfo;
	StruPSPktInfo m_stVideoStreaInfo;

	std::vector<StruPSPktInfo> m_vPSStreamInfo;

public :
	CPSStreamDecoder(void);
	~CPSStreamDecoder(void);
	bool Init(void);
	bool Decode(const unsigned char *pData, int iSize );
	EnumGSCodeID GetStreamCodeID( int iStreamId);
	// <0 错误， 1 已经结束， 0  成功
	int GetMediaFrame(StruFramePktInfo &stInfo);

private :
// #define PS_HEADER_PK_START  1  // 0xbA
// #define PS_HEADER_SMH  2  // 0xbb  SYSTEM_HEADER
// #define PS_HEADER_PSM  3  // 0xbc  PROGRAM_STREAM_MAP
// #define PS_HEADER_PES  4  // 0x**
// #define PS_HEADER_NONE 0  

	//检测 PS 流的包头类型, 当要检测 PES 包时, iStreamType
	INT TestPSStream( const BYTE *p, int iSize, const BYTE **ppStart, INT &iDataSize);

	BOOL ParserPSM( const BYTE *p, int iSize, std::vector<StruPSPktInfo> &vStreamInfo);

	//出错返回 -1， 否则返回 , iPTS|iDTS = MAX_UINT64  表示没有该项目
	BOOL PSStreamGetPES( const BYTE *pPS,int iSize,
		const BYTE **pDataStart, INT &iDataSize, INT &iStreamId,
		UINT64 &iPTS, UINT64 &iDTS );

	void SkipPES(const BYTE **pp, int iSize);


};

#endif


typedef struct _StruIDPair
{
	INT iChn;
	EnumGSCodeID eGSCodeId;
	INT iSipStreamType;
	EnumGSMediaType eMediaType;
}StruIDPair;



} //end namespace SIP

} //end namespace GSP

#endif //end _GS_H_ES2PS_H_