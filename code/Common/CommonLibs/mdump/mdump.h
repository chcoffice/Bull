/*************************************************
  Copyright (C), 2011-2012, GOSUN 
  File name   : mdump.H      
  Author      : HongShuGuang
  Version     : V1.0.0        
  DateTime    : 2012/5/18
  Description : 程序异常崩溃可记录dump，方便调试
*************************************************/

#ifndef _GOSUNCN_MDUMP_H_
#define _GOSUNCN_MDUMP_H_


//设置UnhandledExceptionFilter,程序崩溃可生成相应dump文件，最好在程序一开始的地方调用。
//IsDumpWithFullMemory是否生成整个内存的dump文件(dump会大很多)。true生成完整dump，false生成normal的dump
//IsPreventOtherExceptionHandling如果设置成true会屏蔽其他地方对SetUnhandledExceptionFilter的调用
void MiniDump(bool IsDumpWithFullMemory = true,
			  bool IsPreventOtherExceptionHandling = true);


//IsDumpWithFullMemory是否生成整个内存的dump文件(dump会大很多)。true生成完整dump，false生成normal的dump
bool getDump(bool IsDumpWithFullMemory = true);





#endif