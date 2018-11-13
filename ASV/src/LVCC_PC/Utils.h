#ifndef __UTILS_H__
#define __UTILS_H__

#include <Windows.h>

void DbgMsg(TCHAR *fmt,...);

class CNXAutoLock
{
public:
	CNXAutoLock(CRITICAL_SECTION *pCritSec)
	{
		m_pCritSec = pCritSec;
		EnterCriticalSection( m_pCritSec );
	}
	virtual ~CNXAutoLock()
	{
		LeaveCriticalSection( m_pCritSec );
	}
private:
	CRITICAL_SECTION	*m_pCritSec;
};

#endif	// __UTILS_H__