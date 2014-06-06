#ifndef _GOSUNCN_SIMPLELOG_H_
#define _GOSUNCN_SIMPLELOG_H_

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h> 
#include <stdio.h>
#include <string>

/************************************************************************/
/* 负责记录程序日志的简单日志类*/
/************************************************************************/


class CSimpleLog
{
public:
	CSimpleLog();
	virtual ~CSimpleLog();


	int SetLogPath(char* chPath);
	void Add(const char* fmt, ...);



private:
	enum {BUFSIZE = 3000};  //工作缓冲区
	char	m_tBuf[BUFSIZE];

	CRITICAL_SECTION  m_crit;  	//设置一个临界区
	FILE*             m_fp;
	std::string       m_logdir;
};

#endif 