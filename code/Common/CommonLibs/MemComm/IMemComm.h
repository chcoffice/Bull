#ifndef IMEMCOMM_H_
#define IMEMCOMM_H_

#include "GSCommon.h"
#include "GSType.h"
#include "GSDefine.h"

#include <string>
using namespace std;

#define ERROR_MEMCOMM_OPER_SUCCESS			ERROR_BASE_SUCCESS
#define ERROR_MEMCOMM_START					(ERROR_BASE_START+5000)
#define ERROR_MEMCOMM_OPER_FAILED			(ERROR_MEMCOMM_START + 1)
#define ERROR_MEMCOMM_BUFFER_NOT_EXIST		(ERROR_MEMCOMM_START + 2)//缓冲区不存在
#define ERROR_MEMCOMM_BUFFERID_NOT_FIT		(ERROR_MEMCOMM_START + 3)//缓冲区ID不对
#define ERROR_MEMCOMM_BUFFER_NOT_ENOUGH		(ERROR_MEMCOMM_START + 4)//缓冲区不够
#define ERROR_MEMCOMM_BUFFER_IS_EMPTY		(ERROR_MEMCOMM_START + 5)//缓冲区为空
#define ERROR_MEMCOMM_BUFFER_NOT_LOCK		(ERROR_MEMCOMM_START + 6)//缓冲区没加锁

typedef enum
{
	MEM_COMM_OPEN,	// 只打开
	MEM_COMM_CREATE,						// 只创建
	MEM_COMM_OPEN_CREATE					// 先尝试打开，失败则创建	
} EnumOperType;

class IMemComm
{
public:
	IMemComm(void){};
	virtual ~IMemComm(void){};
public:
	/*************************************************
	  Function:   Create 
	  DateTime:   2010-5-21 14:48:52   
	  Description: 创建消息缓冲区
	  Input:   dwSize   消息缓冲区大小 
	           bLock    是否加锁，TRUE，加锁；FALSE，不加锁
			   bLocal	是否本地消息缓冲区，TRUE，加锁；FALSE，不加锁
			   opType	创建消息缓冲区方式，EnumOperType类型
			   strName  消息缓冲区的名称，bLocal为FALSE时，有效
	  Output:  无
	  Return:  TRUE，创建成功；FALSE，创建失败
	  Note:      
	*************************************************/
	virtual BOOL Create(UINT32 dwSize, BOOL bLock, BOOL bLocal,EnumOperType opType, string &strName) = 0;

	/*************************************************
	  Function:   Reset
	  DateTime:   2010-5-21 14:49:01   
	  Description: 重置消息缓冲区
	  Input:   无     
	             
	  Output:  无
	  Return:  无 
	  Note:      
	*************************************************/
	virtual void Reset(void) = 0;

	/*************************************************
	  Function:   Free
	  DateTime:   2010-5-21 14:49:09   
	  Description: 释放消息缓冲区资源
	  Input:  无     
	             
	  Output: TRUE，释放消息缓冲区资源成功；FALSE，释放消息缓冲区资源失败   
	  Return: bLocal
	  Note:      
	*************************************************/
	virtual BOOL Free(void) = 0;

	/*************************************************
	  Function:   Read
	  DateTime:   2010-5-21 14:49:16   
	  Description: 从消息缓冲区中读取一条消息
	  Input:    pBuf			存放所读取消息的空间地址
				dwMaxLen		存放所读取消息的空间大小
				dwMilliSeconds  读取超时时间，毫秒计算
	  Output:   pBuf			从消息缓冲区里读取的消息  
				pRealLen		所读取消息的长度
	  Return:   ERROR_MEMCOMM_OPER_SUCCESS			操作成功
				ERROR_MEMCOMM_BUFFER_NOT_EXIST		消息缓冲区不存在
				ERROR_MEMCOMM_BUFFERID_NOT_FIT		消息缓冲区ID不正确
				ERROR_MEMCOMM_BUFFER_IS_EMPTY       消息缓冲区无数据
				ERROR_MEMCOMM_BUFFER_NOT_ENOUGH     缓冲区过小，无法容纳消息
	  Note:      
	*************************************************/
	virtual INT32 Read(void *pBuf, UINT32 dwMaxLen, UINT32 *pRealLen, UINT32 dwMilliSeconds) = 0;
	
	/*************************************************
	  Function:   Write
	  DateTime:   2010-5-21 14:49:33   
	  Description: 在消息缓冲区的尾部写入一条消息
	  Input:   pBuf				所写消息地址  
	           dwLen			所写消息长度
			   dwMilliSeconds   写超时时间，毫秒计算
	  Output:    
	  Return:  ERROR_MEMCOMM_OPER_SUCCESS		操作成功
			   ERROR_MEMCOMM_BUFFER_NOT_EXIST   消息缓冲区不存在
			   ERROR_MEMCOMM_BUFFERID_NOT_FIT   消息缓冲区ID不正确
			   ERROR_MEMCOMM_BUFFER_NOT_ENOUGH  消息缓冲区过小
	  Note:      
	*************************************************/
	virtual INT32 Write(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds) = 0;

	/*************************************************
	  Function:   WriteUrgent
	  DateTime:   2010-5-21 14:49:42   
	  Description: 在消息缓冲区的头部写入一条消息
	  Input:  pBuf				所写消息地址
			  dwLen				所写消息长度
			  dwMilliSeconds    写超时时间，毫秒计算
	             
	  Output: 无 
	  Return: ERROR_MEMCOMM_OPER_SUCCESS		操作成功
			  ERROR_MEMCOMM_BUFFER_NOT_EXIST	消息缓冲区不存在
			  ERROR_MEMCOMM_BUFFERID_NOT_FIT	消息缓冲区ID不正确
			  ERROR_MEMCOMM_BUFFER_NOT_LOCK		消息缓冲区没加锁
			  ERROR_MEMCOMM_BUFFER_NOT_ENOUGH	消息缓冲区过小
	  Note:      
	*************************************************/
	virtual INT32 WriteUrgent(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds) = 0;

};
extern IMemComm *GetMemInstance();
extern void ClearMemInstance(IMemComm *pMemComm);
#endif


