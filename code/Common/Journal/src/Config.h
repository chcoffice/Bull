/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : CONFIG.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/8 15:47
Description: 
********************************************
*/

#ifndef _GS_H_CONFIG_H_
#define _GS_H_CONFIG_H_
#include "JouObj.h"



namespace JOU
{

class CConfig :
	public CJouObj
{
public:
	CConfig(void);
	virtual ~CConfig(void);

	void LoadConfig( const char *czConfFilename);


	CGSString m_strIniFilename; //配置文件名

	//日志输出配置
	UINT m_iLogLevelConsole;   //日志屏幕输出项， 默认 所有打开
	UINT m_iLogLevelFile;    //记录日志文件项， 默认 LV_FATAL, LV_ERROR, LV_WARN, LV_DEBUG
	CGSString m_strLogDir; //日志目录, 默认为 ./JouLog


	


	//日志数据库配置
	CGSString m_strDBHostname; //数据库服务器IP
	CGSString m_strDBName;     //数据库名
	CGSString m_strDBUser;     //用户名
	CGSString m_strDBPWD;		 //密码
	INT    m_eDbaseType;		//数据库类型


	//日志管理配置
	
	//操作日志 删除方式
	UINT32 m_iRcdOperDays;   //保留多少天， 0 不生效, 默认 240
//	UINT32 m_iRcdOperRows;   //保留多少条， 0 不生效, 默认 5,000,000

	//运行日志 删除方式
	UINT32 m_iRcdStatusDays;   //保留多少天， 0 不生效, 默认 120
//	UINT32 m_iRcdStatusRows;   //保留多少条， 0 不生效, 默认 1,000,000

	//登陆日志 删除方式
	UINT32 m_iRcdLoginDays;   //保留多少天， 0 不生效, 默认 240
//	UINT32 m_iRcdLoginRows;   //保留多少条， 0 不生效, 默认 5,000,000

	//本地缓存数据存储目录
	CGSString m_strCachePath;  //默认为 , 默认为 ./JouData
	//是否使用缓存功能
	INT m_bEnableCache;

	INT m_iCacheInterval; //缓冲的时间间隔, 默认 20 秒


};

} //end namespace GSS

#endif //end _GS_H_CONFIG_H_
