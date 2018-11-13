#ifndef __EBPRINTF_H__
#define __EBPRINTF_H__

#define MAX_EDIT_BUF_SIZE	(0x7FFFE) 

void EB_Printf(CEdit *pEdt, TCHAR *fmt,...);

#endif	// __EBPRINTF_H__