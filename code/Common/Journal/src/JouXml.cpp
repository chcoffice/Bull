#include "JouXml.h"


using namespace JOU;

/*
*********************************************************************
*
*@brief : CJouXmlMaker 定义
*
*********************************************************************
*/


CJouXmlMaker::CJouXmlMaker(void)
:m_vColName()
,m_csTables()
,m_iCols(0)
{
	XMLNode::setGlobalOptions(XMLNode::char_encoding_GB2312);
}

CJouXmlMaker::~CJouXmlMaker(void)
{

}

BOOL CJouXmlMaker::Init(const std::vector<CGSString> &vColName )
{

	m_vColName = vColName;
	m_csTables.clear();
	m_iCols = m_vColName.size();
	return TRUE;
}

BOOL CJouXmlMaker::AddRow(void)
{
	GS_ASSERT_RET_VAL(m_iCols>0, FALSE);
	m_csTables.push_back( CXmlRow(m_iCols) );
	return TRUE;
}

BOOL  CJouXmlMaker::PutRowValue( INT iColIndex,const CGSString &strValue )
{
	if( m_csTables.size()<1  || iColIndex<0 || iColIndex>=m_iCols )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	CXmlTableIterator csIt = m_csTables.end();
	csIt--;
	CXmlRow *pRow =  &(*csIt);
	(*pRow)[iColIndex] = strValue;	
	return TRUE;
}

BOOL CJouXmlMaker::SerialToXml( CGSString &oStrValue )
{
	//增加头

	XMLNode stDoc;
	XMLNode stNode,stTemp, stRow;
	char czTemp[64];
	int i;
	CXmlTableIterator csIt;

	oStrValue.clear();
	GS_ASSERT_RET_VAL(m_iCols>0, FALSE);	

	stDoc = XMLNode::createXMLTopNode("RESULT");	


	//描述
	stNode = stDoc.addChild("DESCRI");

	stTemp = stNode.addChild("ROWNUM");


	GS_SNPRINTF(czTemp,64, "%d", (int)m_csTables.size());
	if( NULL== stTemp.addText(czTemp) )
	{
		GS_ASSERT_RET_VAL(0, FALSE);
	}

	stTemp = stNode.addChild("COLNUM");


	GS_SNPRINTF(czTemp,64, "%d", m_iCols);
	if( NULL== stTemp.addText(czTemp) )
	{
		GS_ASSERT_RET_VAL(0, FALSE);
	}

	stNode =  stNode.addChild("COLNAME");
	for( i = 0; i<m_iCols; i++ )
	{

		stTemp  = stNode.addChild("COL");
		GS_SNPRINTF(czTemp,64, "%d", i);
		if(NULL== stTemp.addAttribute("Index", czTemp ) )
		{
			GS_ASSERT_RET_VAL(0, FALSE);
		}
		if(NULL== stTemp.addAttribute("NAME", m_vColName[i].c_str() ) )
		{
			GS_ASSERT_RET_VAL(0, FALSE);
		}

	}	

	// <DESCRI> 域结束
	stNode = stDoc.addChild("ROWDATA");


	for( csIt = m_csTables.begin(); csIt != m_csTables.end(); csIt++)
	{
		stRow = stNode.addChild("ROW");

		for( i = 0; i<m_iCols; i++ )
		{
			GS_SNPRINTF(czTemp, 64,"%d", i);
			stTemp = stRow.addChild(czTemp);

			if( !((*csIt)[i].empty()) &&  NULL == stTemp.addText( (*csIt)[i].c_str() ) )
			{
				GS_ASSERT_RET_VAL(0, FALSE);
			}
		}
	}


	char *czXML = stDoc.createXMLString();

	GS_ASSERT_RET_VAL(czXML, FALSE);

	oStrValue = czXML;
	freeXMLString(czXML);
	return TRUE;

}


/*
*********************************************************************
*
*@brief : CJouXmlParser 定义
*
*********************************************************************
*/


CJouXmlParser::CJouXmlParser(void)
:m_vColName()
,m_csTables()
,m_iCols(0)
{
	XMLNode::setGlobalOptions(XMLNode::char_encoding_GB2312);
	m_csCur = m_csTables.end();
}

CJouXmlParser::~CJouXmlParser(void)
{

}

BOOL CJouXmlParser::Init( const char *czXML, INT iXMLSize)
{
	XMLResults stRes;
	XMLNode stDoc, stDescri, stNode, stTempNode;
	const char *czText;
	const char *pEnd;
	int iRow = 0;	
	int i,j, iCnts, iIdx;

	m_iCols = 0;
	m_vColName.clear();
	m_csTables.clear();
	m_csCur=m_csTables.end();

	if( iXMLSize < 1)
	{
		assert(0);
		return FALSE;
	}

	pEnd = czXML+iXMLSize-1;	

	if( *pEnd != '\0' )
	{
		assert(0);
		*(char*)pEnd = '\0';
	}

	stDoc = XMLNode::parseString(czXML, NULL, &stRes );

	if( stRes.error )
	{
		assert(0);
		return FALSE;
	}

	stDescri = stDoc.getChildNode( "DESCRI");
	GS_ASSERT_RET_VAL( !stDescri.isEmpty(), FALSE );

	stNode = stDescri.getChildNode("ROWNUM");
	GS_ASSERT_RET_VAL( !stNode.isEmpty(), FALSE );


	czText = stNode.getText();
	GS_ASSERT_RET_VAL( czText, FALSE );

	iRow = atoi(czText);


	stNode = stDescri.getChildNode("COLNUM");
	GS_ASSERT_RET_VAL( !stNode.isEmpty(), FALSE );


	czText = stNode.getText();
	GS_ASSERT_RET_VAL( czText, FALSE );

	m_iCols = atoi(czText);

	m_vColName.resize(m_iCols, CGSString() );

	stNode = stDescri.getChildNode("COLNAME");
	GS_ASSERT_RET_VAL( !stNode.isEmpty(), FALSE );

	iCnts = stNode.nChildNode();

	GS_ASSERT_RET_VAL( iCnts>=m_iCols , FALSE );


	for( i = 0, iCnts=0; i<m_iCols; i++ )
	{

		stTempNode = stNode.getChildNode("COL", i);
		if( stTempNode.isEmpty() )
		{
			continue;
		}
		czText = stTempNode.getAttribute("Index");
		GS_ASSERT_RET_VAL(czText, FALSE );
		iIdx = atoi(czText);

		if( iIdx<0 || iIdx>=(int)m_vColName.size() )
		{
			GS_ASSERT(0);
			return FALSE;
		}	
		czText = stTempNode.getAttribute("Name");
		GS_ASSERT_RET_VAL(czText, FALSE );
		m_vColName[iIdx] = czText;
		iCnts++;
	}

	GS_ASSERT_RET_VAL(m_iCols == iCnts, FALSE );

	stDescri = stDoc.getChildNode( "ROWDATA");

	if( iRow!=stDescri.nChildNode("ROW") )
	{

		GS_ASSERT(0);
	}
	else if( iRow == 0 )
	{
		m_csCur=m_csTables.begin();
		return TRUE;
	}
	CXmlTableIterator csIt;
	CXmlRow *pRow;
	CXmlRow vStr;

	for( int i = 0; i<m_iCols; i++ )
	{
		char czTemp[64];
		GS_SNPRINTF(czTemp, 64, "%d", i );
		vStr.push_back(czTemp);
	}

	for(  i = 0; i<iRow; i++ )
	{
		stNode = stDescri.getChildNode("ROW",  i);
		if( stNode.isEmpty() )
		{
			GS_ASSERT(0);
			continue;
		}

		m_csTables.push_back( CXmlRow(m_iCols));
		csIt = m_csTables.end();
		csIt--;
		pRow =  &(*csIt);
		for(  j=0; j<m_iCols; j++ )
		{
			stTempNode = stNode.getChildNode(vStr[j].c_str() );
			if( stTempNode.isEmpty() )
			{
				//无值
				continue;
			}
			czText = stTempNode.getText();
			if( czText )
			{
				(*pRow)[j] = czText;
			}
			else
			{
				(*pRow)[j] = ""; //空值
			}

		}
	}
	m_csCur=m_csTables.begin();
	return TRUE;
}

BOOL CJouXmlParser::GetColName( std::vector<CGSString> &vColName )
{
	if( m_iCols<1 )
	{
		return FALSE;
	}
	vColName = m_vColName;
	return TRUE;
}

INT32 CJouXmlParser::GetRows(void)
{
	return m_csTables.size();
}

BOOL CJouXmlParser::IsEof(void)
{
	return ( m_csCur==m_csTables.end() );
}

BOOL CJouXmlParser::MoveFirst(void)
{
	m_csCur=m_csTables.begin();
	return IsEof();
}

void CJouXmlParser::Next(void)
{
	if( !IsEof() )
	{
		m_csCur++;
	}

}


BOOL CJouXmlParser::GetValue(INT iColIndex, CGSString &oStrValue )
{
	oStrValue.clear();
	if( IsEof() || iColIndex>=m_iCols )
	{
		assert(iColIndex<m_iCols);
		return FALSE;
	}
	CXmlRow *pRow =  &(*m_csCur);
	oStrValue = (*pRow)[iColIndex];
	return TRUE;

}