#include <vector>
#include "GSPMemory.h"
#include "Log.h"

//#define DEBUG_MEMORY_STACK 1

#ifdef DEBUG_MEMORY_STACK

#if defined(_WIN32) || defined(_WIN64)
#pragma comment(lib, "version.lib")  // for "VerQueryValue"
#pragma comment(lib, "Dbghelp.lib")  // for "VerQueryValue"
#include <dbghelp.h>
#endif
#endif

#ifdef _DEBUG 
static FILE *s_fpTest = NULL;
#ifdef MY_DEBUG
#undef  MY_DEBUG
#endif

#ifdef _LINUX
#define MY_DEBUG(x, ...)  do{ if( s_fpTest ) fprintf(s_fpTest, x, ##__VA_ARGS__);} while(0)
#else
#define MY_DEBUG(x, ...)  do{ if( s_fpTest ) fprintf(s_fpTest, x, __VA_ARGS__);} while(0)
#endif
#endif


using namespace  GSP;

namespace GSP
{
	

	typedef struct _StruMemListNode StruMemListNode;
	class CSliceManager;

#ifdef WIN32
#pragma pack( push,1 )
#endif

	typedef union
	{
		StruMemListNode *pNext;
		CSliceManager *pManager;
	}GS_MEDIA_ATTRIBUTE_PACKED ListPointer;

	typedef struct _StruMemListNode
	{	
		ListPointer P;
#ifdef DEBUG_MEMORY_STACK
		CGSString *pStrStack;
#endif
		UINT64 iSize;
		BYTE pBuf[8];
	} GS_MEDIA_ATTRIBUTE_PACKED StruMemListNode; 



#ifdef WIN32
#pragma pack( pop )
#endif

#ifdef DEBUG_MEMORY_STACK
#define BUFFER_OFFSET  (sizeof(ListPointer)+sizeof(CGSString *)+sizeof(UINT64))
#else
#define BUFFER_OFFSET  (sizeof(ListPointer)+sizeof(UINT64))
#endif


#if  defined(DEBUG_MEMORY_STACK) && defined(_WIN32)

	static HANDLE s_hProcess=NULL;
	static CGSMutex s_hMutex;


	static std::string addressToString( DWORD address )  
	{  
		std::stringstream oss;  
		// First the raw address  

		// Then any name for the symbol  
		struct tagSymInfo  
		{  
			IMAGEHLP_SYMBOL symInfo;  
			char nameBuffer[ 1024 ];  
		} SymInfo;  
		IMAGEHLP_SYMBOL * pSym = &SymInfo.symInfo;  
		pSym->SizeOfStruct = sizeof( SymInfo );
		pSym->MaxNameLength = 1024;  
		DWORD dwDisplacement = 0;  
		if ( ::SymGetSymFromAddr( s_hProcess, address, &dwDisplacement, pSym) )  
		{  

			oss << pSym->Name;		
		}  


		//  		// Finally any file/line number  
		// 		IMAGEHLP_LINE lineInfo = { sizeof( IMAGEHLP_LINE ) };  
		// 		dwDisplacement = 0;
		// 		if ( ::SymGetLineFromAddr( s_hProcess, address, &dwDisplacement, &lineInfo ) )  
		// 		{  
		// 			char const *pDelim = strrchr( lineInfo.FileName, '//' );  
		// 			//oss << " at " << ( pDelim ? pDelim + 1 : lineInfo.FileName )
		// 			oss << " (" << lineInfo.LineNumber << ")";  
		// 		}  
		return oss.str();  
	}  

#define GET_CURRENT_CONTEXT(c, contextFlags) \
	do { \
	memset(&c, 0, sizeof(CONTEXT)); \
	c.ContextFlags = contextFlags; \
	__asm    call x \
	__asm x: pop eax \
	__asm    mov c.Eip, eax \
	__asm    mov c.Ebp, ebp \
	__asm    mov c.Esp, esp \
	} while(0);


	static void _GetStack( CGSString &strValue)
	{
		strValue.clear();
		STACKFRAME stackFrame = {0};  
		PCONTEXT pContext = NULL;
		CONTEXT c;
		HANDLE hThread = ::GetCurrentThread();

		if(hThread==INVALID_HANDLE_VALUE  )
		{
			GS_ASSERT(0);
			return;
		}
		GET_CURRENT_CONTEXT(c, CONTEXT_FULL);

		bzero(&stackFrame, sizeof(stackFrame));
		pContext = &c;
		stackFrame.AddrPC.Offset = pContext->Eip;  
		stackFrame.AddrPC.Mode = AddrModeFlat;  
		stackFrame.AddrFrame.Offset = pContext->Ebp;  
		stackFrame.AddrFrame.Mode = AddrModeFlat;  
		stackFrame.AddrStack.Offset = pContext->Esp;  
		stackFrame.AddrStack.Mode = AddrModeFlat;  
		std::stringstream oss;  
		int iDeep = 0;

		CGSAutoMutex locker(&s_hMutex);
		while ( iDeep++<10  && ::StackWalk(  
			IMAGE_FILE_MACHINE_I386,  
			s_hProcess,  
			hThread, // this value doesn't matter much if previous one is a real handle  
			&stackFrame,   
			pContext,  
			NULL,  
			::SymFunctionTableAccess,  
			::SymGetModuleBase,  
			NULL )  )  
		{  
			if( iDeep<4 )
			{
				continue;
			}
			if( stackFrame.AddrFrame.Offset == 0 )
			{
				break;
			}
			oss << addressToString( stackFrame.AddrPC.Offset ) << "\r\n";  
		}
		strValue = oss.str();

	}

#endif



	class CSliceManager
	{
	public :
		UINT32 m_iSliceSize; //每块的大小
		UINT32 m_iAllowCounts; //允许的块数
		UINT32 m_iSliceCounts; //已经分配的个数
		UINT32 m_iFreeCounts; //当前空闲的个数
		StruMemListNode *m_pFree;

		//命中率
		UINT32 m_iHitCounts;  //命中
		UINT32 m_iNHitCounts; //不命中

		typedef struct _StruMemBlock
		{
			void *pBuf;
			size_t iSize;
			BOOL bMLock;
		}StruMemBlock;

		std::vector<StruMemBlock> m_vSysMem;
#ifdef DEBUG_MEMORY_STACK
		std::set<StruMemListNode*> m_setStack;
#endif

		CGSMutex m_csMutex;

		CSliceManager(void)
			:m_csMutex()
		{
			m_iSliceSize = 0; //每块的大小	
			m_iAllowCounts= 0; //允许的块数	
			m_iSliceCounts = 0; //已经分配的个数
			m_iFreeCounts = 0; //当前空闲的个数
			m_pFree = NULL;
			//命中率
			m_iHitCounts = 0;
			m_iNHitCounts = 0;
		}

		~CSliceManager(void)
		{

			double iRate = 1.0;
			if( 0<(m_iHitCounts+m_iNHitCounts) )
			{
				iRate = ((double)m_iHitCounts)/(m_iHitCounts+m_iNHitCounts);
			}		
			MY_DEBUG("***内存池: %d (Cache:%d) Bytes 命中:%d, 不命中: %d, 命中率: %.4f\n", 
				m_iSliceSize,m_iSliceCounts, m_iHitCounts, m_iNHitCounts, iRate);
			Unint();
		}
#ifdef ENABLE_MEM_LOCK_OF_PHY 
		BOOL LockInSys( void *pAddr, UINT iSize )
		{
#if  defined(_WIN32) || defined(_WIN64)

			if( ::VirtualLock(pAddr, iSize ) )
			{
				return TRUE;
			}
			else
			{

				if( ::VirtualLock(pAddr, iSize ) )
				{
					return FALSE;
				}				
			}
#elif defined(_LINUX)
			//LINUX
			if( 0== ::mlock(pAddr, iSize) )
			{
				return TRUE;
			}
#endif	
			return FALSE;
		}

		void UnlockOfSys( void *pAddr, UINT iSize )
		{
#if  defined(_WIN32) || defined(_WIN64)

			::VirtualUnlock(pAddr, iSize );
#elif defined(_LINUX)
			//LINUX		
			::munlock(pAddr, iSize);
#endif
		}


#endif   // #ifdef ENABLE_MEM_LOCK_OF_PHY 


#ifdef DEBUG_MEMORY_STACK
		void GetStackString( CGSString &strStack )
		{
			strStack.clear();
#ifdef _WIN32
			_GetStack(strStack);
#endif
		}
#endif //DEBUG_MEMORY_STACK



		INLINE static StruMemListNode *NewMemNode( UINT32 iBufSize )
		{
			INT  i  = iBufSize+BUFFER_OFFSET;
			GS_ASSERT(i>0);
			StruMemListNode *p =  (StruMemListNode *)::malloc(i);
			if( p )
			{
				bzero(p, BUFFER_OFFSET );
				p->iSize = i;
			}
			return p;
		}



		BOOL Init( UINT32 iSliceSize, UINT32 iTotalSize )
		{
			CGSAutoMutex locker( &m_csMutex);
			m_iAllowCounts = iTotalSize/iSliceSize;
			m_iAllowCounts++;
			m_iSliceSize = iSliceSize;


			UINT32 iNodeSize = BUFFER_OFFSET+iSliceSize+16;
			UINT32 iMallocSize; //分配的大小

			//对齐
			iNodeSize = (iNodeSize + ALIGNE_SIZE)&~ALIGNE_SIZE;

#ifdef _WINCE
			iMallocSize = ((UINT32)(1+(KBYTES*512)/(iNodeSize)))*(iNodeSize);
#else
			iMallocSize = ((UINT32)(1+(MBYTES*5)/(iNodeSize)))*(iNodeSize);
#endif

			BYTE *pBuf;
			UINT32 iTemp;
			StruMemListNode *p;
			StruMemBlock stBlock;




			while( m_iAllowCounts>m_iSliceCounts) 
			{
				iTemp = MIN( iNodeSize*(m_iAllowCounts-m_iSliceCounts), iMallocSize);
				pBuf = (BYTE*)::malloc(iTemp);
				if( pBuf==NULL )
				{
					GS_ASSERT(0);
					break;
				}			
				stBlock.bMLock = FALSE;
				//bzero(pBuf, iTemp);
#ifdef ENABLE_MEM_LOCK_OF_PHY 
				//锁定物理内存
				stBlock.bMLock = LockInSys(pBuf, iTemp);
#endif

				stBlock.iSize = iTemp;
				stBlock.pBuf = pBuf;

				m_vSysMem.push_back(stBlock);				
				while( iTemp>=iNodeSize )
				{
					p = (StruMemListNode *)pBuf;
					pBuf += iNodeSize;
					iTemp -= iNodeSize;
					bzero(p, BUFFER_OFFSET );
					p->P.pNext = m_pFree;  //加到空闲
					m_pFree = p;
					m_iSliceCounts++;
					m_iFreeCounts++;


#ifdef DEBUG_MEMORY_STACK
					p->pStrStack = new CGSString;	
					GS_ASSERT_EXIT( p->pStrStack, -1);
#endif
				}
			}
			GS_ASSERT(m_iSliceCounts >= m_iAllowCounts);
			return m_iSliceCounts>0;
		}

		void Unint(void)
		{
			CGSAutoMutex locker( &m_csMutex);
			GS_ASSERT(m_iFreeCounts==m_iSliceCounts ); //还有内存没有释放
			m_pFree = NULL;
#ifdef DEBUG_MEMORY_STACK
			m_setStack.clear();
			StruMemListNode *p;
			while(m_pFree )
			{
				p = m_pFree;
				m_pFree = p->P.pNext;
				delete p->pStrStack;
			}

#endif
			for( UINT i = 0; i<m_vSysMem.size(); i++ )
			{
#ifdef ENABLE_MEM_LOCK_OF_PHY 
				if( m_vSysMem[i].bMLock )
				{
					UnlockOfSys(m_vSysMem[i].pBuf, m_vSysMem[i].iSize);
				}
#endif
				::free(m_vSysMem[i].pBuf);
			}
			m_iFreeCounts = 0;
			m_iSliceCounts = 0;
			m_vSysMem.clear();

		}

		INLINE void *Pop(void)
		{

			m_csMutex.Lock();
			StruMemListNode *p = m_pFree;
			if( p  )
			{			
#ifdef DEBUG_MEMORY_STACK
				if( /*m_iSliceSize == 512 || */ m_iSliceSize == 384)
				{

					GetStackString(*(p->pStrStack));
					m_setStack.insert(p);
				}
#endif
				m_pFree = p->P.pNext;
				bzero(p, sizeof(BUFFER_OFFSET));	
				p->P.pManager = this;
				m_iFreeCounts--;
				m_iHitCounts++;
				m_csMutex.Unlock();			
				return &(p->pBuf[0]);
			}
			m_iNHitCounts++;
			m_csMutex.Unlock();
			return NULL;
		}

		INLINE void Push(StruMemListNode *pNode)
		{


			if( pNode->P.pManager==NULL )
			{
				::free(pNode);			
			}
			else
			{
				m_csMutex.Lock();
#ifdef DEBUG_MEMORY_STACK
				if( m_iSliceSize == 256 ||  m_iSliceSize == 32 )
				{

					m_setStack.erase(pNode);
				}
#endif
				pNode->P.pNext = m_pFree;
				m_pFree = pNode;
				m_iFreeCounts++;
				m_csMutex.Unlock();
			}
		}
	};



} //end namespace GSP


CMemoryPool *CMemoryPool::g_pIntance = NULL;

#ifdef ON_EMBEDDED
UINT64 CMemoryPool::g_iMemMaxSize = MBYTES*5;
#else
UINT64 CMemoryPool::g_iMemMaxSize = MBYTES*256;
#endif


void GspSetMaxCacheMemorySize( UINT iSizeMByte )
{
	if( iSizeMByte<5 )
	{
		iSizeMByte = 5;
	}
	CMemoryPool::g_iMemMaxSize  = MBYTES*iSizeMByte;
}

void CMemoryPool::InitModule(void)
{
#ifndef  MALLOC_FROM_SYSTEM 

#ifdef DEBUG_MEMORY_STACK
#if defined(_WIN32) || defined(_WIN64)
	s_hProcess = ::GetCurrentProcess();  
	DWORD dwOpts = ::SymGetOptions();  
	dwOpts |= SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS;  
	::SymSetOptions ( dwOpts );  
	char  *pSerachPath = "D:\\Devlep\\sp\\source\\trunk\\code\\Common\\GSPSDK-V2\\obj\\TestwxWidget\\Debug;D:\\Devlep\\sp\\source\\trunk\\code\\Common\\GSPSDK-V2\\test";

	if( !::SymInitialize( s_hProcess, pSerachPath, true ) )
	{
		int eErrno = GetLastError();
		GS_ASSERT(0);

	}
#endif


#ifdef ENABLE_MEM_LOCK_OF_PHY 
	//交换虚拟物理内存
#if 0
#if defined(_WIN32) || defined(_WIN64)

	DWORD pid = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA, FALSE, pid);
	GS_ASSERT(hProcess);
	if (hProcess != NULL)
	{

		BOOL result = SetProcessWorkingSetSize(hProcess, 
			MBYTES*50,   // min
			MBYTES*300);  // max
		CloseHandle(hProcess);
		hProcess = NULL;
		GS_ASSERT(result);
	}

#endif

#endif
#endif


#endif


	if( g_pIntance == NULL )
	{
		g_pIntance = new CMemoryPool();
	}

#endif

}

void CMemoryPool::UninitModule(void)
{
#ifndef  MALLOC_FROM_SYSTEM 


#if  defined(DEBUG_MEMORY_STACK) && defined(_WIN32) && defined(_WIN64)
	::SymCleanup( s_hProcess );
#endif

	if(g_pIntance )
	{
		delete g_pIntance;
		g_pIntance = NULL;
	}

#endif
}


CMemoryPool::CMemoryPool(void)
{
	m_iAllocFromSys = 0;
	Init();


}

CMemoryPool::~CMemoryPool(void)
{
	Uninit();
}


void CMemoryPool::Init(void)
{
#ifndef  MALLOC_FROM_SYSTEM 

#ifdef _DEBUG
	if( s_fpTest == NULL )
	{
		s_fpTest = fopen( "gspmemdg.txt", "a+");
		if( s_fpTest )
		{
			fseek( s_fpTest,0, SEEK_END);
			fprintf(s_fpTest, "\r\n\r\n++++++++++++++++++++++++++++++++++++\r\n\r\n" );
		}
	}
#endif




	struct _StruInitSizeList
	{
		UINT32 iSliceSize;
		UINT32 iTotalSize;
	};

	if( g_iMemMaxSize < MBYTES )
	{
		g_iMemMaxSize = MBYTES;
	}


	//按比例分配
	struct _StruInitSizeList stVList[] = 
	{				
		{64,    96}, 
		{192,   128},
		{384,   128},		
		{512,   384},		 
		{KBYTES*2, 1024*4},
		{KBYTES*32, 1024*8},
		{KBYTES*64, 1024*6},  
		{KBYTES*128, 1024*3}, 
		{KBYTES*192, 1024*4}, 	
		{KBYTES*256, 1024*4},
	};

	m_iVectorSize = ARRARY_SIZE(stVList);
	
	UINT64 iS = 0;	
	int i = 0;
	for( i = 0, iS = 0; i<m_iVectorSize; i++ )
	{
		iS += stVList[i].iTotalSize;
	}
	for(  i = 0; i<m_iVectorSize; i++ )
	{
		stVList[i].iTotalSize =  (UINT32) (1024ULL*stVList[i].iTotalSize/iS);  //计算为千分比
	}

	i = 0;
	iS = g_iMemMaxSize;
	while( iS && i<m_iVectorSize )
	{
		
		stVList[i].iTotalSize = (UINT32)(g_iMemMaxSize*stVList[i].iTotalSize/1024ULL);
		if( stVList[i].iTotalSize > iS )
		{
			break;
		}		
		iS -= stVList[i].iTotalSize;
		i++;
	}
	GS_ASSERT(i==m_iVectorSize);
	m_iVectorSize = i;


	
	m_vManager = (CSliceManager **) ::malloc(sizeof(CSliceManager *)*m_iVectorSize);
	GS_ASSERT_EXIT(m_vManager, -1);
	for( int i = 0; i<m_iVectorSize; i++ )
	{
		m_vManager[i] = new CSliceManager();
		GS_ASSERT_EXIT(m_vManager, -1);
		m_vManager[i]->Init(stVList[i].iSliceSize, stVList[i].iTotalSize);
	}
#endif 

}


void CMemoryPool::Uninit(void)
{
#ifndef  MALLOC_FROM_SYSTEM 

	for(int i = 0; i<m_iVectorSize; i++ )
	{
		delete m_vManager[i];
	}
	if( m_vManager )
	{
		::free(m_vManager);
		m_vManager = NULL;
	}
#ifdef _DEBUG
	if( s_fpTest )
	{
		fclose(s_fpTest);
		s_fpTest = NULL;
	}
#endif
#endif

}

BOOL CMemoryPool::IsNoMemory(void)
{
	if( g_pIntance && g_pIntance->m_iAllocFromSys>MBYTES*768 )
	{
		return TRUE;
	}
	return FALSE;
}

void *CMemoryPool::Malloc(UINT iSize )
{
#ifdef  MALLOC_FROM_SYSTEM 
	return ::malloc(iSize);
#else

	if( g_pIntance )
	{
		INT iTrys = 2;
		for( int i = 0; i<g_pIntance->m_iVectorSize; i++ )
		{
			if( iSize<=g_pIntance->m_vManager[i]->m_iSliceSize )
			{
				iTrys --;
				void *p =  g_pIntance->m_vManager[i]->Pop();			
				if( p )
				{
					return p;
				}
				else if( iTrys==0 )
				{
					break;
				}
			}

		}
	}
	//为什么不命中?? 
//	GS_ASSERT(0);
	if(g_pIntance && g_pIntance->m_iAllocFromSys >= MBYTES*1024 )
	{
		return NULL;
	}
	

	StruMemListNode *p = CSliceManager::NewMemNode(iSize);
	if( p )
	{
		if(g_pIntance)
		{

			g_pIntance->m_csMutex.Lock();
			g_pIntance->m_iAllocFromSys += p->iSize;
			g_pIntance->m_csMutex.Unlock();
		}
	}
	GS_ASSERT_RET_VAL(p, NULL);	
	return &p->pBuf[0];
#endif

}

void CMemoryPool::Free(void *pBuffer)
{
#ifdef  MALLOC_FROM_SYSTEM 
	return ::free(pBuffer);
#else
	if( !pBuffer )
	{
		GS_ASSERT(0);
		return;
	}
	StruMemListNode *p = (StruMemListNode *) (((BYTE*)pBuffer)-BUFFER_OFFSET);

	GS_ASSERT( &(p->pBuf[0]) == (BYTE*)pBuffer );	
	if( p->P.pManager )
	{			
		p->P.pManager->Push(p);
	}
	else
	{
		if(g_pIntance)
		{
			g_pIntance->m_csMutex.Lock();
			g_pIntance->m_iAllocFromSys -= p->iSize;
			g_pIntance->m_csMutex.Unlock();
		}
		::free(p);
	}
#endif
}



/*
*********************************************************************
*
*@brief : CFrameCache 实现
*
*********************************************************************
*/

CFrameCache::CFrameCache(void) 
: CRefObject()
,m_pBuf(NULL)
, m_stFrameInfo()
{

}

CFrameCache::~CFrameCache(void)
{
	SAFE_DESTROY_REFOBJECT(&m_pBuf);
}


CFrameCache *CFrameCache::Create(CRefObject *pRefObj,const void *pData,  UINT iSize )
{
	CPasteBuffer *pBuf = pBuf->Create(pRefObj, (const BYTE *)pData, iSize, iSize);
	if( !pBuf )
	{
		GS_ASSERT(0);
		return NULL;
	}
	CFrameCache *pRet = new CFrameCache();	
	if( !pRet )
	{
		GS_ASSERT(0);
		pBuf->UnrefObject();
		return NULL;
	}
	pRet->m_pBuf = pBuf;
	return pRet;	
}

CFrameCache *CFrameCache::Create(const StruBaseBuf *vBuf, UINT iSize )
{
	UINT iTemp = CGSPBuffer::SumArrayBufTotalSize(vBuf, iSize);
	CDynamicBuffer *pBuf = pBuf->Create(iTemp+128);

	if( !pBuf )
	{
		GS_ASSERT(0);
		return NULL;
	}
	iSize = pBuf->TryAppend(vBuf, iSize);
	GS_ASSERT(iSize==iTemp);
	CFrameCache *pRet = new CFrameCache();	
	if( !pRet )
	{
		GS_ASSERT(0);
		pBuf->UnrefObject();
		return NULL;
	}
	pRet->m_pBuf = pBuf;
	return pRet;	
}


CFrameCache *CFrameCache::MergeFront(const StruBaseBuf *vBuf, UINT iSize )
{
	
	UINT iTotals = 0;
	UINT iTemp;
	iTemp = CGSPBuffer::SumArrayBufTotalSize(vBuf, iSize);
	iTotals = iTemp;
	iTotals += m_pBuf->m_iDataSize;
	CDynamicBuffer *pBuf = pBuf->Create(iTotals+128);
	if( !pBuf )
	{
		GS_ASSERT(0);
		return NULL;
	}
	UINT i;
	i = pBuf->TryAppend(vBuf, iSize);	
	GS_ASSERT(i==iTemp);
	i = pBuf->TryAppend(m_pBuf->m_bBuffer, m_pBuf->m_iDataSize);
	GS_ASSERT(i==m_pBuf->m_iDataSize);

	CFrameCache *pRet = new CFrameCache();	
	if( !pRet )
	{
		GS_ASSERT(0);
		pBuf->UnrefObject();
		return NULL;
	}
	pRet->m_pBuf = pBuf;
	::memcpy( &pRet->m_stFrameInfo, &m_stFrameInfo, sizeof(m_stFrameInfo));
	return pRet;
}

CFrameCache *CFrameCache::CreateMerge( const StruBaseBuf *vBuf, UINT iSize , CProFrame *pFrame)
{
	CProPacket **pPkts =NULL;
	UINT iNums;
	iNums = pFrame->GetPackets(&pPkts);
	if( iNums == 0 )
	{
		GS_ASSERT(0);
		return NULL;
	}
	UINT iTotals = 0;
	UINT iTemp;
	iTemp = CGSPBuffer::SumArrayBufTotalSize(vBuf, iSize);
	iTotals = iTemp;
	for( UINT i = 0; i<iNums; i++ )
	{
		iTotals += pPkts[i]->GetParser().iPlayloadSize;
	}

	CDynamicBuffer *pBuf = pBuf->Create(iTotals+128);

	if( !pBuf )
	{
		GS_ASSERT(0);
		return NULL;
	}
	UINT i;
	i = pBuf->TryAppend(vBuf, iSize);
	GS_ASSERT(i==iTemp);
	
	for(  i = 0; i<iNums; i++ )
	{
		iTemp = pBuf->TryAppend( (const BYTE *)pPkts[i]->GetParser().bPlayload, pPkts[i]->GetParser().iPlayloadSize);
		GS_ASSERT(iTemp != pPkts[i]->GetParser().iPlayloadSize);
	}	
	CFrameCache *pRet = new CFrameCache();	
	if( !pRet )
	{
		GS_ASSERT(0);
		pBuf->UnrefObject();
		return NULL;
	}
	pRet->m_pBuf = pBuf;
	return pRet;
}

CFrameCache *CFrameCache::Create(UINT iMaxSize)
{
	CFrameCache *pRet = new CFrameCache();
	if( pRet )
	{
		pRet->m_pBuf = CDynamicBuffer::Create(iMaxSize);
		if( pRet->m_pBuf )
		{
			return pRet;
		}
		pRet->UnrefObject();
		return NULL;
	}
	return NULL;

}


CFrameCache *CFrameCache::Create(CProFrame *pFrame)
{
	CProPacket **pPkts =NULL;
	UINT iNums;
	iNums = pFrame->GetPackets(&pPkts);
	if( iNums == 0 )
	{
		GS_ASSERT(0);
		return NULL;
	}
	UINT iTotals = 0;

	for( UINT i = 0; i<iNums; i++ )
	{
		iTotals += pPkts[i]->GetParser().iPlayloadSize;
	}

	CDynamicBuffer *pBuf = pBuf->Create(iTotals+128);

	if( !pBuf )
	{
		GS_ASSERT(0);
		return NULL;
	}
	UINT iTemp;
	for( UINT i = 0; i<iNums; i++ )
	{
		iTemp = pBuf->TryAppend( (const BYTE *)pPkts[i]->GetParser().bPlayload, pPkts[i]->GetParser().iPlayloadSize);
		GS_ASSERT(iTemp == pPkts[i]->GetParser().iPlayloadSize);
	}	
	CFrameCache *pRet = new CFrameCache();	
	if( !pRet )
	{
		GS_ASSERT(0);
		pBuf->UnrefObject();
		return NULL;
	}
	pRet->m_pBuf = pBuf;
	return pRet;
}

CRefObject *CFrameCache::Clone(void) const
{
	CFrameCache *pRet = new CFrameCache();
	GS_ASSERT(pRet);
	if( pRet )
	{
		if( m_pBuf )
		{
			pRet->m_pBuf = dynamic_cast<CDynamicBuffer*>(m_pBuf->Clone());
			if(NULL ==  pRet->m_pBuf )
			{
				GS_ASSERT(0);
				pRet->UnrefObject();
				return NULL;
			}			
		}
		memcpy( &pRet->m_stFrameInfo, &m_stFrameInfo, sizeof(m_stFrameInfo));
	}

	return pRet;
}

/*
*********************************************************************
*
*@brief : CDynamicBuffer 实现
*
*********************************************************************
*/
CDynamicBuffer::CDynamicBuffer(BYTE *pBuf, UINT iMaxBufSize ) 
: CGSPBuffer(pBuf, iMaxBufSize)
, m_bPri(pBuf)
{

}

CDynamicBuffer::~CDynamicBuffer(void)
{
	if( m_bPri )
	{
		CMemoryPool::Free(m_bPri);
		m_bPri = NULL;
	}
}

CDynamicBuffer* CDynamicBuffer::Create( UINT iMaxBufSize )
{
	BYTE *p = (BYTE*) CMemoryPool::Malloc(iMaxBufSize);
	GS_ASSERT_RET_VAL(p, NULL);
	CDynamicBuffer *pRet = new CDynamicBuffer(p, iMaxBufSize);
	GS_ASSERT(pRet);
	if( pRet )
	{
		return pRet;
	}
	CMemoryPool::Free(p);
	return NULL;
}

CRefObject *CDynamicBuffer::Clone(void) const
{
	CDynamicBuffer *pNew = pNew->Create(m_iBufferSize);
	GS_ASSERT(pNew);
	if( pNew )
	{
		if( m_iDataSize )
		{
			pNew->SetData(m_bBuffer, m_iDataSize);
		}

	}
	return pNew;

}


/*
*********************************************************************
*
*@brief :  CProFrame 实现
*
*********************************************************************
*/

CProFrame::CProFrame(void)
: CRefObject()	
,m_ppPkts(NULL)
,m_iPktNumber(0)
,m_iPktMax(0)
{
	m_iTotalSize = 0;
}

CProFrame::~CProFrame(void)
{
	FreeData();
}

void CProFrame::FreeData(void)
{
	if( m_ppPkts )
	{

		for( UINT i = 0;i < m_iPktNumber; i++ )
		{
			m_ppPkts[i]->UnrefObject();
		}
		CMemoryPool::Free(m_ppPkts);
	}
	m_ppPkts = NULL;
	m_iPktNumber = 0;
	m_iPktMax = 0;
	m_iTotalSize = 0;
}

EnumErrno CProFrame::AppendBack(  CProPacket *pPkt )
{
	if(m_iPktMax && m_iPktNumber<m_iPktMax )
	{
		m_ppPkts[m_iPktNumber] = pPkt;
		pPkt->RefObject();
		m_iPktNumber++;

		m_iTotalSize += pPkt->GetParser().GetTotalSize();
		return eERRNO_SUCCESS;
	}

	UINT iNewMax = m_iPktMax+64;

	CProPacket **pp = (CProPacket **)CMemoryPool::Malloc(sizeof(CProPacket *)*iNewMax);

	GS_ASSERT_RET_VAL(pp, eERRNO_SYS_ENMEM);

	m_iPktMax = iNewMax;

	UINT i = 0; 
	for( ; i<m_iPktNumber; i++ )
	{
		pp[i] = m_ppPkts[i];
	}
	pp[i] = pPkt;
	pPkt->RefObject();
	m_iPktNumber++;
	m_iTotalSize += pPkt->GetParser().GetTotalSize();
	if( m_ppPkts )
	{
		CMemoryPool::Free(m_ppPkts);
	}
	m_ppPkts = pp;
	return eERRNO_SUCCESS;

}

EnumErrno CProFrame::AppendFront( CProPacket *pPkt )
{
	if(m_iPktMax && m_iPktNumber<m_iPktMax )
	{
		for( UINT i = m_iPktNumber; i>0;  i--)
		{			
			m_ppPkts[i] = m_ppPkts[i-1];
		}
		m_ppPkts[0] = pPkt;
		pPkt->RefObject();
		m_iPktNumber++;
		m_iTotalSize += pPkt->GetParser().GetTotalSize();
		return eERRNO_SUCCESS;
	}

	UINT iNewMax = m_iPktMax+64;

	CProPacket **pp = (CProPacket **)CMemoryPool::Malloc(sizeof(CProPacket *)*iNewMax);

	GS_ASSERT_RET_VAL(pp, eERRNO_SYS_ENMEM);

	m_iPktMax = iNewMax;

	UINT i = 0; 
	pp[i] = pPkt;
	pPkt->RefObject();
	i++;
	for( ; i<=m_iPktNumber; i++ )
	{
		pp[i] = m_ppPkts[i-1];
	}	
	m_iPktNumber++;
	m_iTotalSize += pPkt->GetParser().GetTotalSize();
	if( m_ppPkts )
	{
		CMemoryPool::Free(m_ppPkts);
	}
	m_ppPkts = pp;
	return eERRNO_SUCCESS;

}



CGSPBuffer *CProFrame::MergePlayload(void) const
{
	if( m_iPktNumber == 1 )
	{
		const StruPktInfo &stInfo = m_ppPkts[0]->GetParser();
		CPasteBuffer *pPasteBuf = pPasteBuf->Create(
			(CRefObject*) this, stInfo.bPlayload,
			stInfo.iPlayloadSize, 
			stInfo.iPlayloadSize );
		GS_ASSERT(pPasteBuf);
		return pPasteBuf;
	}
	else
	{


		CDynamicBuffer *pBuf = pBuf->Create(m_iTotalSize);
		GS_ASSERT_RET_VAL(pBuf, NULL);

		for( UINT  i = 0; i<m_iPktNumber; i++ )
		{
			const StruPktInfo &stInfo = m_ppPkts[i]->GetParser();
			// 		if( stInfo.iHeaderSize )
			// 		{
			// 			if( pBuf->AppendData(stInfo.bHeader, stInfo.iHeaderSize) )
			// 			{
			// 				GS_ASSERT(0);
			// 				pBuf->UnrefObject();
			// 				return NULL;
			// 			}
			// 		}
			// 		if( stInfo.iHeaderSize )
			// 		{
			// 			if( pBuf->AppendData(stInfo.bHeader, stInfo.iHeaderSize) )
			// 			{
			// 				GS_ASSERT(0);
			// 				pBuf->UnrefObject();
			// 				return NULL;
			// 			}
			// 		}
			if( stInfo.iPlayloadSize )
			{
				if( pBuf->AppendData(stInfo.bPlayload, stInfo.iPlayloadSize) )
				{
					GS_ASSERT(0);
					pBuf->UnrefObject();
					return NULL;
				}
			}
			// 		if( stInfo.iTailerSize )
			// 		{
			// 			if( pBuf->AppendData(stInfo.bTailer, stInfo.iTailerSize) )
			// 			{
			// 				GS_ASSERT(0);
			// 				pBuf->UnrefObject();
			// 				return NULL;
			// 			}
			// 		}

		}
		return pBuf;
	}
}

CRefObject *CProFrame::Clone(void) const
{
	CProFrame *pNew = new CProFrame();
	GS_ASSERT(pNew);
	if( pNew )
	{
		for( UINT i = 0; i<m_iPktNumber; i++ )
		{
			CProPacket *p = dynamic_cast<CProPacket*>(m_ppPkts[i]->Clone());
			if( p )
			{
				if( pNew->AppendBack(p) )
				{
					p->UnrefObject();
					pNew->UnrefObject();
					return NULL;
				}
			}
			else
			{
				pNew->UnrefObject();
				return NULL;					
			}
		}
	}
	return pNew;
}


/*
*********************************************************************
*
*@brief : CProFrameCache 实现
*
*********************************************************************
*/
namespace GSP
{
	class CProPacketCache : public CProPacket
	{
	private :
		CFrameCache *m_pFrame;
	public :
		static CProPacketCache *Create(const void *pData, UINT iSize)
		{
			StruBaseBuf vTemp;
			vTemp.iSize = iSize;
			vTemp.pBuffer  = (void*)pData;
			return Create(&vTemp, 1);
		}

		static CProPacketCache *Create(const StruBaseBuf *vBuf, UINT iSize)
		{
			CFrameCache *p = p->Create(vBuf, iSize);
			if( !p )
			{
				GS_ASSERT(0);
				return NULL;
			}
			CProPacketCache *pRet = new CProPacketCache();
			if( !pRet )
			{
				GS_ASSERT(0);
				p->UnrefObject();
				return NULL;

			}
			pRet->m_pFrame = p;
			pRet->m_stPktParser.bPlayload = (BYTE*) p->GetBuffer().m_bBuffer;
			pRet->m_stPktParser.iPlayloadSize = p->GetBuffer().m_iDataSize;
			return pRet;
		}
	protected :
		CProPacketCache(void) : CProPacket(), m_pFrame(NULL)
		{

		}
		virtual ~CProPacketCache(void)
		{
			SAFE_DESTROY_REFOBJECT(&m_pFrame);
		}

	};



} //end namespace GSP


EnumErrno CProFrameCache::AddBack(const void *pData, UINT iSize)
{
	StruBaseBuf vTemp;
		vTemp.iSize = iSize;
		vTemp.pBuffer = (void*)pData;
		return AddBack(&vTemp, 1);
}

EnumErrno CProFrameCache::AddBack(const StruBaseBuf *vBuf, UINT iSize )
{
	CProPacketCache *p = p->Create(vBuf, iSize);
	if( !p )
	{
		return eERRNO_SYS_ENMEM;
	}
	if( AppendBack(p) )
	{
		p->UnrefObject();
		return eERRNO_SYS_ENMEM;
	}
	p->UnrefObject();
	return eERRNO_SUCCESS;
}

EnumErrno CProFrameCache::AddFront(const void *pData, UINT iSize)
{
	StruBaseBuf vTemp;
	vTemp.iSize = iSize;
	vTemp.pBuffer = (void*)pData;
	return AddFront(&vTemp, 1);
}

EnumErrno CProFrameCache::AddFront(const StruBaseBuf *vBuf, UINT iSize )
{
	CProPacketCache *p = p->Create(vBuf, iSize);
	if( !p )
	{
		return eERRNO_SYS_ENMEM;
	}
	if( AppendFront(p) )
	{
		p->UnrefObject();
		return eERRNO_SYS_ENMEM;
	}
	p->UnrefObject();
	return eERRNO_SUCCESS;
}