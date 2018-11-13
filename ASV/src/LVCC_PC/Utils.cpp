#include "StdAfx.h"

void DbgMsg(TCHAR *fmt,...)
{
    TCHAR string[1024]; //margin for '\n'->'\r\n'
    va_list ap;
    va_start(ap,fmt);
    _vsntprintf(string,1024-1,fmt,ap);
    va_end(ap);

	OutputDebugString(string);
}
