
#include "GSPObject.h"
#include "GSPMemory.h"

#include <time.h>
using namespace GSP;

#if defined(_DEBUG) || defined(DEBUG)
#include <map>
#include <set>
//#define DEBUG_ND
#endif



#ifdef DEBUG_ND
typedef struct _StruDebugRef
{
	std::string strClassName;
	UINT32 iRefs;
	UINT32 iKey;
	_StruDebugRef(const char *pName )
	{
		iRefs = 1;
		strClassName = pName;
	}
}StruDebugRef;

static std::map<std::string,StruDebugRef> s_csDebugRefMap;

static std::map<CGSPObject *, UINT32 > s_csDebugRefSet;
static UINT32 s_iAutoKey = 0;
static CGSMutex s_csMapMutex;
#endif

BOOL CGSPObject::g_bModuleRunning = TRUE;
CGSMutex CGSPObject::g_csMutex;


CGSPObject::CGSPObject(void)
{

#ifdef DEBUG_ND
	CGSAutoMutex locker(&s_csMapMutex);
// 	std::map<std::string,StruDebugRef>::iterator csIt;
// 	const char *pName = GSPGetClassName();
// 	UINT32 iRef = 1;
// 	csIt = s_csDebugRefMap.find( pName );
// 	if( csIt!=s_csDebugRefMap.end() )
// 	{
// 		csIt->second.iRefs++;
// 		iRef = csIt->second.iRefs; 
// 	}
// 	else
// 	{
// 		StruDebugRef stRef(pName );
// 		s_csDebugRefMap.insert(std::make_pair(stRef.strClassName, stRef) );
// 	}
// // 	MY_PRINTF("Obj '%s' New...%d %p (%d)\n", pName,
// // 		(int)time(NULL), this,iRef);

	s_csDebugRefSet.insert(std::make_pair(this, s_iAutoKey++) );
#endif
}


CGSPObject::~CGSPObject(void)
{
#ifdef DEBUG_ND
	CGSAutoMutex locker(&s_csMapMutex);
// 	std::map<std::string,StruDebugRef>::iterator csIt;
// 	const char *pName = GSPGetClassName();
// 	csIt = s_csDebugRefMap.find( pName );
// 	if( csIt!=s_csDebugRefMap.end() )
// 	{
// 		csIt->second.iRefs--;
// 		s_csDebugRefMap.erase(csIt);
// // 		MY_PRINTF("Obj '%s' Del...%d %p (%d)\n", pName,
// // 			(int)time(NULL), this,csIt->second.iRefs );
// 	}
// 	else
// 	{
// 		GSP_ASSERT(0);
// 	}
// 	
	s_csDebugRefSet.erase(this);
#endif
}

void *CGSPObject::operator new(size_t iSize)
{
	return CMemoryPool::Malloc(iSize);
}

void CGSPObject::operator delete(void *pBuffer)
{
	CMemoryPool::Free(pBuffer);
}

