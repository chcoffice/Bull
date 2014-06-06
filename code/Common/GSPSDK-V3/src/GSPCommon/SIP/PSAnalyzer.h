/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : PSANALYZER.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/5/15 8:52
Description: 分析 格式化 PS 流
********************************************
*/

#ifndef _GS_H_PSANALYZER_H_
#define _GS_H_PSANALYZER_H_
#include "../GSPObject.h"
#include "../GSPMemory.h"
#include "ES2PS.h"
#include "../RefObjVector.h"

namespace GSP
{
namespace SIP
{

	/*
	*********************************************************************
	*
	*@brief : 
	*
	*********************************************************************
	*/
	class CPSSlice : public CRefObject
	{
	public :
		typedef struct _StruPESBody
		{
			const BYTE *pBody;
			UINT iBodySize;
			INT iPSStreamID;
			UINT64 iPTS;
			UINT64 iDTS;
		}StruPESBody;

	private :		
		CRefObject *m_pRefHeader;	
		CRefObject *m_pRefPES;	
		INT m_iPSHeaderType; 
		const BYTE *m_bPSHeader;
		UINT m_iPSHeaderSize;
		StruPESBody m_stPesBody;
	public :
		//返回数据片的类型  PS_HEADER_PK_START/PS_HEADER_SMH ...
		INLINE INT GetPSHeaderType(void) const
		{
			return m_iPSHeaderType;
		}
		INLINE const BYTE *GetPSHeader(void) const
		{
			return m_bPSHeader;
		}
		INLINE UINT GetPSHeaderSize(void) const
		{
			return m_iPSHeaderSize;
		}
		
		INLINE const CPSSlice::StruPESBody &GetPESBody(void) const
		{
			return m_stPesBody;
		}

	

		EnumErrno ParserPSM( std::vector<StruPSStreamInfo> &vStreamInfo);	
		static CPSSlice *Create(const BYTE **pIOData, UINT &iIOSize);
		static CPSSlice *Create(CRefObject *pRefBuf, const BYTE **pIOData, UINT &iIOSize);

	protected :
		CPSSlice(void);
		virtual ~CPSSlice(void);
		/*
		 *********************************************
		 Function : Parser
		 Version : 1.0.0.1
		 Author : 邹阳星
		 DateTime : 2013/5/15 8:58
		 Description :  分析数据， 返回使用数据的大小
		 Input :  pInputData 输入的数据
		 Input : iInputSize 输入数据的大小
		 Input : pRefBuf 使用的 输入数据由此对象 引用
		 Output : 
		 Return : 返回使用数据的长度， 出错返回 -1
		 Note :   
		 *********************************************
		 */
		INT  Parser( const BYTE *pInputData, UINT iInputSize );
		INT  Parser(CRefObject *pRefBuf, const BYTE *pInputData, UINT iInputSize );
	};


	/*
	*********************************************************************
	*
	*@brief : 
	*
	*********************************************************************
	*/
	


	typedef CRefObjVector<CPSSlice*> CVectorPSSlice;

	class CPSSliceParser : public CGSPObject
	{
	public :
		static EnumErrno Slice( CFrameCache *pPSFrame, BOOL bExistGSFHeader, CVectorPSSlice &vResult );
	};


	



} //end namespace SIP
} //end namespace GSP

#endif //end _GS_H_PSANALYZER_H_
