/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : MYSOCKET.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/13 16:40
Description: 错误号定义
********************************************
*/

#ifndef _GS_H_MYSOCKET_H_
#define _GS_H_MYSOCKET_H_

namespace GSP
{

	typedef enum
	{
		eERRNO_SUCCESS = 0,
		eERRNO_EUNKNOWN,  //未知错误
		eERRNO_EFATAL, //不可修复的错误
		eERRNO_ENONE, 

		//===================================================

		eERRNO_SYS_EINVALID = 0x0100,  //无效参数
		eERRNO_SYS_EFLOWOUT,         //缓冲溢出       
		eERRNO_SYS_ENMEM,            //没有内存
		eERRNO_SYS_EOPER,           //不支持的操作
		eERRNO_SYS_ESTATUS,         //错误状态下的操作 
		eERRNO_SYS_ENCTRL,          //不支持的控制操作
		eERRNO_SYS_ETIMEOUT,        //超时
		eERRNO_SYS_EKPL,            // keepalive 超时
		eERRNO_SYS_EBUSY,           //资源缺乏
		eERRNO_SYS_ETKP,            //任务操作失败
		eERRNO_SYS_EFUNC,           //该功能没有实现
		eERRNO_SYS_EPRO,            //不支持的协议
		eERRNO_SYS_EPERMIT,			//没有权限
		eERRNO_SYS_ENEXIST,			//不存在的对象
		eERRNO_SYS_EEXIST,			//已经存在对象
		eERRNO_SYS_ELOSE,			//丢失数据  -- TODO
		eERRNO_SYS_ECRC,			//效验错误  -- TODO
		eERRNO_SYS_ECODEID,			//错误编码 -- TODO

		//===================================================

		eERRNO_NET_EUNK = 0x0200,            //未知的网络操作错误
		eERRNO_NET_EDISCNN,                //对端网络关闭
		eERRNO_NET_ECLOSED,                  //Socket 关闭
		eERRNO_NET_EREG,                    //事件注册失败
		eERRNO_NET_EWEVT,                    //请求写事件失败
		eERRNO_NET_EREVT,                    //请求读事件失败
		eERRNO_NET_EEVT,                    //请求读事件失败
		eERRNO_NET_EBIN,                    //绑定端口失败
		eERRNO_NET_EOPT,                    //SOCKET 属性操作失败
		eERRNO_NET_ESEND,                  //写错误
		eERRNO_NET_ERCV,                   //读错误
		eERRNO_NET_ESOCKET,                 //建立SOCKET 失败

		eERRNO_NET_END,

		//===================================================

		eERRNO_SRC_ENXIST = 0x0400,       //不存在的数据源
		eERRNO_SRC_EUNUSED,             //无人使用
		eERRNO_SRC_ECLOSE,				//数据源被关闭
		eERRNO_SRC_EASSERT,				//数据源异常
		eERRNO_SRC_ECODER,				//不支持的编码方式

		//===================================================

		eERRNO_CLI_ECLOSE = 0x0600,       //对端请求关闭
		eERRNO_CLI_EASSERT,             //对端异常关闭 
		eERRNO_CLI_ENSRC,               //不存在数据源

		//================================================
		eERRNO_SRV_REFUSE = 0x0800,  //上层拒绝连接

		//===================================================
		eERRNO_EVT_CONTINUE_NEXT = 0x0a00, //还有下一个命令
	}EnumErrno;

#define IS_NETWORK_ERRNO( xErrno) ((INT)(xErrno)>=(INT)eERRNO_NET_EUNK && (INT)(xErrno)<(INT)eERRNO_NET_END)


	const char *GetGSPError( EnumErrno eErrno );


} //end namespace GSP

#endif //end _GS_H_MYSOCKET_H_