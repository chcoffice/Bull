/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : JOUXML.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/20 14:01
Description: 把查询数据和结果集之间的转换
********************************************
*/

#ifndef _GS_H_JOUXML_H_
#define _GS_H_JOUXML_H_
#include "GSCommon.h"
#include <vector>
#include <list>


/*
*********************************************************************
*
*@brief : XML 格式说明, 大小写敏感
*

<?xml version="1.0" ?>
<RESULT>
	<DESCRI>
		<ROWNUM>行数</ROWNUM>
		<COLNUM>列数</COLNUM>
		<COLNAME>
			<COL  Index=列索引号 Name="列名"/>
			.
			.
			.
			<COL  Index=列索引号 Name="列名"/>
		</COLNAME>
	<DESCRI>

	<ROWDATA>
		<ROW>
			<列索引号>值</列索引号>
			.
			.
			.
			<列索引号>值</列索引号>
		</ROW>
		.
		.
		.
	
	</ROWDATA>
</RESULT>



*********************************************************************
*/


namespace JOU
{
	typedef std::vector<CGSString> CXmlRow;
	typedef std::list<CXmlRow> CXmlTable;
	typedef std::list<CXmlRow>::iterator CXmlTableIterator;

	
	/*
	********************************************************************
	类注释
	类名    :    CJouXmlMaker
	作者    :    zouyx
	创建时间:    2011/9/20 14:13
	类描述  :		合成XML
	*********************************************************************
	*/
	
class CJouXmlMaker
{
public:
	CJouXmlMaker(void);
	~CJouXmlMaker(void);
	
	/*
	 *********************************************
	 Function : Init
	 DateTime : 2011/9/20 14:05
	 Description :  初始化
	 Input :  vColName 列名
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	BOOL Init(const std::vector<CGSString> &vColName );

	/*
	 *********************************************
	 Function : AddRow
	 DateTime : 2011/9/20 14:06
	 Description :  添加一行数据
	 Input :  
	 Output : 
	 Return : TRUE/FALSE
	 Note :   
	 *********************************************
	 */
	BOOL AddRow(void);

	/*
	 *********************************************
	 Function : PutValue
	 DateTime : 2011/9/20 14:07
	 Description :  写行数据
	 Input :  iColIndex 列号
	 Input :  strValue 添加的数据
	 Output : 
	 Return : TRUE/FALSE
	 Note :   
	 *********************************************
	 */
	BOOL PutRowValue( INT iColIndex,const CGSString &strValue );


	/*
	 *********************************************
	 Function : SerialToXml
	 DateTime : 2011/9/20 14:09
	 Description :  转化为XML文本
	 Input :  oStrValue 返回的数据
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	BOOL SerialToXml( CGSString &oStrValue );
private :

	CXmlRow m_vColName; //列名
	CXmlTable m_csTables;
	INT m_iCols; //列数
};



/*
********************************************************************
类注释
类名    :    CJouXmlParser
作者    :    zouyx
创建时间:    2011/9/20 14:13
类描述  :    解析分解XML
*********************************************************************
*/

class CJouXmlParser
{
public:
	CJouXmlParser(void);
	~CJouXmlParser(void);

	BOOL Init(const char *czXML, INT iXMLSize );

	BOOL GetColName( std::vector<CGSString> &vColName );

	INT32 GetRows(void);

	BOOL IsEof(void);

	BOOL MoveFirst(void);

	void Next(void);

	/*
	 *********************************************
	 Function : GetValue
	 DateTime : 2011/9/20 14:15
	 Description :  获取当前行的列数据
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	BOOL GetValue(INT iColIndex, CGSString &oStrValue );
private :
	CXmlRow m_vColName; //列名
	CXmlTable m_csTables;
	INT m_iCols; //列数
	CXmlTableIterator m_csCur; //游标

};

} //end namespace GSS

#endif //end _GS_H_JOUXML_H_