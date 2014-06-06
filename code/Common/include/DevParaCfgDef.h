/*******************************************************************************
  Copyright (C), 2010-2011, GOSUN 
  File name   : DEVPARACFGDEF.H      
  Author      : jiangsx      
  Version     : Vx.xx        
  DateTime    : 2011/10/18 11:42
  Description : 设备参数配置定义（具体说明参考文档《设备参数配置协议说明.docx》）
*******************************************************************************/
#ifndef DEVPARACFGDEF_DEF_H
#define DEVPARACFGDEF_DEF_H

#include "GSCommon.h"

// 协议命名空间
namespace	CmdProtocolDef
{
	// 操作结果定义
#define GS_CFG_OPT_SUCCESS "success"
#define GS_CFG_OPT_FAIL "fail"
#define GS_CFG_OPT_UNKNOWN_ERROR "unknownerror"


	// 码流类型枚举
	typedef enum
	{
		VIDEO_STREAM = 0,         // 视频流
		COMPLEX_STREAM            // 复合流
	} EnumGSStreamType;

	// 分辨率类型
#define GS_DCIF		"DCIF"
#define GS_CIF		"CIF"
#define GS_QCIF		"QCIF"
#define GS_4CIF     "4CIF"
#define GS_2CIF		"2CIF"
#define GS_D1		"D1"
#define GS_QVGA		 "QVGA"
#define GS_VGA		 "VGA"
#define GS_720P		"720P"
#define GS_1080P	"1080P"
#define GS_USRDEF	"USRDEF"

	// 码率类型
#define GS_BITRATE_CBR      "CBR"    // 定码率
#define GS_BITRATE_VBR	    "VBR"    // 变码率
#define GS_BITRATE_MBR		"MBR"    // 混合码率

	// 图像质量枚举
	typedef enum
	{
		PQ_BEST = 0,   // 最好
		PQ_SUB,        // 次好
		PQ_BETTER,     // 较好
		PQ_GENERIC,    // 一般
		PQ_WORSE,            // 较差
		PQ_WORST            // 差
	} EnumGSPicQuality;

	// 视频编码类型
#define GS_VIDEO_ENC_H264  "h264"
#define GS_VIDEO_ENC_MPEG4  "mpeg4"

	//音频编码类型 0-OggVorbis;1-G711_U;2-G711_A;6-G726
	// 音频编码类型
#define GS_AUDIO_ENC_OGGVORBIS   "OggVorbis"
#define GS_AUDIO_ENC_G711_U       "G711_U"
#define GS_AUDIO_ENC_G711_A       "G711_A"
#define GS_AUDIO_ENC_G726         "G726"

	// 视频制式
#define GS_VIDEO_FORMAT_PAL  "PAL"
#define GS_VIDEO_FORMAT_NTSC  "NTSC"



	// 以下定义参数配置通用关键字
#define GS_CFG_PARAM  "cfgparam"
#define GS_CMD_ID   "cmd"



#define GS_GET_CFG   "getcfg"
#define GS_RES_GET_CFG "responsegetcfg"
#define GS_SET_CFG   "setcfg"
#define GS_RES_SET_CFG "responsesetcfg"

#define GS_DEV_PARAM  "devparam"
#define GS_DEV_ID   "devid"

#define GS_DEV_TYPE "devtype"

#define GS_OPT_CONTEXT  "context"        // 参数获取时用到
#define GS_RESULT   "result"			 // 操作结果:  成功：success  失败： fail...

#define GS_CHANNEL_PARAM "chanparam"
#define GS_CHANNEL_TYPE "chantype"
#define GS_CHANNEL_ID  "chanid"


#define GS_ALL_CFG    "allcfg"


// 设备参数
#define GS_DEV_CFG   "devcfg"


// 网络参数
#define GS_NET_WORK  "network"                 
#define GS_DEV_IP     "devip"                   // 设备IP
#define GS_DEV_PORT    "devport"				// 设备端口
#define GS_DEV_USR_NAME  "devusrname"			// 登陆用户名
#define GS_DEV_PASSWORD  "devpassword"			// 登陆密码
#define GS_DEV_MAC   "devmac"					// 设备MAC

// 压缩参数
#define GS_COMPRESS   "compress"
#define GS_STREAM_TYPE          "streamtype"           // 码流类型
#define GS_RESOLUTION           "resolution"           // 分辨率
#define GS_BITRATE_TYPE         "bitratetype"          // 码率类型
#define GS_PIC_QUALITY          "picquality"		   // 图象质量
#define GS_VIDEO_BITRATE        "videobitrate"         // 码率
#define GS_VIDEO_FRAME_RATE     "videoframerate"       // 帧率
#define GS_INTERVAL_FRAME_I     "intervalframei"       // I帧间隔 
#define GS_VIDEO_ENCODE_TYPE	"videoenctype"         // 视频编码类型 
#define GS_AUDIO_ENCODE_TYPE    "audioenctype"         // 音频编码类型
#define GS_STREAM_TYPE_EX2      "streamtypeex2"        // 码流类型
#define GS_RESOLUTION_EX2       "resolutionex2"        // 分辨率
#define GS_BITRATE_TYPE_EX2     "bitratetypeex2"       // 码率类型
#define GS_PIC_QUALITY_EX2      "picqualityex2"		   // 图象质量
#define GS_VIDEO_BITRATE_EX2    "videobitrateex2"      // 码率
#define GS_VIDEO_FRAME_RATE_EX2  "videoframerateex2"   // 帧率
#define GS_INTERVAL_FRAME_I_EX2   "intervalframeiex2"  // I帧间隔 
#define GS_VIDEO_ENCODE_TYPE_EX2	"videoenctypeex2"  // 视频编码类型 
#define GS_AUDIO_ENCODE_TYPE_EX2   "audioenctypeex2"   // 音频编码类型
//#define GS_STREAM_TYPE_EX3  "streamtypeex3"          // 码流类型
//#define GS_RESOLUTION_EX3  "resolutionex3"           // 分辨率
//#define GS_BITRATE_TYPE_EX3  "bitratetypeex3"        // 码率类型
//#define GS_PIC_QUALITY_EX3   "picqualityex3"		   // 图象质量
//#define GS_VIDEO_BITRATE_EX3  "videobitrateex3"      // 码率
//#define GS_VIDEO_FRAME_RATE_EX3  "videoframerateex3" // 帧率
//#define GS_INTERVAL_FRAME_I_EX3   "intervalframeiex3"// I帧间隔 
//#define GS_VIDEO_ENCODE_TYPE_EX3	"videoenctypeex3"  // 视频编码类型 
//#define GS_AUDIO_ENCODE_TYPE_EX3   "audioenctypeex3" // 音频编码类型


// 图像参数
#define GS_PICTURE   "picture"
#define GS_CHANNEL_NAME   "channame"							  // 通道名称
#define GS_VIDEO_FORMAT   "videoformat"							  // 视频制式：1-NTSC；2-PAL 
#define GS_BRIGHTNESS    "brightness"							  // 亮度，取值范围[0,255]	
#define GS_CONTRAST    "contrast"								  // 对比度，取值范围[0,255] 
#define GS_SATURATION  "saturation"								  // 饱和度，取值范围[0,255] 
#define GS_HUE   "hue"											  // 色调，取值范围[0,255] 


// OSD参数
#define GS_OSD   "osd" 
#define GS_SHOW_CHANNELE_NAME  "showchanname"                     // 预览的图象上是否显示通道名称:0-不显示，1-显示（区域大小704*576）
#define GS_SHOW_NAME_TOP_LEFT_X  "shownametopleftx"               // 通道名称显示位置的x坐标 
#define GS_SHOW_NAME_TOP_LEFT_Y  "shownametoplefty"              // 通道名称显示位置的y坐标
#define GS_SHOW_OSD    "showosd"                                  // 预览的图象上是否显示OSD
#define GS_OSD_TOP_LEFT_X    "osdtopleftx"                        // OSD的x坐标 
#define GS_OSD_TOP_LEFT_Y    "osdtoplefty"                        // OSD的y坐标
#define GS_OSD_TYPE   "osdtype"                                   // OSD类型
#define GS_DISP_WEEK  "dispweek"                                  // 是否显示星期：0-不显示，1-显示 
#define GS_OSD_ATTRIB  "osdattrib"                                // OSD属性
#define GS_OSD_STRING_TOP_LEFT_X   "stringtopleftx"               // 字符内容的x坐标
#define GS_OSD_STRING_TOP_LEFT_Y   "stringtoplefty"               // 字符内容的y坐标
#define GS_OSD_STRING   "osdstring"                               // 字符内容


// 视频屏蔽参数
#define GS_VIDEO_MASK    "videomask"                    
#define GS_MASK_ENABLE    "enable"           // 使能开关
#define GS_AREA           "area"             // 区域
#define GS_INDEX          "index"            // 索引(从1开始)
#define GS_MASK_X         "x"                // x坐标
#define GS_MASK_Y         "y"                // y坐标
#define GS_MASK_HEIGHT    "h"                // 高度
#define GS_MASK_WIDTH     "w"                // 宽度


// 设备软硬件参数
#define GS_SOFTHARDWARE "softhardware"
#define GS_SOFTHARDWARE_DEVNAME              "devname"                     //设备名称
#define GS_SOFTHARDWARE_DEVNUM               "devnum"					   //设备编号
#define GS_SOFTHARDWARE_DEVTYPE              GS_DEV_TYPE                   //设备型号
#define GS_SOFTHARDWARE_SERIALNO             "devserialno"                 //设备序列号
#define GS_SOFTHARDWARE_SOFTWAREVERSION      "SoftWareVersion"             //软件版本号 
#define GS_SOFTHARDWARE_HARDWAREVERSION      "hardsoftversion"             //硬件版本号
#define GS_SOFTHARDWARE_VIDEONUM             "videonum"                    //视频通道个数
#define GS_SOFTHARDWARE_AUDIONUM             "audionum"                    //语音通道个数
#define GS_SOFTHARDWARE_ALARMINNUM           "alarminnum"                  //报警输入个数
#define GS_SOFTHARDWARE_ALARMOUTNUM          "alarmoutnum"                 //报警输出个数
#define GS_SOFTHARDWARE_NETIONUM              "netionum"                   //网络口数
#define GS_SOFTHARDWARE_COMIONUM             "comionum"                    //串口数量


// 北斗信息参数
#define GS_GPS_INFO						"gpsinfo"				
#define GS_GPS_VALID					"gpsvalid"
#define GS_GPS_LONGITUDE_TYPE			"longitudetype"
#define GS_GPS_LONGITUDE				"longitude"
#define GS_GPS_LATITUDE_TYPE			"latitudetype"
#define GS_GPS_LATITUDE					"latitude"
#define GS_GPS_ALTITUDE					"altitude"
#define GS_GPS_SPEED					"gpspeed"
#define GS_GPS_TIME						"gpstime"

//   以下为SIP28181设备相关参数项(协议参考28181规定)
/*********** begin *******/

#define GS_SIP_GBDEVICEID    "DeviceID"
#define GS_SIP_SUMNUM        "SumNum"
#define GS_SIP_ITEM           "Item"
#define GS_SIP_DEVICELIST     "DeviceList"
#define GS_SIP_NUM            "Num"


// 设备目录查询参数
#define GS_SIP_CATALOG        "Catalog"
#define GS_SIP_NAME                       "Name"
#define GS_SIP_MANUFACTURER               "Manufacturer"
#define GS_SIP_MODEL                      "Model"
#define	GS_SIP_OWNER                      "Owner"
#define GS_SIP_CIVILCODE                  "CivilCode"
#define GS_SIP_ADDRESS                    "Address"
#define GS_SIP_PARENTAL                   "Parental"
#define GS_SIP_SAFETYWAY                  "SafetyWay"
#define GS_SIP_REGISTERWAY                "RegisterWay"
#define GS_SIP_SECRECY                    "Secrecy"
#define GS_SIP_STATUS                     "Status"

// 设备信息查询参数
#define GS_SIP_DEVICEINFO     "DeviceInfo"
#define GS_SIP_DEVICETYPE                 "DeviceType"
// #define GS_SIP_MANUFACTURER               "Manufacturer"
// #define GS_SIP_MODEL                      "Model"
#define GS_SIP_FIRMWARE                   "Firmware"
#define GS_SIP_MAXOUT                     "MaxOut"

// 设备状态查询参数
#define GS_SIP_DEVICESTATUS    "DeviceStatus"
#define GS_SIP_ONLINE                     "Online"
#define GS_SIP_STATUS                     "Status"
#define GS_SIP_RECORD                     "Record"
#define GS_SIP_DEVICETIME                 "DeviceTime"
#define GS_SIP_ALARMSTATUS                "Alarmstatus"
#define GS_SIP_DUTYSTATUS                 "DutyStatus"
/*********** end ******* /

	// to add:


/*************************************************
// ex 1: get
<cfgparam>
	<cmd>getcfg</cmd>
	<devparam devid=1234>
		<context>compress</context> 
	</devparam>
	<devparam devid=1235>
		<chanparam chanid=1 chantype=101></chanparam>
		<chanparam chanid=2 chantype=101></chanparam>
		<context>picture</context>
		<context>compress</context> 
		<context>softhardware</context>
	</devparam>
</cfgparam>

// ex2: get response
<cfgparam>
	<cmd>responsegetcfg</cmd>
	<devparam devid=1234>
		<compress>
			<result>fail</result> 
		</compress>
	</devparam>
	<devparam devid=1235>
		<picture>
			<chanparam chanid=1 chantype=101>
				<result>success</result> 
				<channame>高新兴软件园正门</channame>
				<videoformat>PAL</videoformat>
			</chanparam>
			<chanparam chanid=2 chantype=101>
				<result>success</result> 
				<channame>高新兴软件园正门左侧</channame>
				<videoformat>1</videoformat>
			</chanparam>
		</picture>
		<compress>
			<result>success</result> 
			<streamtype>1</streamtype>
			<resolution>5</resolution>
			<bitratetype>1</bitratetype>
			<picquality>0</picquality>
			<videobitrate>1024</videobitrate>
			<videoframerate>25</videoframerate>
		</compress>
		<softhardware>
			 <result>success</result> 
		     <devname>Embedded Net DVR</devname>
             <devnum>255</devnum>
			 <devtype>""</devtype>
			 <devserialno>DS-8116HF-ST1620110715BBRR404140705WCVU</devserialno>
			 <SoftWareVersion>V2.0.1 build 110704</SoftWareVersion>
			 <hardsoftversion>0xc300</hardsoftversion>
			 <videonum>16</videonum>
			 <audionum>4</audionum>
			 <alarminnum>16</alarminnum>
			 <alarmoutnum>4</alarmoutnum>
			 <netionum>1</netionum>
			 <comionum>1</comionum>
		</softhardware>
	</devparam>
</cfgparam>

// ex 3:  set
<cfgparam>
	<cmd>setcfg</cmd>
	<devparam devid=1234>
		<network>
			<devip>192.168.15.88</devip>
			<devport>8000</devport>
			<devusrname>admin</devusrname>
			<devpassword>12345</devpassword>
		</network>
		<compress>
			<streamtype>1</streamtype>
			<resolution>5</resolution>
			<bitratetype>1</bitratetype>
			<picquality>0</picquality>
			<videobitrate>1024</videobitrate>
			<videoframerate>25</videoframerate>
		</compress>
		<picture>
			<chanparam chanid=1 chantype=101>
				<channame>高新兴软件园正门</channame>
				<videoformat>1</videoformat>
			</chanparam>
			<chanparam chanid=2 chantype=101>
				<channame>高新兴软件园正门左侧</channame>
				<videoformat>1</videoformat>
			</chanparam>
		</picture>
	</devparam>
	<devparam devid=1235>
		<picture>
			<chanparam chanid=1 chantype=101>
				<channame>高新兴软件园后门</channame>
				<videoformat>1</videoformat>
			</chanparam>
		</picture>
	</devparam>
</cfgparam>

// ex4: set response

<cfgparam>
	<cmd>responsesetcfg</cmd>
	<devparam devid=1234>
		<network>
			<result>success</result> 
		</network>
		<compress>
			<result>success</result> 
		</compress>
		<picture>
			<chanparam chanid=1 chantype=101>
				<result>fail</result> 
			</chanparam>
			<chanparam chanid=2 chantype=101>
				<result>success</result> 
			</chanparam>
		</picture>
	</devparam>
	<devparam devid=1235>
		<picture>
			<chanparam chanid=1 chantype=101>
				<result>success</result> 
			</chanparam>
		</picture>
	</devparam>
</cfgparam>

**************************************************/

//
// 参数配置XML封装类
//
class CXMLCfg
{
public:
	CXMLCfg(void){};
	~CXMLCfg(void){};

	XMLNode& GetXMLNode(void) { return m_csXMLNode; };

private:
	XMLNode m_csXMLNode;
};

// 
// XML打包类
// 
typedef CXMLCfg CXMLMaker;

//
//  XML解析类
//
typedef CXMLCfg CXMLParser;

}

#endif // DEVPARACFGDEF_DEF_H
