#include <stdio.h>
#include <string.h>
#include <string>  
#include <IServer.h>
#include "Log.h"

#include "cmdline.h"

using namespace GSP;
using namespace std;



static BOOL CmdLineParse( char *czCmdLine,char  **czCmdKey,char **czArgs )
{
    char *p;
    p = czCmdLine;
    *czCmdKey = NULL;
    *czArgs = NULL;
    if( *p!='-' )
    {
        return FALSE;
    }
    *czCmdKey = czCmdLine;
    while( *p !='\0' && *p!='\n')
    {
        if( isspace(*p) )
        {
            *p = '\0';
            p++;           
            break;
        }
        p++;
    }
    while( *p !='\0' && isspace(*p) )
    {
       p++;
       
    }

    if( *p !='\0' && *p!='\n')
    {
        *czArgs = p;
    }
    return TRUE;

}

static void PrintfHelp( StruOptionLine Options[] )
{
    MY_PRINTF("\n------------help--------------\n");
    MY_PRINTF("  -?  --?       print help.\n");
    MY_PRINTF("  -e  --exit    exit.\n");
    for( int i = 0;Options[i].czCmd!=NULL && Options[i].czShortCmd!=NULL ;i++ )
    {
        MY_PRINTF("  %s  %s   %s\n",Options[i].czCmd==NULL ? "  " : Options[i].czCmd,
            Options[i].czShortCmd==NULL ? " " : Options[i].czShortCmd,
            Options[i].czDescri==NULL ? "  " : Options[i].czDescri);
    }
    MY_PRINTF("------------------------------\n");
}

void OptionsEntery( StruOptionLine Options[])
{
    char czCmdLine[256];
    char *pCmdKey;
    char *pArgs;
    int i;

    while(1)
    {
        bzero( czCmdLine, sizeof( czCmdLine));
        czCmdLine[0] = '\0';
        gets(czCmdLine);
        if( !CmdLineParse(czCmdLine, &pCmdKey, &pArgs ) )
        {
            MY_PRINTF("ERR: Input '%s' is invalid. Input -? for help.\n", czCmdLine);
            continue;
        }
        if( strcmp(pCmdKey, "-?")==0 ||
            strcmp(pCmdKey, "--?")==0 )
        {
            PrintfHelp(Options);
            continue;
        }
        if( strcasecmp(pCmdKey, "-e")==0 ||
            strcasecmp(pCmdKey, "-exit")==0 )
        {
            return;
        }
        for( i = 0;  Options[i].czCmd!=NULL && Options[i].czShortCmd!=NULL ; i++ )
        {
            if( (Options[i].czCmd!=NULL && strcasecmp(pCmdKey, Options[i].czCmd)==0)  ||
                (Options[i].czShortCmd!=NULL && strcasecmp(pCmdKey, Options[i].czShortCmd) ==0) )
            {
                if( Options[i].fnHandle )
                {
                    if( Options[i].fnHandle(pCmdKey, pArgs ) )
                    {
                        MY_PRINTF( "Command '%s' args is invalid. Input -? for help.\n", pCmdKey);
                    }
                    i = -1;
                    break;
                }                   
            }
        }
        if( i>-1 )
        {
            //没有处理 
            MY_PRINTF("Unknow '%s' command. Input -? for help.\n", pCmdKey  );
        }
    }  //end while(1)

}

char *ArgsGetParser(const char *czArgs, char *czBuffer)
{ 
static char _TempBuffer[256];
    if( !czArgs  )
    {
        return NULL;
    }
    if( !czBuffer )
    {
        czBuffer = _TempBuffer;
    }

    bzero( czBuffer, 256);
    strncpy( czBuffer, czArgs, 255);
    return czBuffer;
}

BOOL ParserFetch(char **ppParser, char **czOptionKey, char **czOptionValue)
{
    if( !ppParser  || *ppParser==NULL)
    {
        return -1;
    }
   

char *p = NULL;

    *czOptionKey = NULL;
    *czOptionValue = NULL;

    p=*ppParser;

    while( isspace(*p))
    {
        p++;
    }
    if( *p=='\0')
    {
        return 0;
    }

    if( *p == '-' )
    {
        p++;
        *czOptionKey = p;
    }
    else
    {
      return -1;
    }
    while( *p++ !='\0' )
    {
        if( isspace(*p) )
        {
            //没有参数
            *p = '\0';
            p++;
            *ppParser = p;
            return 1;
        }
        if( *p == ':')
        {
            //有参数
            *p = '\0';
            p++;
           *czOptionValue=p;
           break;

        }
    }
    if( *czOptionValue )
    {
		
        while( isspace(*p) )
        {
            p++;
        }
		INT bArgs = 0;
		if( *p=='"' )
		{
			bArgs = 1;
			p++;
		}

        if( *p=='-' ||  *p=='\0' )
        {
            //不合法
            return -1;
        }
        *czOptionValue=p;
       while( *p++!='\0' )
       {
		   if( bArgs )
		   {
				if( *p=='"' )
				{
					bArgs  = 0;
					*p = '\0';
					p++;
					break;
				}
		   } 
		   else if( isspace(*p) )
           {
               //没有参数
               *p = '\0';
               p++;
               break;
           }
       }

	   if( bArgs )
	   {
		  MY_PRINTF("'%s' command.Argus not end <\">.\n", czOptionKey  );
		   return -1;
	   }
    }
    *ppParser = p;
    return 1;
}


typedef struct _StruThreadArgus
{
	 void* (*fnCb)(void*);
	void *pParam;
}StruThreadArgus;
#if defined(_WIN32) || defined(WIN32)

static void  __cdecl _TestThreadProc( LPVOID pParam )
{
StruThreadArgus *p = (StruThreadArgus*)pParam;
	p->fnCb(p->pParam);
	free(p);
	_endthread();
}


#else // pthread
static void* _TestThreadProc( void *pParam)
{
	StruThreadArgus *p = (StruThreadArgus*)pParam;
	p->fnCb(p->pParam);
	free(p);
	return 0;
}
#endif


BOOL TestCreateMyThread( void*(*fnCb)(void*), void *pParam )
{
StruThreadArgus *p = (StruThreadArgus*)malloc( sizeof(StruThreadArgus));
	if( !p )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	p->fnCb = fnCb;
	p->pParam = pParam;

#if defined(_WIN32) || defined(WIN32)
	uintptr_t hHandle;
	hHandle = _beginthread(_TestThreadProc,0, p);  
	if( hHandle != 1L)
	{
		return TRUE;
	}

#else  // pthread
	pthread_t tThreadID;
	if ( pthread_create(&tThreadID, 0, _TestThreadProc, p ) == 0) 
	{
		pthread_detach( tThreadID );
		return TRUE;
	}
#endif  
	free(p);
	return FALSE;

}

