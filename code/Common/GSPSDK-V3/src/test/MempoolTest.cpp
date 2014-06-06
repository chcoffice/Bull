#include "MyMemory.h"
#include "cmdline.h"

using namespace GSP;

static int _OnTestAMode(const char *czCmd,const char *args)
{

void *pA;
UINT64 iTv1, iTv2;
int i;
    iTv1 = DoGetTickCount();
    for(i=0; i<1000000; i++ )
    {
        pA = CMemPool::Malloc(1024);
        CMemPool::Free(pA);
    }
    iTv2 = DoGetTickCount();
    GSP_PRINTF("From mempool use:%d\n", iTv2-iTv1);

    iTv1 = DoGetTickCount();

    for(i=0; i<1000000; i++ )
    {
        pA = new char[1024];
        delete pA;
    }
    iTv2 = DoGetTickCount();
    GSP_PRINTF("From new use:%d\n", iTv2-iTv1);

    UINT32 iSize = 0;
    for(i=0; i<1000; i++ )
    {
        iSize += 65;
        pA = CMemPool::Malloc(iSize);
        if( pA )
        {
            memset(pA, 0, 64 );
        }
        CMemPool::Free(pA);
    }
   
    for(i=1; i<1000; i++ )
    {
        
        iSize += 1024;
        pA = CMemPool::Malloc(iSize);
        if( pA )
        {
            memset(pA, 0, 64 );
        }
        CMemPool::Free(pA);
    }
     GSP_PRINTF("Tets End, iSize=%d\n", iSize );
    return 0;
}


static StruOptionLine _Options[]=
{

    {
        "-a",
            "-amode",
            "Test mempool of a mode function.",
            _OnTestAMode
    },

    {
        NULL,
            NULL,
            NULL,
            NULL,
        }
};

int TestMemPool(const char *czCmd,const char *args)
{
    GSP_PRINTF("Test MemPool test enter..\n");

    OptionsEntery(_Options);

    GSP_PRINTF("Test MemPool test leave..\n");
    return 0;


}