#ifndef CRECORDSET_DEF_H
#define CRECORDSET_DEF_H

// 头文件
#include "CConnection.h"
// OuterInterface头文件
#include "../OuterInterface/IDBAccessModule.h"

/**************************************************************************************************
Copyright (C), 2010-2011, GOSUN 
File name : CRECORDSET.H      
Author :  liujs      
Version : Vx.xx        
DateTime : 2010/5/22 14:35
Description : 
数据集对象类
**************************************************************************************************/
namespace	DBAccessModule
{

	class CRecordSet:public IRecordSet
	{
	public:
		CRecordSet(void);
		virtual ~CRecordSet(void);

	public:
		// OuterInterface之间继承IRecordSet类 [4/26/2010 liujs]

		//获取二进制大数据
		BOOL	GetBlobCollect(const char* szFieldName,string&	strValue);

	public:
		// 内部接口的定义
		// 查询接口
		virtual	IRecordSet*		QuerySql(const char*  szSql) = 0;

		
	};

}
#endif // CRECORDSET_DEF_H

