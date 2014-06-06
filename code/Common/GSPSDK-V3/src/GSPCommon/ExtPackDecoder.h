/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : EXTDECODER.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/6/25 9:06
Description: 使用厂商码流分析库的解码器
********************************************
*/

#ifndef _GS_H_EXTPACKDECODER_H_
#define _GS_H_EXTPACKDECODER_H_

#include "StreamPackDecoder.h"

namespace GSP
{


	//注册使用其他厂商分析库的解析器
	void ResigterExtPackDecoder(void);



} //end namespace GSP

#endif //end _GS_H_EXTDECODER_H_
