#include "StdAfx.h"
#include "Comport.h"
#include "Utils.h"

BOOL GetComPortFromReg(DWORD PortNum, TCHAR *szPortName)
{
	LONG Result = ERROR_SUCCESS;
	HKEY hKey;
	TCHAR szKey[MAX_PATH];
	DWORD dwKeySize = MAX_PATH, dwClassSize = MAX_PATH, dwType;
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_ALL_ACCESS, &hKey))
	{
		MessageBox(NULL, TEXT("Cannot Get Serial Port Infomation"), TEXT("ERROR"), MB_OK);
		return FALSE;
	}
	DWORD PortCount = 0;
	while(ERROR_SUCCESS == Result){
		memset(szKey, 0, dwKeySize);
		dwKeySize = MAX_PATH;
		dwClassSize = MAX_PATH;
		Result=RegEnumValue(hKey, PortCount, szKey, &dwKeySize, NULL, &dwType, (unsigned char *)szPortName, &dwClassSize);
		if(PortNum == PortCount)
			break;
		PortCount++;
	}
	RegCloseKey(hKey);
	if(ERROR_SUCCESS == Result)
	{
//		MessageBox(NULL, szKey, szClass, MB_OK);
	}else if(ERROR_NO_MORE_ITEMS == Result)
	{
//		MessageBox(NULL, "No more Items", "Info", MB_OK);
		return FALSE;
	}else
	{
		MessageBox(NULL, TEXT("Get Serial Port Info fail"), TEXT("Info"), MB_OK);
		return FALSE;
	}
	return TRUE;
}

CComPort::CComPort()
{
	m_BaudRate = CBR_115200;
	m_RxCallbackFunc = NULL;
	m_RxCallbacArg = NULL;
	m_bTxEmpty = true;

	InitializeCriticalSection( &m_TxCritSec );
	InitializeCriticalSection( &m_RxCbCritSec );
}

CComPort::~CComPort()
{
	CloseComPort();
	DeleteCriticalSection( &m_TxCritSec );
	DeleteCriticalSection( &m_RxCbCritSec );
}

bool CComPort::OpenComPort( DWORD nComPortNum )
{
	DCB dcb;
	COMMTIMEOUTS commTimeOuts;

	//====================================================
	m_osRead.Offset=0;
	m_osRead.OffsetHigh=0;
	m_osRead.hEvent = NULL;
	m_osWrite.Offset=0;
	m_osWrite.OffsetHigh=0;
	m_osWrite.hEvent = NULL;

	m_osRead.hEvent = CreateEvent(NULL, TRUE/*bManualReset*/, FALSE, NULL);
	//manual reset event object should be used.
	//So, system can make the event objecte nonsignalled.
	//osRead.hEvent & osWrite.hEvent may be used to check the completion of
	// WriteFile() & ReadFile(). But, the DNW doesn't use this feature.
	if(m_osRead.hEvent==NULL)
	{
		//EB_Printf(TEXT("[ERROR:CreateEvent for osRead.]\n"));
		goto ErrorExit;
	}

	m_osWrite.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	if(m_osWrite.hEvent==NULL)
	{
		//EB_Printf(TEXT("[ERROR:CreateEvent for osWrite.]\n"));
		goto ErrorExit;
	}
	//====================================================

	wsprintf(m_szComPortName, TEXT("COM%d"), nComPortNum);

	m_hComDev=CreateFile(m_szComPortName,
		GENERIC_READ|GENERIC_WRITE,
		0, //exclusive access
		/*FILE_SHARE_READ|FILE_SHARE_WRITE,*/
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL);

	if(m_hComDev==INVALID_HANDLE_VALUE)
	{
		//EB_Printf(TEXT("[ERROR:CreateFile for opening COM port.]\n") );
		goto ErrorExit;
	}

	SetCommMask(m_hComDev,EV_RXCHAR);
	SetupComm(m_hComDev,4096,4096);
	PurgeComm(m_hComDev,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	commTimeOuts.ReadIntervalTimeout=0xffffffff;
	commTimeOuts.ReadTotalTimeoutMultiplier=0;
	commTimeOuts.ReadTotalTimeoutConstant=1000;
	commTimeOuts.WriteTotalTimeoutMultiplier=0;
	commTimeOuts.WriteTotalTimeoutConstant=1000;
	SetCommTimeouts(m_hComDev,&commTimeOuts);

	//====================================================
	dcb.DCBlength=sizeof(DCB);
	GetCommState(m_hComDev,&dcb);

	dcb.fBinary=TRUE;
	dcb.fParity=FALSE;
	dcb.BaudRate=m_BaudRate;
	dcb.ByteSize=8;
	dcb.Parity=0;
	dcb.StopBits=0;
#if 1
	dcb.fDtrControl=DTR_CONTROL_DISABLE;
	dcb.fRtsControl=RTS_CONTROL_DISABLE;
#else
	dcb.fDtrControl=DTR_CONTROL_ENABLE;
	dcb.fRtsControl=RTS_CONTROL_DISABLE;
#endif
	dcb.fOutxCtsFlow=0;
	dcb.fOutxDsrFlow=0;

	if(SetCommState(m_hComDev,&dcb)==TRUE)
	{
		m_bIsConnected=true;
		_beginthread( (void (*)(void *))DoRxTxStub, 0x2000, this);
		return TRUE;
	}
	else
	{
		m_bIsConnected=false;
		CloseHandle(m_hComDev);
		m_hComDev = NULL;
		return FALSE;
	}

ErrorExit:
	if( m_hComDev != NULL && m_hComDev != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hComDev );
		m_hComDev = NULL;
	}
	if( m_osRead.hEvent != NULL && m_osRead.hEvent != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_osRead.hEvent );
		m_osRead.hEvent = NULL;
	}
	if( m_osWrite.hEvent != NULL && m_osWrite.hEvent != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_osWrite.hEvent );
		m_osWrite.hEvent = NULL;
	}
	return FALSE;
}

void CComPort::CloseComPort()
{
    if(m_bIsConnected)
    {
		//	EB_Printf(TEXT("[Disconnect]\n") );
		m_bIsConnected=false;
		SetCommMask(m_hComDev,0);
		//disable event notification and wait for thread to halt
		EscapeCommFunction(m_hComDev,CLRDTR);
		PurgeComm(m_hComDev,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		CloseHandle(m_hComDev);

		CloseHandle(m_osRead.hEvent);
		CloseHandle(m_osWrite.hEvent);
	}
    Sleep(100);
}

bool CComPort::SetRTS()
{
	return EscapeCommFunction(m_hComDev,SETRTS)?true:false;
}

bool CComPort::ClearRTS()
{
	return EscapeCommFunction(m_hComDev,CLRRTS)?true:false;
}

bool CComPort::SetDTR()
{
	return EscapeCommFunction(m_hComDev,SETDTR)?true:false;
}

bool CComPort::ClearDTR()
{
	return EscapeCommFunction(m_hComDev,CLRDTR)?true:false;
}


void CComPort::DoRxTxStub(void *args)
{
	CComPort *pObj = (CComPort *)args;
	if( pObj )
	{
		pObj->DoRxTx();
	}
}

void CComPort::DoRxTx()
{
    OVERLAPPED os;
    DWORD dwEvtMask;
    int nLength;

    memset(&os,0,sizeof(OVERLAPPED));
    os.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(os.hEvent==NULL)
    {
		//EB_Printf(TEXT("[ERROR:DoRxTx os.hEvent]\n"));
		_endthread();
		return;
    }
    if(!SetCommMask(m_hComDev,EV_RXCHAR|EV_TXEMPTY))
    {
		//EB_Printf(TEXT("[ERROR:SetCommMask()]\n"));
		CloseHandle(os.hEvent);
		_endthread();
		return;
    }
    while(m_bIsConnected)
    {
		dwEvtMask=0;

		//WaitCommEvent(idComDev,&dwEvtMask,&os); //pass although no event is occurred.
		WaitCommEvent(m_hComDev,&dwEvtMask,NULL);  //wait until any event is occurred.

		if( (dwEvtMask & EV_TXEMPTY) == EV_TXEMPTY )
			m_bTxEmpty=TRUE;

		if((dwEvtMask & EV_RXCHAR) == EV_RXCHAR)
		{
			if( nLength=ReadCommBlock(m_RxBuf,MAX_BLOCK_SIZE-1) )
			{
				m_RxBuf[nLength]='\0';
				//EB_Printf(rxBuf);
				EnterCriticalSection( &m_RxCbCritSec );
				if( m_RxCallbackFunc )
				{
					m_RxCallbackFunc( m_RxCallbacArg, m_RxBuf, nLength );
				}
				LeaveCriticalSection( &m_RxCbCritSec );
			}
		}

		// Clear OVERRUN condition.
		// If OVERRUN error is occurred,the tx/rx will be locked.
		if(dwEvtMask & EV_ERR)
		{
			COMSTAT comStat;
			DWORD dwErrorFlags;
			ClearCommError(m_hComDev,&dwErrorFlags,&comStat);
			//EB_Printf(TEXT("[DBG:EV_ERR]\n"));
		}
	}
	CloseHandle(os.hEvent);
    _endthread();
}


int CComPort::ReadCommBlock(char *buf,int maxLen)
{
	BOOL fReadStat;
	COMSTAT comStat;
	DWORD dwErrorFlags;
	DWORD dwLength;

	ClearCommError(m_hComDev,&dwErrorFlags,&comStat);
	dwLength=min((DWORD)maxLen,comStat.cbInQue);
	if(dwLength>0)
	{
		fReadStat=ReadFile(m_hComDev,buf,dwLength,&dwLength,&m_osRead);
		if(!fReadStat)
		{
			//EB_Printf(TEXT("[RXERR]") );
		}
	}
	return dwLength;
}

void CComPort::WriteCommBlock(char *buf, int len)
{
    DWORD temp;
	while( len > 0 )
	{
		while(m_bTxEmpty==FALSE);
		WriteFile(m_hComDev,buf,1,&temp,&m_osWrite);
		while(m_bTxEmpty==FALSE);
		len --;
		buf ++;
	}

	FlushFileBuffers( m_hComDev );
}
