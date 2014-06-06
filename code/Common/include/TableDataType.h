#ifndef  TABLEDATATYPE_H_
#define  TABLEDATATYPE_H_
/********************************************************************
    Copyright (C), 2010-2011, GOSUN 
    File name 	: TableDataType.h      
    Author 		: sdj      
    Version 	: V1.00        
    DateTime 	: 2010/21/7  16:11
    Description : 此头文件主要是根据数据库的表结构定义相应数据结构，用于PMS数据库的访问及数据结构的定义， 除此之外还
				  用于PMS中Excel配置转换工具使用.定义包含设备信息，服务信息，以及设备服务关联信息。

				  ----设备的信息主要包括设备基本信息，设备通道信息，设备告警信息，设备型号信息等几个方面。

				  ----服务的信息

				  ----设备服务关联信息
				  
				  PMS内部各模块以及各服务所需要信息是有差异的，所以定义形式各不相同，并且随着
*********************************************************************/
#include "GSType.h"

#define TB_MAX_NAME_LEN 128
#define TB_MAX_PWD_LEN 128
#define TB_MAX_CONN_LEN 128
#define TB_MAX_REMARKS_LEN 256
#define TB_MAX_VERSION_LEN 32

/************************************************************************/
/*                         基本结构                                     */
/************************************************************************/

//基本结构

//单元（设备，客户端，以及服务）基本信息
typedef struct StruBaseInfo
{
	INT32 iID;//单元ID
	INT32 iModelID;//单元型号ID
	char szVersion[TB_MAX_VERSION_LEN];//单元版本
	
}StruBaseInfo;

typedef struct StruDevChn
{
	INT32 iDevID;//设备ID
	INT32 iChnID;//通道ID
}StruDevChn;
//单元描述
typedef struct StruDescri
{	
	char szName[TB_MAX_NAME_LEN];//设备名称
	char szRemarks[TB_MAX_REMARKS_LEN];//备注
}StruDescri;

//设备登陆信息
typedef struct StruDevLoginInfo 
{
	char szDevConn[TB_MAX_CONN_LEN];//设备连接方式
	char szLoginName[TB_MAX_NAME_LEN];//登录名
	char szLoginPWD[TB_MAX_PWD_LEN];// 登陆密码
}StruDevLoginInfo;

//扩展参数
typedef struct StruMemBlock 
{
	INT32 iLen;
	void *pBlock;
}StruMemBlock;

//设备的扩展信息，如配置等
typedef struct StruDevExInfo 
{
	StruMemBlock stParam1;//扩展参数1
	StruMemBlock stParam2;//扩展参数2
	StruMemBlock stParam3;//扩展参数3

}StruDevExInfo;

//单元域信息
typedef struct StruDomain
{
	INT32 iDomain;//所属域
	
}StruDomain;

//类型信息
typedef struct StruTypeInfo 
{
	INT32 iTypeID;
	char szTypeName[TB_MAX_NAME_LEN];
}StruTypeInfo;

typedef struct StruTBTypeInfo
{
	INT32 iKey;
	StruTypeInfo stTypeInfo;
}StruTBTypeInfo;
/************************************************************************/
/*            用于Excel配置转换工具                                     */
/************************************************************************/

/////////////////设备基本信息/////////////////////////////////////////////////////////////


//设备基本信息
typedef struct StruBaseDevInfo 
{
	StruBaseInfo stBaseInfo;//基本信息
	StruDomain stDomain;//域信息
	StruDevLoginInfo stDevLoginInfo;//登陆信息
	StruDescri stDescri;//描述	
}StruBaseDevInfo;

typedef struct StruDevInfo 
{
	INT32 iKey;//自增长ID
    StruBaseDevInfo stBaseDevInfo;
	StruDevExInfo stDevExInfo;//扩展参数
}StruDevInfo;

//设备型号
typedef struct StruBaseDevModel
{
	StruTypeInfo stModel;//设备型号信息
	INT32 iVendorID;//设备厂商ID
	INT32 iTypeID;//设备类型ID

}StruBaseDevModel;

typedef struct StruDevModel
{
	INT32 iKey;//自增长ID
	StruBaseDevModel stBaseDevModel;
}StruDevModel;

//设备类型
typedef StruTBTypeInfo StruDevType;

//设备厂家
typedef StruTBTypeInfo StruDevVendor;




///////////////////设备告警信息////////////////////////////////////////////////////////////////////////////////
//设备告警类型
typedef StruTBTypeInfo StruAlarmType ;

//设备告警等级
typedef StruTBTypeInfo StruAlarmLevel;


//设备告警信息
typedef struct StruBaseAlarmInfo 
{
	StruDevChn stDevChn;//设备对应通道,通道ID，暂定-1时表示与设备通道无关的告警，比如设备故障告警
	INT32 iAlarmTypeID;//告警类型ID
	INT32 iAlarmLevelID;//告警等级ID
	char szAlarmRemark[256];//备注
}StruBaseAlarmInfo;

typedef struct StruAlarmInfo 
{
	INT32 iKey;//自增长ID
    StruBaseAlarmInfo stBaseAlarmInfo;
}StruAlarmInfo;


///////////////////设备通道信息////////////////////////////////////////////////////////////////////////////////

//设备通道信息
typedef struct StruBaseChnInfo 
{
	StruDevChn stDevChn;//设备对应通道
	INT32 iChnTypeID;//设备通道类型ID
	StruDescri stChn;//描述
}StruBaseChnInfo;

typedef struct StruChnInfo 
{
	INT32 iKey;//自增长ID
	StruBaseChnInfo stBaseChnInfo;
}StruChnInfo;

//设备通道类型
typedef StruTBTypeInfo StruChnType;


///////////服务信息///////////////////////////////////////////////////////////////////////////////////////

//服务信息
typedef struct StruBaseServInfo
{
	StruBaseInfo stBaseInfo;//基本信息
	StruDomain stDomain;//域信息
	StruDescri stDescri;//描述
	StruMemBlock stAttri;//属性
}StruBaseServInfo;

typedef struct StruServInfo
{
	INT32 iKey;//自增长ID
	StruBaseServInfo stBaseServInfo;
}StruServInfo;
//服务类型
typedef StruTBTypeInfo StruServType;



//////////////////服务设备关联////////////////////////////////

//服务管理信息
typedef struct StruBaseSerDev
{
	INT32 iSerID;//服务ID
	INT32 iDevID;//设备ID
}StruBaseSerDev;

typedef struct StruSerDev
{
	INT32 iKey;//自增长ID
	StruBaseSerDev stBaseSerDev;
}StruSerDev;


/************************************************************************/
/*                                 用于CLI、PMS                          */
/************************************************************************/

//////////////////////////设备信息/////////////////

//基本信息
typedef StruBaseDevInfo StruBaseDevItem;


//设备型号信息
typedef StruBaseDevModel StruDevModelItem;

//设备类型
typedef StruTypeInfo StruDevTypeItem;

//设备厂家
typedef StruTypeInfo StruDevVendorItem;



///////////////////设备告警信息////////////////////////////////////////////////////////////////////////////////
//设备告警类型
typedef StruTypeInfo StruAlarmTypeItem ;

//设备告警等级
typedef StruTypeInfo StruAlarmLevelItem;



//设备告警信息
typedef StruBaseAlarmInfo StruAlarmInfoItem;


///////////////////设备通道信息////////////////////////////////////////////////////////////////////////////////


//设备通道类型
typedef StruTypeInfo StruChnTypeItem;

//设备通道信息
typedef StruBaseChnInfo StruChnInfoItem;

///////////服务信息///////////////////////////////////////////////////////////////////////////////////////

//服务信息

typedef StruBaseServInfo StruBaseServItem;//服务信息

//服务类型
typedef StruTypeInfo StruServTypeItem;



//////////////////服务设备关联////////////////////////////////

//服务管理信息

typedef StruBaseSerDev StruSerDevItem;



/************************************************************************/
/*				   用于DAS、PMS                                         */
/************************************************************************/
typedef struct StruDASDevItem
{
	StruBaseInfo stBaseInfo;//基本信息
	StruDevLoginInfo stDevLoginInfo;//登陆信息
}StruDASDevItem;



/************************************************************************/
/*         用于PMS设备管理模块（PMS内部使用）                                          */
/************************************************************************/

////////////////////////服务信息//////////////////////////////////
typedef struct StruPMSDevItem
{
	StruBaseDevItem stBaseDevItem;//基本信息
	INT32 iOnlineSta;//在线状态,0--不在线,1--在线
	
}StruPMSDevItem;
//服务信息
typedef struct StruPMSServItem
{
	StruBaseServItem stBaseServItem;
	void * pNetChn;//服务通道指针,该通道不存在时值为NULL,存在时值为非NULL
	INT32 iOnlineSta;//在线状态,0--不在线,1--在线
}StruPMSServItem;

typedef		struct		StruDomainInfoItem 
{
	INT32					iDomainID;							//	域ID
	char					szDomainName[TB_MAX_NAME_LEN];		//	域名称
	INT32					iParentDomainID;					//	父域ID
	INT32					iPlatfromID;						//  本级平台ID,初始值为0
	INT32					iParentPlatfromID;					//	上级平台ID,没有上级平台为0
}StruDomainInfoItem;

//MapSerDev MapDASDev,MapSTSDev,MapCSSDev,MapCli; //为每种服务都定义一个独立的map，设备ID作为key

#endif
