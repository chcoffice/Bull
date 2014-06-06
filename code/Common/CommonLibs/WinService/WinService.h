#ifndef _GOSUNCN_SERVICE_H_
#define _GOSUNCN_SERVICE_H_


typedef bool (*StartServiceCallback)(void);

typedef void (*StopServiceCallback)(void);

bool RunAsService(int argc, char* argv[],StartServiceCallback pStartServiceCB,StopServiceCallback pStopServiceCB);

#endif