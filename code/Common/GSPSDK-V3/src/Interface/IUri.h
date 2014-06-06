/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : IURI.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/13 15:46
Description:  URI 分析接口

* scheme://host:port/path/filename?attrname&attrname1=attrvalue1

* scheme = 通信协议 (常用的http,ftp,maito 等)
* host = 主机 (域名或IP)
* port = 端口号(可选)
* path = 路径
* filename = 名称
* attrname 属性名
* attrname1 属性1名
* attrvalue1 属性1的值

* host:port 为NetInfo
* path/filename 为Key
********************************************
*/

#ifndef _GS_H_IURI_H_
#define _GS_H_IURI_H_




#include "GSPConfig.h"


namespace GSP
{

// URI 属性 Attributive
#define MAX_URI_ATTRI_NAME_LEN 32
#define MAX_URI_ATTRI_VALUE_LEN 1024
typedef struct _StruUriAttr
{
	char szName[MAX_URI_ATTRI_NAME_LEN];
	char szValue[MAX_URI_ATTRI_VALUE_LEN];
}StruUriAttr;


/*
********************************************************************
类注释
类名    :    CIUri 
作者    :    zouyx
创建时间:    2012/4/24 11:01
类描述  :    URI 分析打包器接口
*********************************************************************
*/

class CIUri
{    
public:
    virtual ~CIUri(void)
	{

	}

	/*
	 *********************************************
	 Function : Clear
	 DateTime : 2012/4/24 11:01
	 Description :  清除所有内容
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
    virtual void Clear(void) = 0;

	/*
	 *********************************************
	 Function : GetScheme
	 DateTime : 2012/4/24 11:01
	 Description :  返回 URI 的 Scheme 项
	 Input :  
	 Output : 
	 Return : 失败返回NULL
	 Note :   
	 *********************************************
	 */
    virtual const char *GetScheme(void) const = 0;

	/*
	 *********************************************
	 Function : SetScheme
	 DateTime : 2012/4/24 11:02
	 Description :  设置 URI 的 Scheme 项
	 Input :  szScheme 设置的值
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
    virtual void  SetScheme(const char *szScheme) = 0;

	/*
	*********************************************
	Function : GetHost
	DateTime : 2012/4/24 11:01
	Description :  返回 URI 的 Host 项
	Input :  
	Output : 
	Return : 失败返回NULL
	Note :   
	*********************************************
	*/
    virtual const char *GetHost(void) const = 0;

	/*
	*********************************************
	Function : SetHost
	DateTime : 2012/4/24 11:02
	Description :  设置 URI 的 Host 项
	Input :  szHost 设置的值
	Output : 
	Return : 
	Note :   
	*********************************************
	*/
    virtual void  SetHost( const char *szHost ) = 0;



	/*
	*********************************************
	Function : GetHost
	DateTime : 2012/4/24 11:01
	Description :  返回 URI 的 Port 项
	Input :  
	Output : 
	Return : 默认值为0
	Note :   
	*********************************************
	*/
    virtual UINT GetPort(void) const = 0;

	/*
	*********************************************
	Function : SetPortArgs
	DateTime : 2012/4/24 11:02
	Description :  设置 URI 的 Host 项
	Input :  iPort 设置的值
	Output : 
	Return : 
	Note :   
	*********************************************
	*/
    virtual void SetPortArgs(UINT iPort) = 0;
  
	/*
	*********************************************
	Function : GetKey
	DateTime : 2012/4/24 11:01
	Description :  返回 URI 的 Key 项
	Input :  
	Output : 
	Return : 失败返回NULL
	Note :   
	*********************************************
	*/
    virtual const char *GetKey(void) const = 0;

	/*
	*********************************************
	Function : SetKey
	DateTime : 2012/4/24 11:02
	Description :  设置 URI 的 Key 项
	Input :  szKey 设置的值
	Output : 
	Return : 
	Note :   
	*********************************************
	*/
    virtual void SetKey(const char *szKey) = 0;

	/*
	*********************************************
	Function : GetAttr
	DateTime : 2012/4/24 11:01
	Description : 查找属性项
	Input :  szAttrName 查找的属性名
	Output : 
	Return : 失败返回NULL, 如果有多个同名属性返回 其中一个
	Note :   
	*********************************************
	*/
    virtual StruUriAttr *GetAttr(const char *szAttrName) const = 0;

	/*
	*********************************************
	Function : AttrBegin
	DateTime : 2012/4/24 11:01
	Description : 返回第一个属性项
	Input :  
	Output : 
	Return : 失败返回NULL
	Note :   
	*********************************************
	*/
	virtual StruUriAttr *AttrBegin(void)= 0;

	/*
	 *********************************************
	 Function : AttrNext
	 DateTime : 2012/4/24 11:09
	 Description :  获得属性的下一项属性
	 Input :  pAttr 当前属性
	 Output : 
	 Return : 结尾返回NULL
	 Note :   
	 *********************************************
	 */
	virtual StruUriAttr *AttrNext(const StruUriAttr *pAttr) = 0;



	/*
	 *********************************************
	 Function : AddAttr
	 DateTime : 2012/4/24 11:07
	 Description :  增加属性项， 同一个属性项可以添加多个
	 Input :  szAttrName 属性名
	 Input : szAttrValue 属性值
	 Output : 
	 Return : TRUE/FALSE
	 Note :   
	 *********************************************
	 */
	virtual BOOL AddAttr( const char *szAttrName, const char *szAttrValue ) = 0;

	/*
	 *********************************************
	 Function : ClearAttr
	 DateTime : 2012/4/24 11:14
	 Description :  清除属性 
	 Input :  szName 被清除属性的属性名， 同名的所有属性都会被清除
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	virtual void ClearAttr( const char *szName) = 0; 

	/*
	 *********************************************
	 Function : Analyse
	 DateTime : 2012/4/24 11:16
	 Description :  分析URI
	 Input :  szURI 本分析URI 字符串
	 Output : 
	 Return : TRUE/FLASE
	 Note :   
	 *********************************************
	 */
    virtual BOOL Analyse(const char *szURI ) = 0;

	/*
	 *********************************************
	 Function : GetURI
	 DateTime : 2012/4/24 11:17
	 Description :  返回URI 字符串
	 Input :  
	 Output : 
	 Return : 失败返回NULL
	 Note :   
	 *********************************************
	 */
    virtual const char *GetURI(void) const = 0;

	/*
	 *********************************************
	 Function : Clone
	 DateTime : 2012/4/24 11:17
	 Description :  拷贝当前对象
	 Input :  
	 Output : 
	 Return : 失败返回NULL
	 Note :   
	 *********************************************
	 */
	virtual CIUri *Clone(void) const = 0;
};


};

#endif //end _GS_H_IURI_H_
