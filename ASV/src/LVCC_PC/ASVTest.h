#ifndef __ASVTEST_H__
#define __ASVTEST_H__

#include "..\ASVCommandLib\asv_type.h"
#include "..\ASVCommandLib\asv_command.h"
#include "Comport.h"

#define	NUM_VPU_FREQ_TABLE	3
#define	NUM_3D_FREQ_TABLE	3
extern const unsigned int gVpuFreqTable[NUM_VPU_FREQ_TABLE];
extern const unsigned int g3DFreqTable[NUM_3D_FREQ_TABLE];

#define	MAX_FREQ_TABLE		64

typedef enum {
	ASVT_EVT_ERR = -1,
	ASVT_EVT_DONE = 0,
	ASVT_EVT_ECID,
	ASVT_EVT_REPORT_RESULT,
} ASVT_EVT_TYPE;

typedef struct {
	ASV_COMMAND		cmd;
	ASV_MODULE_ID	module;
	ASV_PARAM		param;
} ASVT_REPORT_TYPE;

typedef struct {
	int				enableCpu, enableVpu, enable3D;
	unsigned int	freqStart, freqEnd, freqStep;
	float			armTypical, armVoltStart, armVoltEnd, armVoltStep;
	float			coreTypical, coreVoltStart, coreVoltEnd, coreVoltStep;
	int				testTime;
	int				timeout;

	int				numVpuFreq;
	int				numVrFreq;
	unsigned int	vpuFreqTable[MAX_FREQ_TABLE];
	unsigned int	vrFreqTable[MAX_FREQ_TABLE];

	int				enableFixedCore, enableFixedSys;
	float			voltFixedCore, voltFixedSys;
} ASV_TEST_CONFIG;

typedef struct {
	ASV_MODULE_ID	module;
	unsigned int	frequency;
	float			lvcc;			//	volt
	int				time;			//	processing time in milli second
} ASV_EVT_DATA;


//	Fuse	MSB	Description
//	0~20	0	Lot ID
//	21~25	21	Wafer No
//	26~31	26	X_POS_H
//	0~1	32	X_POS_L
//	2~9	34	Y_POS
//	10~15	42	"000000" fix
//	16~23	48	CPU_IDS
//	24~31	56	CPU_RO
//	64~79	64	"0000000000000000" fix
//	80~95	80	"0000000000000000" fix
//	96~111	96	"0001011100100000" fix  (0x04E8)
//	112~127	112	"0010110001001000" fix  (0x1234)

typedef struct {
	unsigned int	lotID;
	unsigned int	waferNo;
	unsigned int	xPos, yPos;
	unsigned int	ids;
	unsigned int	ro;
	unsigned int	usbProductId;
	unsigned int	usbVendorId;
} CHIP_INFO;

class CASVTest{
public:
	CASVTest();
	virtual ~CASVTest();

	void RegisterCallback(
		void *pArg,															//	Argument
		void (*cbEvent)(void *pArg, ASVT_EVT_TYPE evtCode, void *evtData),	//	Event Callback
		void (*cbMsg)(void *pArg, char *str, int32_t len) );				//	Message Callback

	//
	//	Min/Max Voltage, Voltage Step
	//
	//
	void SetTestConfig( ASV_TEST_CONFIG *config, CComPort *pCom );
	bool Start( bool bChipIdMode = false );
	void Stop();
	void Scan(unsigned int ecid[4]);
	void ParseECID( unsigned int ECID[4], CHIP_INFO *chipInfo);

private:
	void *m_pcbArg;
	void (*m_cbEventFunc)(void *pArg, ASVT_EVT_TYPE evtCode, void *evtData);
	void (*m_cbMessageFunc)(void *pArg, char *str, int32_t len);
	void DbgLogPrint( int flag, char *fmt,... );

	static void FindLVCCThreadStub( void *pArg )
	{
		((CASVTest *)pArg)->FindLVCCThread();
	}
	void FindLVCCThread();
	float FastTestLoop( ASV_MODULE_ID module, unsigned int frequency );
	//
	bool	SetFrequency( ASV_MODULE_ID module, unsigned int frequency );
	bool	SetVoltage( ASV_MODULE_ID module, float voltage );
	bool	StartTest( ASV_MODULE_ID module, int retryCnt = 1 );
	float	SelectNextVoltage( float min, float current, float step );

	bool	TestLowestVolt( ASV_MODULE_ID module, unsigned int freq, float typical, float target );
	bool	TestHighestVolt( ASV_MODULE_ID module, unsigned int freq, float typical, float target );
	bool	TestTypicalVolt( ASV_MODULE_ID module, unsigned int freq, float typical );
	bool	TestTargetVolt( ASV_MODULE_ID module, unsigned int freq, float typical, float target );

	static void RxComRxCallbackStub( void *pArg, char *buf, int len )
	{
		((CASVTest *)pArg)->RxComRxCallback( buf, len );
		
	}
	void RxComRxCallback( char *buf, int len );

	//
	bool CheckCommandResponse();
	bool GetECIDCmdResponse(unsigned int ecid[4]);
	bool HardwareReset();
	bool WaitBootOnMessage();

private:
	bool			m_bConfig;
	CComPort		*m_pCom;

	ASV_TEST_CONFIG	m_TestConfig;

	ASV_MODULE_ID	m_TestModuleId;
	uint32_t		m_Frequency;
	float			m_MinVolt;
	float			m_MaxVolt;
	uint32_t		m_Timeout;

	ASV_TEST_CONFIG	m_LastConfig;

	bool			m_bThreadExit;
	LONG			m_nSem;

	bool			m_bChipIdMode;

	//	Response Event
	HANDLE			m_hRxSem;

	CRITICAL_SECTION m_CritRxMsg;

	char			m_RxMessage[4096];
	int				m_RxMsgLen;
	char			m_DbgStr[4096];

};

#endif	// __ASVTEST_H__