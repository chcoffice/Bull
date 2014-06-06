/************************************************************
  Copyright (C), 2010-2011, GOSUN 
  FileName:     ILogLibrary.h
  Author:       sdj   
  Version:      1.0.0.0    
  Date:         2010-5-25 9:40:02
  Description:  提供了一个日志接口，可以设置同时设置多种输出方式，多种日志级别，
                其特点是效率较高，线程安全
***********************************************************/
#if !defined (ILOGLIBRARY_DEF_H)
#define ILOGLIBRARY_DEF_H
#include <stdio.h>
#include <string>
#include <iostream>

#include "GSCommon.h"
#include "GSType.h"
#include "GSDefine.h"

using namespace std;

/*日志级别:级别由高到低，LEMERG最高，LNOTSET最低*/
#define LEMERG  0x00000001
#define LFATAL  0x00000002
#define LALERT  0x00000004
#define LCRIT   0x00000008
#define LERROR  0x00000010
#define LWARN   0x00000020
#define LNOTICE 0x00000040
#define LINFO   0x00000080
#define LDEBUG  0x00000100
#define LNOTSET 0x00000200

/*日志输出方式，控制台或文件*/
#define MCONSOLE   0x00000001//控制台
#define MFILE      0x00000002//文件

/*文件创建方式*/
#define WREWIND  0x00000001//回卷
#define WCREATE  0x00000002//新建


/*错误码定义*/
#define ERROR_LOGLIBRARY_OPER_SUCCESS ERROR_BASE_SUCCESS 
#define ERROR_LOGLIBRARY_START (ERROR_BASE_START+4000)
#define ERROR_LOGLIBRARY_DIR_NOT_EXIST		(ERROR_LOGLIBRARY_START + 1) //路径错误
#define ERROR_LOGLIBRARY_FILE_NOT_OPEN		(ERROR_LOGLIBRARY_START + 2) //文件没有打开
#define ERROR_LOGLIBRARY_FILESIZE_ILLEGAL	(ERROR_LOGLIBRARY_START + 3) //大小非法
#define ERROR_LOGLIBRARY_FILEWAY_ILLEGAL	(ERROR_LOGLIBRARY_START + 4) //文件创建策略非法
#define ERROR_LOGLIBRARY_LOGLEVEL_ILLEGAL	(ERROR_LOGLIBRARY_START + 5) //日志等级非法
#define ERROR_LOGLIBRARY_LOGMEDIUM_ILLEGAL	(ERROR_LOGLIBRARY_START + 6) //日志输出介质非法
#define ERROR_LOGLIBRARY_MSGSIZE_ILLEGAL	(ERROR_LOGLIBRARY_START + 7) //日志信息大小非法
#define ERROR_LOGLIBRARY_STRING_ILLEGAL		(ERROR_LOGLIBRARY_START + 8) //字符串大小非法
#define ERROR_LOGLIBRARY_MKDIR_FAIl			(ERROR_LOGLIBRARY_START + 9) //创建文件夹失败
#define ERROR_LOGLIBRARY_DISK_ERROR         (ERROR_LOGLIBRARY_START + 10)//磁盘错误
class ILogLibrary
{

public:
	ILogLibrary(){};
	virtual ~ILogLibrary(void){};
public:
	/*************************************************
	  Function:   SetLogDir
	  DateTime:   2010-5-21 11:39:37   
	  Description: 设置日志路径
	  Input:    pszDir	日志文件的存放路径字符串 
				pszExtName	日志文件名后缀
	             
	  Output:   无 
	  Return:   ERROR_LOGLIBRARY_OPER_SUCCESS	操作成功	
				ERROR_LOGLIBRARY_DIR_NOT_EXIST	路径不存在
				ERROR_LOGLIBRARY_FILE_NOT_OPEN	日志文件没有打开
	  Note:     路径建议采用绝对路径,并且路径必须合法,否则造成创建不成功  
	*************************************************/
	virtual INT  SetLogDir(const char *pszDir,const char *pszExtName) = 0;

	/*************************************************
	  Function:   SetLogSize
	  DateTime:   2010-5-21 11:44:58   
	  Description:设置日志大小及是否回卷 
	  Input:   uiWay     日志文件是否回卷，值为WREWIND，回卷；值为WCREATE，新建
	           uiSize	 日志文件大小，为0时，日志文件为默认大小2MB；为正整数时，是日志文件的最大字节数（单位为MB）；其他，错误
	  Output:  无
	  Return:  ERROR_LOGLIBRARY_OPER_SUCCESS		操作成功
			   ERROR_LOGLIBRARY_FILESIZE_ILLEGAL	日志文件大小非法
			   ERROR_LOGLIBRARY_FILEWAY_ILLEGAL		日志文件方式非法

	  Note:     
	*************************************************/
	virtual INT  SetLogSize(UINT uiWay,UINT uiSize) = 0;

	/*************************************************
	  Function:   SetLogLevel
	  DateTime:   2010-5-21 11:45:42   
	  Description: 设置日志输出方式（控制台或文件）及对应日志级别
	  Input:   uiMedium	日志文件输出方式，值为MCONSOLE，输出到控制台；值为MFILE，输出到文件；
			   uiLevel	日志文件的输出等级，其值及级别关系 LEMERG>LFATAL>LALERT>LCRIT>LERROR>LWARN
			            >LNOTICE>LINFO>LDEBUG>LNOTSET
	             
	  Output:  无
	  Return:  ERROR_LOGLIBRARY_OPER_SUCCESS		操作成功
			   ERROR_LOGLIBRARY_LOGLEVEL_ILLEGAL	日志等级非法	
			   ERROR_LOGLIBRARY_LOGMEDIUM_ILLEGAL	日志输出媒介非法
	  Note:    可以设置一种输出方式的多个级别    
	*************************************************/
	virtual INT SetLogLevel(UINT uiMedium,UINT uiLevel) = 0;
    
	/*************************************************
	  Function:   SetLogCB
	  DateTime:   2010-5-21 11:45:58   
	  Description: 设置日志回调函数
	  Input:    pCaller		用户参数
	             
	  Output:   pCaller		由用户传入的
				pszLevel    等级
				pszPrefix	前缀
				pszMsg		日志信息
	  Return:   无
	  Note:      
	*************************************************/
	virtual void SetLogCB(void *pCaller,void (  *pfnDealCB)(void *pCaller,const char *pszLevel,const char *pszPrefix,const char *pszMsg)) = 0;

	/*************************************************
	  Function:   Log
	  DateTime:   2010-5-21 11:46:07   
	  Description: 写日志
	  Input:    uiLevel		日志文件的输出等级，其值及级别关系 LEMERG>LFATAL>LALERT>LCRIT>LERROR>LWARN
							>LNOTICE>LINFO>LDEBUG>LNOTSET
	            pszPrefix	日志前缀，为NULL时，为默认前缀no，其长度不应超过64字节
				pszFormat	日志格式串，与printf中格式串用法相同
				...			格式串中对应值
	  Output:   无
	  Return:   ERROR_LOGLIBRARY_OPER_SUCCESS		操作成功
				ERROR_LOGLIBRARY_LOGLEVEL_ILLEGAL	日志等级非法
	  Note:      
	*************************************************/
	virtual INT  Log(UINT uiLevel,char *pszPrefix,char  *pszFormat,...) = 0;
	
};
extern ILogLibrary *GetLogInstance();
extern void ClearLogInstance(ILogLibrary *pLogLibrary);
#endif
