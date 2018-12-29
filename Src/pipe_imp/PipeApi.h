#pragma once

#include "PipeDef.h"

void  InitPipe();
void  UnInitPipe();
HLINK PipeCreate(EventCallBack eventCb);
bool  PipeListen(HLINK hLink);
bool  PipeConnnect(HLINK hlink);
void  PipeClose(HLINK hLink);
bool  PipeSend(HLINK hLink,const char* pData, unsigned int cbSize);