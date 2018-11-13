#ifndef __COMPORT_H__
#define __COMPORT_H__

#include "..\ASVCommandLib\asv_type.h"

#define MAX_BLOCK_SIZE (4096)


class CComPort{
public:
	CComPort();
	virtual ~CComPort();
	bool OpenComPort( DWORD nComPortNum );
	void SetRxCallback( void *cbArg, void (*cbFunc)(void*, char *buf, int len) )
	{
		EnterCriticalSection( &m_RxCbCritSec );
		m_RxCallbacArg = cbArg;
		m_RxCallbackFunc = cbFunc;
		LeaveCriticalSection( &m_RxCbCritSec );
	}
	void CloseComPort();

	bool SetRTS();
	bool ClearRTS();
	bool SetDTR();
	bool ClearDTR();

	void Flush()
	{
		EnterCriticalSection( &m_TxCritSec );
		if( m_hComDev )
		{
			FlushFileBuffers( m_hComDev );
			PurgeComm(m_hComDev,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		}
		LeaveCriticalSection( &m_TxCritSec );
	}

	static void DoRxTxStub(void *args);
	void DoRxTx();
	void WriteData( char *buf, int len )
	{
		EnterCriticalSection( &m_TxCritSec );
		WriteCommBlock( buf, len );
		LeaveCriticalSection( &m_TxCritSec );
	}

private:
	int ReadCommBlock(char *buf, int maxLen);
	void WriteCommBlock(char *buf, int len);

	//	Comport Information
private:
	DWORD			m_ComPortNum;		// Current Comport Number
	HANDLE			m_hComDev;
	TCHAR			m_szComPortName[12];

	void			*m_RxCallbacArg;
	void			(*m_RxCallbackFunc)(void*, char *buf, int len);

	bool			m_bIsConnected;

	//	Serial Config
	DWORD			m_BaudRate;
	OVERLAPPED		m_osWrite, m_osRead;

	bool			m_bTxEmpty;
	char			m_RxBuf[MAX_BLOCK_SIZE+1];

	//	Critical Section for Tx
	CRITICAL_SECTION m_TxCritSec;
	CRITICAL_SECTION m_RxCbCritSec;
};

BOOL GetComPortFromReg(DWORD PortNum, TCHAR *szPortName);

#endif	// __COMPORT_H__