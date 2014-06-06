#include "GSCommon.h"
#include "cmdline.h"
#include "JournalIf.h"
#include "GSPObject.h"
using namespace CmdProtocolDef;
#include <time.h>
#include "JouXml.h"
#include "Log.h"

using namespace GSP;


static void   OnAsyncQueryCallback(  const StruQueryResult *pResult,  
									void *pUserContent )
{
	MY_PRINTF("\n*****************Query Result Begin****************\n\n" );

	if( pResult->eResult )
	{
		MY_PRINTF("Result fail: %s\n",  JouGetError(pResult->eResult ) );
	}
	else
	{
		MY_PRINTF("===Totals: %ld , Start: %ld , Rows:%ld===\n\n", pResult->iTotals, pResult->iRowStart, pResult->iRows );
		MY_PRINTF("%s", (const char*)pResult->pResultData );
	}
	MY_PRINTF("\n\n*****************Query Result End****************\n" );



	if( eJOU_R_SUCCESS != pResult->eResult )
	{
		return;
	}
	MY_PRINTF("\n\n*****************Test Xml ****************\n" );

	JOUXML::CJouXmlParser csParser;
	
	if( !csParser.Init( (const char *) pResult->pResultData,pResult->iResultSize) )
	{
		MY_PRINTF("XML Fail.\n");
		return;
	}
	std::vector<CGSString> vColName;
	if( !csParser.GetColName(vColName) )
	{
		MY_PRINTF("XML Get Col Name Fail.\n");
		return;	
	}	
	MY_PRINTF("\n*****************Field Name ****************\n" );
	for( UINT  i=0; i<vColName.size(); i++ )
	{
		MY_PRINTF("%s ", vColName[i].c_str() );
	}
	MY_PRINTF("\n\n***********************************\n" );
	MY_PRINTF("\n*****************Data****************\n" );
	CGSString strVal;
	for( ; !csParser.IsEof(); csParser.Next() )
	{
		for( UINT  i=0; i<vColName.size(); i++ )
		{
			if( csParser.GetValue(i, strVal ) )
			{
				MY_PRINTF("'%s' ", strVal.c_str() );
			}
			else
			{
				assert(0);
				MY_PRINTF("XML Get Col Value Fail.\n");
				break;
			}
			
		}
		MY_PRINTF("\n");
	}
	MY_PRINTF("\n\n***********************************\n" );

}

static int _OnAddQuery(const char *czCmd,const char *args)
{
StruQueryArgs czArgs;

	bzero(&czArgs, sizeof(StruQueryArgs) );
	czArgs.iRowStart = 1;
	czArgs.iPageRows = 100;

char *pFuncName = NULL;
INT iRowSart = 1;
INT iPageRow = 10;
char *pArgus = NULL;
	char *pParser = ArgsGetParser(args,NULL);
	if( pParser )
	{
		char *czKey, *czValue = NULL;
		int ret;
		while((ret=ParserFetch(&pParser, &czKey, &czValue )))
		{
			if( ret != 1 || !czKey )
			{
				return -1;
			}
		if( strcmp(czKey, "f")== 0 && czValue )
		{
			pFuncName = czValue;
			GS_SNPRINTF(czArgs.czFuncName, JOU_FUNC_NAME_LEN, pFuncName);
		}
		else if( strcmp(czKey, "args")== 0 && czValue )
		{
			pArgus = czValue;
			GS_SNPRINTF(czArgs.czArgs, JOU_ARG_LEN, pArgus);
		}
		else if( strcmp(czKey, "i")== 0 && czValue )
		{
			iRowSart = atoi(czValue);
			czArgs.iRowStart = iRowSart;
		}
		else if( strcmp(czKey, "p")== 0 && czValue )
		{
			iPageRow = atoi(czValue);
			czArgs.iPageRows = iPageRow;
		}
		else 
		{
			printf("Invalid arg: %s\n", czKey);
			return -1;
		}
		}
	}
	else
	{
		return -1;
	}
	if( !pFuncName )
	{
		printf( "Must input funcion name of query.\n");
		return -1;
	}





EnumJouErrno eRet = JouAsyncQuery(&czArgs);

		MY_PRINTF("Query: %s\n", JouGetError(eRet) );
		return 0;
}

static int _OnAddTest(const char *czCmd,const char *args)
{
StruJournalInfo stInfo;
EnumJouErrno eErrno;
time_t tv;
struct tm tmNow;

int iCnts = 1;

char *pParser = ArgsGetParser(args,NULL);
if( pParser )
{
	char *czKey, *czValue = NULL;
	int ret;
	ret=ParserFetch(&pParser, &czKey, &czValue );
	if( ret != 1 || !czKey )
	{
		return -1;
	}
	if( strcmp(czKey, "c")== 0 && czValue )
	{
		iCnts = atoi(czValue);
	}

}

UINT64 iOldTv = DoGetTickCount();

	for( int i = 0; i<iCnts; i++ )
	{

	
	bzero(&stInfo, sizeof( stInfo) );
	
	tv = time(NULL);
#ifdef _WIN32
	localtime_s(&tmNow, &tv);
#else
	localtime_r(&tmNow, &tv);
#endif
	stInfo.stTime.iYear = tmNow.tm_year+1900;

	stInfo.stTime.iMonth = tmNow.tm_mon+1;
	stInfo.stTime.iDay = tmNow.tm_mday;
	stInfo.stTime.iHour = tmNow.tm_hour;
	stInfo.stTime.iMinute= tmNow.tm_min;
	stInfo.stTime.iSecond = tmNow.tm_sec;

	UINT16 iSectionID = 0;


	stInfo.eType = eJOU_TYPE_OPERATOR;
	GS_SNPRINTF( stInfo.unLog.stOperation.czContent, 64, "²âÊÔ', %d -%d", (int)tv, i);
	stInfo.unLog.stOperation.czFailure[0] = '\0';
	GS_SNPRINTF(stInfo.unLog.stOperation.czHostName, 64, "127.0.0.1");
	stInfo.unLog.stOperation.eChnType = (i&1) ? CHANNEL_TYPE_VIDEO : CHANNEL_TYPE_OUTPUT;
	stInfo.unLog.stOperation.eClientType = CLIENT_TYPE_PU;
	stInfo.unLog.stOperation.eResult = tv&0xFF;	
	stInfo.unLog.stOperation.iCliID = 333;
	stInfo.unLog.stOperation.iCliPmsID = 20;
	stInfo.unLog.stOperation.iCmdSectionID  = iSectionID++;
	stInfo.unLog.stOperation.iDevID = i%400+1;
	stInfo.unLog.stOperation.iPmsID  = 20;
	stInfo.unLog.stOperation.iChn = 1;
	stInfo.unLog.stOperation.iCmdSectionID  = 0;
	GS_SNPRINTF(stInfo.unLog.stOperation.OperatorID, 64, "Teste,aa';''',r");
	GS_SNPRINTF(stInfo.unLog.stOperation.czFailure, 64, "´íÎó-²âÊÔ''");
	eErrno = JouAdd(&stInfo);
	if( eErrno )
	MY_PRINTF("Add operation: %s\n", JouGetError(eErrno) );

	


	bzero(&stInfo.unLog,sizeof(stInfo.unLog) );
	stInfo.eType = eJOU_TYPE_LOGIN;
	GS_SNPRINTF( stInfo.unLog.stLogin.czContent, 64, "²âÊÔ '''%d -%d", (int)tv, i);
	GS_SNPRINTF( stInfo.unLog.stLogin.czDescri, 64, "ÃèÊö'-°²È«");
	GS_SNPRINTF( stInfo.unLog.stLogin.czFailure, 64, "´íÎó''-²âÊÔ");
	GS_SNPRINTF(stInfo.unLog.stLogin.czHostName, 64, "127.0.0.1'");
	stInfo.unLog.stLogin.eResult = 1;
	GS_SNPRINTF(stInfo.unLog.stLogin.OperatorID, 64, "Tester");
	eErrno = JouAdd(&stInfo);
	if( eErrno )
	MY_PRINTF("Add Login: %s\n", JouGetError(eErrno) );


	bzero(&stInfo.unLog,sizeof(stInfo.unLog) );
	stInfo.eType = eJOU_TYPE_RUNSTATUS;
	
	GS_SNPRINTF( stInfo.unLog.stRunStatus.czContent, 64, "²âÊÔ'' %d -%d", (int)tv, i);
	GS_SNPRINTF( stInfo.unLog.stRunStatus.czDescri, 64, "²â''ÊÔ");
	stInfo.unLog.stRunStatus.eClientType =  CLIENT_TYPE_PU;
	stInfo.unLog.stRunStatus.iDevID =  999;
	stInfo.unLog.stRunStatus.iPmsID =  777;
	eErrno = JouAdd(&stInfo);
	if( eErrno )
		MY_PRINTF("Add RunStatus: %s\n", JouGetError(eErrno) );

	}

	MY_PRINTF( "Add Jou : %d Complete. %lld\n", iCnts,(long long) (DoGetTickCount()-iOldTv));
	return 0;


}

static int _OnUpdateOPTest( const char *czCmd,const char *args )
{
StruJouOperationUpdate stInfo;

INT iSectionID = 0;

char *pParser = ArgsGetParser(args,NULL);
if( pParser )
{
	char *czKey, *czValue = NULL;
	int ret;
	ret=ParserFetch(&pParser, &czKey, &czValue );
	if( ret != 1 || !czKey )
	{
		return -1;
	}
	if( strcmp(czKey, "i")== 0 && czValue )
	{
		iSectionID = atoi(czValue);
	}

}

	stInfo.iCliID = 333;
	stInfo.iCliPmsID = 20;
	stInfo.iCmdSectionID  = iSectionID;
	stInfo.eResult = 999;

	GS_SNPRINTF( stInfo.czFailure, 64, "²âÊÔ--¸üÐÂ");
	JouUpdateOperation(&stInfo);
	return 0;
}

static int _OnInitTest(const char *czCmd,const char *args)
{
	CGSString theAppPath = GSGetApplicationPath();
	theAppPath += "JouConf.ini";
	EnumJouErrno  eRet = JouModuleInit( theAppPath.c_str(),NULL);
	MY_PRINTF("Init : %s\n", JouGetError(eRet) );
	eRet = JouSetAsyncQueryCallback(OnAsyncQueryCallback, NULL);
	MY_PRINTF("SetCallback : %s\n", JouGetError(eRet) );
	return 0;

}

static int _OnUninitTest(const char *czCmd,const char *args)
{
	JouModuleUnint();
	
	MY_PRINTF("Uninit : OK\n" );
	return 0;

}

static StruOptionLine _Options[]=
{
	{
		"-a",
			"-add",
			"add jou. \n\
			-c:number   Manay row addd, defatul is 1.\n",			 
			_OnAddTest
	},
	{
		"-q",
			"-query",
			"query jou."
			" -f:funcname  query funcname.\n" 
			" -args:argus function argus. default is NULL.\n"
			" -i:startrow query start rowindex. default is 1.\n"
			" -p:pagesize query result page size. default is 10.\n",
			_OnAddQuery
		},
		{
			"-i",
				"-init",
				"Init Journal Module.\n", 
				_OnInitTest
		},
		{
			"-o",
				"-updateop",
				"Update Operation Jou. \n  -i:SectionID  default is 0.\n",				
				_OnUpdateOPTest
			},
		{
			"-u",
				"-uninit",
				"Uninit Journal Module.\n", 
				_OnUninitTest
			},
			{
				NULL,
					NULL,
					NULL,
					NULL,
			}
};

int TestJouEntry(const char *czCmd,const char *args)
{
	printf("Test jou  enter..\n");
	OptionsEntery(_Options);
	JouModuleUnint();
	printf("Test jou  leave..\n");

	return 0;


}