#include "BTree.h"
#include "cmdline.h"

using namespace GSP;

static CBTree _csBtreeObject(GSPBTreeUINT32Cmp);
static UINT32 _iValue = 0;
static UINT32 _iSize = 0;

static int _OnTestRemove(const char *czCmd,const char *args)
{
    char *pParser = ArgsGetParser(args,NULL);
    if( !pParser )
    {
        return -1;
    }
    char *czKey, *czValue;
    int ret;
    ret=ParserFetch(&pParser, &czKey, &czValue );
    if( ret != 1 || !czKey )
    {
       return -1;
    }

    UINT32 iKey; 
    iKey = atoi(czKey);
    CBTree::CIterator csIt;
    csIt = _csBtreeObject.Remove((void*)iKey);
    if( csIt.Second() )
    {
        _iSize--;
        GSP_PRINTF("Remove: %d(%d,%d) OK.\n", (UINT32)iKey,csIt.Key(),(UINT32) csIt.First() );
    }
    else
    {
        GSP_PRINTF("Remove: %d Failure.\n",iKey);
    }
    return 0;
}

static int _OnTestAdd(const char *czCmd,const char *args)
{
    char *pParser = ArgsGetParser(args,NULL);
    if( !pParser )
    {
       return -1;
    }
    char *czKey, *czValue;
    int ret;
    ret=ParserFetch(&pParser, &czKey, &czValue );
    if( ret != 1 || !czValue )
    {
      return -1;
    }

    UINT32 iKey;
    UINT32 iValue;
    iKey = atoi(czKey);
    iValue = atoi(czValue);
    CBTree::CIterator csIt;
    csIt = _csBtreeObject.Insert((void*)iKey, (void*)iValue);
    if( csIt.Second() )
    {
        _iSize++;
        GSP_PRINTF("Insert: %d(%d) OK.\n", iKey,iValue);
    }
    else
    {
        GSP_PRINTF("Insert: %d(%d) Failure.\n",iKey,iValue);
    }
    return 0;
}

static int _OnTestFind(const char *czCmd,const char *args)
{
    char *pParser = ArgsGetParser(args,NULL);
    if( !pParser )
    {
        return -1;
    }
    char *czKey, *czValue;
    int ret;
    ret=ParserFetch(&pParser, &czKey, &czValue );
    if( ret != 1 || !czKey )
    {
        return -1;
    }
    UINT32 iKey; 
    iKey = atoi(czKey);

    CBTree::CIterator csIt;
    csIt = _csBtreeObject.Find((void*)iKey);
    if( csIt.Second() )
    {
        _iSize++;
        GSP_PRINTF("Find: %d(%d,%d) OK.\n", iKey,(UINT32)csIt.Key(), (UINT32)csIt.First() );
    }
    else
    {
        GSP_PRINTF("Find: %d Failure.\n",iKey);
    }
    return 0;
}

static int _OnTestTravel(const char *czCmd,const char *args)
{
    CBTree::EnumTravelMode eMode= CBTree::IN_ORDER;
    char *czMode = "in";
    char *pParser = ArgsGetParser(args,NULL);

    if( pParser )
    {
        char *czKey, *czValue;
        int ret=ParserFetch(&pParser, &czKey, &czValue );
        if( ret != 1)
        {
           return -1;
        }
        if( 0==strcmp(czKey, "in" ) )
        {
            eMode = CBTree::IN_ORDER;
        }
        else if( 0==strcmp(czKey, "pre" ) )
        {
            eMode = CBTree::PRE_ORDER;
        }
        else if( 0==strcmp(czKey, "post" ) )
        {
            eMode = CBTree::POST_ORDER;
        }
        else if( 0==strcmp(czKey, "out" ) )
        {
            eMode = CBTree::OUT_ORDER;
        }
        else
        {
            GSP_PRINTF("BTree not support %s order travel mode.\n", czKey);
            return -1;
        }
        czMode = czKey;

    }

    CBTree::CIterator csIt;
    GSP_PRINTF("\n-----------BTree travel %s order begin----------------\n", czMode);
    csIt = _csBtreeObject.Travel(eMode);
    for(int i = 0; csIt.Second(); csIt++, i++ )
    {
        GSP_PRINTF("%06x  %d(%d).\n", i, (UINT32)csIt.Key(), (UINT32)csIt.First() );
    }
    GSP_PRINTF("---------------------------------------------------------\n");
    return 0;
}

static int  _OnTestRandAdd(const char *czCmd,const char *args)
{
    int iCnts = 1;
    char *czMode = "in";
    char *pParser = ArgsGetParser(args,NULL);
    UINT32 iKey;
    if( pParser )
    {
        char *czKey, *czValue;
        int ret=ParserFetch(&pParser, &czKey, &czValue );
        if( ret != 1)
        {
           return -1;
        }
        iCnts = atoi( czKey);
    }
    CBTree::CIterator csIt;
    GSP_PRINTF("\n-----------BTree rand add  begin----------------\n");
    for(int i = 0; i<iCnts; i++ )
    {
        iKey = (UINT32)rand();
        csIt = _csBtreeObject.Insert((void*)iKey, (void*)_iValue++);
        if( csIt.Second() )
        {
            _iSize++;
            GSP_PRINTF("Insert: %d(%d) OK.\n", iKey,_iValue-1);
        }
        else
        {
            GSP_PRINTF("Insert: %d Failure.\n",iKey );
        }
    }
    GSP_PRINTF("----------------------------------------------------\n");
    return 0;

}

static int _OnTestPrintSize(const char *czCmd,const char *args)
{
    GSP_PRINTF("Local size: %d, Tree size:%d\n", _iSize, _csBtreeObject.Size());
    return 0;
}

static StruOptionLine _Options[]=
{
    {
        "-a",
            "--add",
            "Add object to btree.\n   Args:-key:value.",
            _OnTestAdd
    },
    {
        "-r",
            "--remove",
            "Remove object from btree.\n   Args:-key:value.",
            _OnTestRemove
        },
    {
        "-f",
            "--find",
            "Find object from btree.\n   Args:-key.",
            _OnTestFind
        }
        ,
        {
            "-t",
                "-travel",
                "Find object from btree.\n   Args:[-pre[-in|-post|-out]].default is in order.",
                _OnTestTravel
        }
        ,
        {
            "-ra",
                "-randadd",
                "Add rand key object to btree.\n   Args:-number .default is 1.",
                _OnTestRandAdd
        },
        {
            "-s",
                "-size",
                "Printf btree size.",
                _OnTestPrintSize
         },

        {
            NULL,
                NULL,
                NULL,
                NULL,
            }
};

int TestBtree(const char *czCmd,const char *args)
{
    GSP_PRINTF("Test CBTree enter..\n");

    OptionsEntery(_Options);

    GSP_PRINTF("Test CBTree leave..\n");
    return 0;


}
