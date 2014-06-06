/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : CMDLINE.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/8/18 8:50
Description: 命令行处理
********************************************
*/
#ifndef GSP_CMDLINE_DEF_H
#define GSP_CMDLINE_DEF_H

//返回非0 表示参数错误
typedef int (*CommandHandleFunctionPtr)(const char *czCmd,const char *czArgs);
typedef struct _StruOptionLine
{
    char *czCmd;         //以 - 开头
    char *czShortCmd;   //以 -- 开头
    char *czDescri;
    CommandHandleFunctionPtr fnHandle;
}StruOptionLine;

//-e  或者 --exit  默认为退出 不要使用
//-？ --？ 为帮助

// 数组以czCmd==NULL && czShortCmd==NULL 为空
extern void OptionsEntery( StruOptionLine Options[]);

//args 德国格式 -key: value -key1 -key2 -ke3: val
extern char *ArgsGetParser(const char *czArgs, char *czBuffer);
// -1 表示失败， 0 表示结束， 1 表示可以继续
extern int ParserFetch(char **ppParser, char **czOptionKey, char **czOptionValue);

#ifndef strcasecmp

#ifdef WINCE
#define strcasecmp    strcmp
#elif _WIN32
#define strcasecmp    stricmp
#endif

#endif





extern BOOL TestCreateMyThread( void* (*fnCb)(void*), void *pParam );


#endif