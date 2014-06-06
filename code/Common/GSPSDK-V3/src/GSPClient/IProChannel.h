/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : IPROCHANNEL.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/28 11:09
Description: 协议通道接口定义
********************************************
*/

#ifndef _GS_H_IPROCHANNEL_H_
#define _GS_H_IPROCHANNEL_H_

#include "GSPObject.h"
#include "Uri.h"

namespace GSP
{

	
	/*
	********************************************************************
	类注释
	类名    :    CIProChannel
	作者    :    zouyx
	创建时间:    2012/4/24 10:22
	类描述  :		客户端协议通道的接口定义
	*********************************************************************
	*/
	
class CIProChannel :
	public CGSPObject
{
public:
	CIProChannel(void);
	virtual ~CIProChannel(void);

	/*
	 *********************************************
	 Function : Open
	 DateTime : 2012/4/24 10:22
	 Description :  打开URI
	 Input :  csUri  打开的URI
	 Input : bAsync 是异步
	 Input : iTimeouts 超时时间， 毫秒
	 Output : 
	 Return : 参考EnumErrno定义
	 Note :   
	 *********************************************
	 */
	virtual EnumErrno Open(const CUri &csUri, BOOL bAsync, INT iTimeouts) = 0;

	/*
	*********************************************
	Function : Ctrl
	DateTime : 2012/4/24 10:22
	Description :  控制通道
	Input :  stCtrl  控制的命令
	Input : bAsync 是异步
	Input : iTimeouts 超时时间， 毫秒
	Output : 
	Return : 参考EnumErrno定义
	Note :   
	*********************************************
	*/
	virtual EnumErrno Ctrl(const StruGSPCmdCtrl &stCtrl, BOOL bAsync,INT iTimeouts) = 0;

	virtual EnumErrno CtrlOfManstrsp(const char *czMansrtsp, BOOL bAsync,INT iTimeouts,StruGSPCmdCtrl &stGspCtrl)
	{
		return eERRNO_SYS_EFUNC;
	}

	/*
	 *********************************************
	 Function : FlowCtrl
	 DateTime : 2012/4/24 10:24
	 Description :  进行流控
	 Input :  bStart 开始或结束
	 Output : 
	 Return : 参考EnumErrno定义
	 Note :   
	 *********************************************
	 */
	virtual EnumErrno FlowCtrl( BOOL bStart ) = 0;



	/*
	 *********************************************
	 Function : DestoryBefore
	 DateTime : 2012/4/24 10:24
	 Description :  对象释放前调用,用以释放对象资源
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	virtual void DestoryBefore(void) = 0;


	virtual CGSString GetSdp(void)
	{
		return CGSString();
	}
};

} //end namespace GSP

#endif //end _GS_H_IPROCHANNEL_H_
