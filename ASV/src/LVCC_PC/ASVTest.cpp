#include "StdAfx.h"
#include "ASVTest.h"
#include "Utils.h"

#define	RETRY_COUNT				1
#define	LVCC_RETRY_COUNT		3

#define	HARDWARE_BOOT_DELAY		10000
#define COMMAND_RESPONSE_DELAY	15000

#define	HARDWARE_RESET_TIME		2000

#define	NUM_VPU_FREQ_TABLE	3
#define	NUM_3D_FREQ_TABLE	3
const unsigned int gVpuFreqTable[NUM_VPU_FREQ_TABLE] = 
{
	500000000,
	400000000,
	330000000,
};

const unsigned int g3DFreqTable[NUM_3D_FREQ_TABLE] = 
{
	500000000,
	400000000,
	330000000,
};

CASVTest::CASVTest()
	: m_pcbArg(NULL)
	, m_cbEventFunc(NULL)
	, m_cbMessageFunc(NULL)

	, m_bConfig(false)
	, m_pCom(NULL)

	, m_TestModuleId(ASV_MODULE_ID::ASVM_CPU)
	, m_Frequency( 400000000 )
	, m_MinVolt( 0.5 )
	, m_MaxVolt( 1.5 )
	, m_Timeout( 15 )	//	sec

	, m_bChipIdMode(false)

	, m_RxMsgLen(0)
{
	m_hRxSem = CreateSemaphore( NULL, 0, 4096, NULL );
	InitializeCriticalSection( &m_CritRxMsg );
}

CASVTest::~CASVTest()
{
	CloseHandle(m_hRxSem);
	DeleteCriticalSection( &m_CritRxMsg );
}

void CASVTest::RegisterCallback( void *pArg,
					void (*cbEvent)(void *pArg, ASVT_EVT_TYPE evtCode, void *evtData),
					void (*cbMsg)(void *pArg, char *str, int32_t len) )
{
	m_pcbArg = pArg;
	m_cbEventFunc = cbEvent;
	m_cbMessageFunc = cbMsg;
}

void CASVTest::SetTestConfig( ASV_TEST_CONFIG *config, CComPort *pCom )
{
	if( config )
		m_TestConfig = *config;
	if( pCom )
	{
		m_pCom = pCom;
		m_bConfig = true;
		if( m_pCom )
			m_pCom->SetRxCallback(this, CASVTest::RxComRxCallbackStub );
	}
}

bool CASVTest::Start( bool bChipIdMode )
{
	if( !m_bConfig )
	{
		return false;
	}
	m_bChipIdMode = bChipIdMode;
	m_bThreadExit = false;
	_beginthread( (void (*)(void *))FindLVCCThreadStub, 0, this);
	return true;
}

void CASVTest::Stop()
{
	m_bThreadExit = true;
	Sleep( 30 );
}

void CASVTest::Scan(unsigned int ecid[4])
{
	if( !HardwareReset() )
	{
		if( m_cbEventFunc )
			m_cbEventFunc( m_pcbArg, ASVT_EVT_ERR, NULL );
		MessageBox(NULL, TEXT("Scan : Hardware Reset Failed!!!"), TEXT("ERROR!!!"), MB_OK );
		return;
	}

	char cmdBuf[MAX_CMD_STR];
	//	Set Target Voltage
	memset( cmdBuf, 0, sizeof(cmdBuf) );
	ASV_PARAM param = {0};
	MakeCommandString( cmdBuf, sizeof(cmdBuf), ASVC_GET_ECID, ASVM_CPU, param );

	//	Send Command
	if( m_pCom )
	{
		m_pCom->WriteData(cmdBuf, strlen(cmdBuf));
	}

	if( GetECIDCmdResponse(ecid) )
	{
		if( m_cbEventFunc )
		{
			m_cbEventFunc( m_pcbArg, ASVT_EVT_ECID, ecid );
		}
	}
}

bool CASVTest::TestLowestVolt( ASV_MODULE_ID module, unsigned int freq, float typical, float target )
{
	DbgLogPrint(1, "\n[%s] Start Voltage %f (lowest).\n", ASVModuleIDToString(module), target );
	//	여기서 System이 반드시 죽어야만 한다.
	//	만약 이번 Test가 Path가 된다면 Voltage Range가 잘못 설정된 것이다.
	if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
	{
		DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
		if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
		{
			HardwareReset();
			return false;
		}
	}
	if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
	{
		DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
		if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
		{
			HardwareReset();
			return false;
		}
	}

	//	Target 전압이 Typical 전압 보다 높으므로 
	if( target > typical )
	{
		if( !SetVoltage(module, target) )
		{
			HardwareReset();
			return true;
		}
		else
		{
			if( m_bThreadExit )
				return false;
			if( !SetFrequency(module, freq) )
			{
				HardwareReset();
			}
			else
			{
				if( !StartTest( module, RETRY_COUNT ) )
				{
					HardwareReset();
					return true;
				}
				else
				{
					DbgLogPrint(1, "[Warnning] Lowest Voltage Too High\n");
					return false;
				}
			}
		}
	}
	else
	{
		// Target 전압이 Typical전압보다 낮기 때문에 주파수를 먼저 바꾸고 전압을 바꾼다.
		if( !SetFrequency(module, freq) )
		{
			HardwareReset();
			return true;
		}
		else
		{
			if( m_bThreadExit )
				return false;
			if( !SetVoltage(module, target) )
			{
				HardwareReset();
				return true;
			}
			else
			{
				if( !StartTest( module, RETRY_COUNT ) )
				{
					HardwareReset();
					return true;
				}
				else
				{
					DbgLogPrint(1, "[Warnning] Lowest Voltage Too High\n");
					return false;
				}
			}
		}
	}
	return false;
}


//
//	여기서는 System이 반드시 살아 있어야 한다.
//	만약 여기서 Test가 실패하면 Voltage Range 설정이 잘못된 것이다.
//
bool CASVTest::TestHighestVolt( ASV_MODULE_ID module, unsigned int freq, float typical, float target )
{
	DbgLogPrint(1, "\n[%s] Start Voltage %f (highest).\n", ASVModuleIDToString(module), target );
	if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
	{
		DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
		if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
		{
			HardwareReset();
			return false;
		}
	}
	if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
	{
		DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
		if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
		{
			HardwareReset();
			return false;
		}
	}

	//	Typical 이 Target보다 높은 경우 Frequency 먼저 바꾸고 전압을 바꾼다.
	if( typical > target )
	{
		if( !SetFrequency(module, freq) || !SetVoltage(module, target) || !StartTest( module, RETRY_COUNT ) )
		{
			DbgLogPrint(1, "[Warnning] Hightest Voltage Too Low\n");
			HardwareReset();
			return false;
		}
	}
	else
	{
		//	Typical 이 Target보다 낮은 경우 전압을 먼저 바꾸고 Frequency를 바꾼다.
		if( !SetVoltage(module, target) || !SetFrequency(module, freq) || !StartTest( module, RETRY_COUNT ) )
		{
			DbgLogPrint(1, "[Warnning] Hightest Voltage Too Low\n");
			HardwareReset();
			return false;
		}
	}
	return true;
}

bool CASVTest::TestTypicalVolt( ASV_MODULE_ID module, unsigned int freq, float typical )
{
	DbgLogPrint(1, "\n[%s] Start Voltage %f (typical).\n", ASVModuleIDToString(module), typical );
	if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
	{
		DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
		if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
		{
			HardwareReset();
			return false;
		}
	}
	if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
	{
		DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
		if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
		{
			HardwareReset();
			return false;
		}
	}

	if( !SetFrequency(module, freq) || !SetVoltage(module, typical) || !StartTest( module, RETRY_COUNT ) )
	{
		DbgLogPrint(1, "[Fatal]  Typical Voltage Test Failed!!\n");
		HardwareReset();
		return false;
	}
	return true;
}


#if 1
float CASVTest::FastTestLoop( ASV_MODULE_ID module, unsigned int frequency )
{
	float lowVolt, highVolt, step, curVolt, typical, lvcc = -1;
	int tryCount = 0;
	bool lastSuccess = false;
	float prevVolt;

	if( module == ASVM_CPU )
	{
		typical  = m_TestConfig.armTypical;
		lowVolt  = m_TestConfig.armVoltStart;
		highVolt = m_TestConfig.armVoltEnd;
		step     = m_TestConfig.armVoltStep;
	}
	else
	{
		typical  = m_TestConfig.coreTypical;
		lowVolt  = m_TestConfig.coreVoltStart;
		highVolt = m_TestConfig.coreVoltEnd;
		step     = m_TestConfig.coreVoltStep;
	}

	// 
	DbgLogPrint(1, "================ Start %s %dMHz LVCC =====================\n", ASVModuleIDToStringSimple(module), frequency/1000000);

	if( !TestLowestVolt( module, frequency, typical, lowVolt ) )
		return lvcc;

	if( m_bThreadExit )
		return lvcc;

	if( !TestHighestVolt( module, frequency, typical, highVolt ) )
	{
		if( highVolt < typical || typical < lowVolt )
		{
			DbgLogPrint(1, "[%s] Typcial > Highest Voltage.\n", ASVModuleIDToString(module), curVolt );
			return lvcc;
		}

		//	전압이 너무 높을 경우 Typical을 최저 전압으로 설정해야 한다.
		if( !TestTypicalVolt( module, frequency, typical ) )
		{
			return lvcc;
		}
		else
		{
			lvcc = typical;
		}
	}
	else
	{
		lvcc = highVolt;
	}

	lastSuccess = true;	//
	prevVolt = lvcc;

	while( !m_bThreadExit )
	{
		curVolt = SelectNextVoltage( lowVolt, highVolt, step );
		DbgLogPrint(1, "\n== > SelectNextVoltage( %f, %f ) = %f\n", lowVolt, highVolt, curVolt );
		if( curVolt < 0 )
			break;

		if( lastSuccess )
		{
			prevVolt = prevVolt;
		}
		else
		{
			prevVolt = typical;
		}

		DbgLogPrint(1, "[%s] Start Voltage %f.\n", ASVModuleIDToString(module), curVolt );
		if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
		{
			DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
			if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
			{
				HardwareReset();
				return -1;
			}
		}
		if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
		{
			DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
			if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
			{
				HardwareReset();
				return -1;
			}
		}

		DbgLogPrint(1, "curVolt = %f, prevVolt = %f, lastSuccess = %d\n", curVolt, prevVolt, lastSuccess );

		lastSuccess = false;
		//	Frequency 를 먼저 바꾸고 전압을 바꿔야 함.
		if( prevVolt > curVolt )
		{
			if(!SetFrequency(module, frequency) ||  !SetVoltage(module, curVolt) || !StartTest( module, RETRY_COUNT ) )
			{
				//	Hardware가 TimeOut이 난 상태로 보이므로 H/W Reset
				HardwareReset();

				//	전압 설정/ Frequency / Test 
				//	실패하였으므로 최저 lvcc가 아닌 최전 전압은 마지막 실패 값으로 update해야함.
				lowVolt = curVolt;
			}
			else
			{
				//	테스트를 성공하였으므로 여기서는 낮은 쪽을 선택하게 된다.
				lvcc = curVolt;	//	성공하였기 때문에 lvcc를 먼저 update함
				highVolt = curVolt; // 성공하였기 때문에 최고 전압을 성공 전압으로 설정함.
				lastSuccess = true;
				prevVolt = curVolt;
				DbgLogPrint(1, "1. Success : prevVolt = %f\n", prevVolt );
			}
		}
		//	전압을 먼저 바꾸고 Frequency를 바꿔야 함.
		else
		{
			//	전압 설정
			if( !SetVoltage(module, curVolt) || !SetFrequency(module, frequency) || !StartTest( module, RETRY_COUNT ) )
			{
				//	Hardware가 TimeOut이 난 상태로 보이므로 H/W Reset
				HardwareReset();

				//	전압 설정/ Frequency / Test 
				//	실패하였으므로 최저 lvcc가 아닌 최전 전압은 마지막 실패 값으로 update해야함.
				lowVolt = curVolt;
			}
			else
			{
				//	테스트를 성공하였으므로 여기서는 낮은 쪽을 선택하게 된다.
				lvcc = curVolt;	//	성공하였기 때문에 lvcc를 먼저 update함
				highVolt = curVolt; // 성공하였기 때문에 최고 전압을 성공 전압으로 설정함.
				lastSuccess = true;
				prevVolt = curVolt;
				DbgLogPrint(1, "2. Success : prevVolt = %f\n", prevVolt );
			}
		}
	}

	if( lvcc > 0 )
	{
		if( lastSuccess )
		{
			HardwareReset();
		}

		DbgLogPrint( 1, "\nFound LVCC = %f, Check LVCC Validity\n", lvcc);
		while( !m_bThreadExit )
		{
			if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
			{
				DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
				if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
				{
					HardwareReset();
					return -1;
				}
			}
			if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
			{
				DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
				if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
				{
					HardwareReset();
					return -1;
				}
			}

			if( lvcc > typical )
			{
				if( !SetVoltage( module, lvcc ) || !SetFrequency( module, frequency ) || !StartTest( module, LVCC_RETRY_COUNT ) )
				{
					DbgLogPrint( 1, "LVCC Validity Failed(lvcc=%fvolt), Try next voltage(%fvolt)\n", lvcc, lvcc+step);
					HardwareReset();
					if( module == ASVM_CPU && lvcc >= m_TestConfig.armVoltEnd )
					{
						DbgLogPrint( 1, "===> Validation Failed !!! Exit Loop!!\n");
						lvcc = -1;
						break;
					}
					if( ((module==ASVM_VPU)||(module==ASVM_3D)) && lvcc >= m_TestConfig.coreVoltEnd )
					{
						DbgLogPrint( 1, "===> Validation Failed !!! Exit Loop!!\n");
						lvcc = -1;
						break;
					}
					lvcc += step;
				}
				else
				{
					break;
				}
			}
			else
			{
				if( !SetFrequency( module, frequency ) || !SetVoltage( module, lvcc ) || !StartTest( module, LVCC_RETRY_COUNT ) )
				{
					DbgLogPrint( 1, "LVCC Validity Failed(lvcc=%fvolt), Try next voltage(%fvolt)\n", lvcc, lvcc+step);
					HardwareReset();
					if( module == ASVM_CPU && lvcc >= m_TestConfig.armVoltEnd )
					{
						DbgLogPrint( 1, "===> Validation Failed !!! Exit Loop!!\n");
						lvcc = -1;
						break;
					}
					if( ((module==ASVM_VPU)||(module==ASVM_3D)) && lvcc >= m_TestConfig.coreVoltEnd )
					{
						DbgLogPrint( 1, "===> Validation Failed !!! Exit Loop!!\n");
						lvcc = -1;
						break;
					}
					lvcc += step;
				}
				else
				{
					break;
				}
			}
		}
	}


	DbgLogPrint(1, "================ End %s %dMHz LVCC(%f) =====================\n", ASVModuleIDToStringSimple(module), frequency/1000000, lvcc);
	return lvcc;
}
#else

float CASVTest::FastTestLoop( ASV_MODULE_ID module, unsigned int frequency )
{
	float lowVolt, highVolt, step, curVolt, lvcc = -1;
	int tryCount = 0;

	if( module == ASVM_CPU )
	{
		lowVolt  = m_TestConfig.armVoltStart;
		highVolt = m_TestConfig.armVoltEnd;
		step = m_TestConfig.armVoltStep;
	}
	else
	{
		lowVolt  = m_TestConfig.coreVoltStart;
		highVolt = m_TestConfig.coreVoltEnd;
		step = m_TestConfig.coreVoltStep;
	}

	// 
	DbgLogPrint(1, "================ Start %s %dMHz LVCC =====================\n", ASVModuleIDToStringSimple(module), frequency/1000000);
	if( !HardwareReset() )
	{
		DbgLogPrint(1, "\n!!!!!!!!!!! Hardware Reset Failed !!!!!!!!!!!!!!\n");
	}

	//	여기서 System이 반드시 죽어야만 한다.
	//	만약 이번 Test가 Path가 된다면 Voltage Range가 잘못 설정된 것이다.
	if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
	{
		DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
		if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
		{
			HardwareReset();
			return lvcc;
		}
	}
	if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
	{
		DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
		if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
		{
			HardwareReset();
			return lvcc;
		}
	}

	DbgLogPrint(1, "\n[%s] Start Voltage %f (lowest).\n", ASVModuleIDToString(module), lowVolt );
	if( !SetFrequency(module, frequency) )
	{
		HardwareReset();
	}
	else
	{
		if( m_bThreadExit )
			return lvcc;
		if(  !SetVoltage(module, lowVolt) )
		{
			HardwareReset();
		}
		else
		{
			if( !StartTest( module, RETRY_COUNT ) )
			{
				HardwareReset();
			}
			else
			{
				DbgLogPrint(1, "[Warnning] Lowest Voltage Too High\n");
				return lvcc;
			}
		}
	}

	if( m_bThreadExit )
		return lvcc;


	//
	//	여기서는 System이 반드시 살아 있어야 한다.
	//	만약 여기서 Test가 실패하면 Voltage Range 설정이 잘못된 것이다.
	//
	DbgLogPrint(1, "\n[%s] Start Voltage %f (highest).\n", ASVModuleIDToString(module), highVolt );
	if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
	{
		DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
		if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
		{
			HardwareReset();
			return lvcc;
		}
	}
	if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
	{
		DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
		if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
		{
			HardwareReset();
			return lvcc;
		}
	}

	if( !SetFrequency(module, frequency) || !SetVoltage(module, highVolt) || !StartTest( module, RETRY_COUNT ) )
	{
		DbgLogPrint(1, "[Warnning] Hightest Voltage Too Low\n");
		HardwareReset();

		if( lowVolt < 1.1 && 1.1 < highVolt )
		{
			float typical = 1.1;
			DbgLogPrint(1, "==> Try Typical Voltage.(%f volt)\n", typical );
			if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
			{
				DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
				if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
				{
					HardwareReset();
					return lvcc;
				}
			}
			if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
			{
				DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
				if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
				{
					HardwareReset();
					return lvcc;
				}
			}

			if( !SetFrequency(module, frequency) || !SetVoltage(module, typical) || !StartTest( module, RETRY_COUNT ) )
			{
				HardwareReset();
				return lvcc;
			}
			else
			{
				highVolt = typical;
			}
		}
		else
		{
			return lvcc;
		}
	}

	lvcc = highVolt;

	while( !m_bThreadExit )
	{
		curVolt = SelectNextVoltage( lowVolt, highVolt, step );
		DbgLogPrint(1, "\n== > SelectNextVoltage( %f, %f ) = %f\n", lowVolt, highVolt, curVolt );
		if( curVolt < 0 )
			break;

		DbgLogPrint(1, "[%s] Start Voltage %f.\n", ASVModuleIDToString(module), curVolt );
		if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
		{
			DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
			if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
			{
				HardwareReset();
				return lvcc;
			}
		}
		if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
		{
			DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
			if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
			{
				HardwareReset();
				return lvcc;
			}
		}

		//	전압 설정
		if( !SetFrequency(module, frequency) || !SetVoltage(module, curVolt) || !StartTest( module, RETRY_COUNT ) )
		{
			//	Hardware가 TimeOut이 난 상태로 보이므로 H/W Reset
			HardwareReset();

			//	전압 설정/ Frequency / Test 
			//	실패하였으므로 최저 lvcc가 아닌 최전 전압은 마지막 실패 값으로 update해야함.
			lowVolt = curVolt;
		}
		else
		{
			//	테스트를 성공하였으므로 여기서는 낮은 쪽을 선택하게 된다.
			lvcc = curVolt;	//	성공하였기 때문에 lvcc를 먼저 update함
			highVolt = curVolt; // 성공하였기 때문에 최고 전압을 성공 전압으로 설정함.
		}
	}

	if( lvcc > 0 )
	{
		DbgLogPrint( 1, "\nFound LVCC = %f, Check LVCC Validity\n", lvcc);
		while( !m_bThreadExit )
		{
			//	여기서 System이 반드시 죽어야만 한다.
			//	만약 이번 Test가 Path가 된다면 Voltage Range가 잘못 설정된 것이다.
			if( module == ASVM_CPU && m_TestConfig.enableFixedCore )
			{
				DbgLogPrint(1, "Fixed Core Voltage Mode %f volt.\n", m_TestConfig.voltFixedCore );
				if( !SetVoltage(ASVM_VPU, m_TestConfig.voltFixedCore) )
				{
					HardwareReset();
					return lvcc;
				}
			}
			if( module == ASVM_CPU && m_TestConfig.enableFixedSys )
			{
				DbgLogPrint(1, "Fixed Sys Voltage Mode %f volt.\n", m_TestConfig.voltFixedSys );
				if( !SetVoltage(ASVM_LDO_SYS, m_TestConfig.voltFixedSys) )
				{
					HardwareReset();
					return lvcc;
				}
			}
			if( !SetFrequency( module, frequency ) || !SetVoltage( module, lvcc ) || !StartTest( module, LVCC_RETRY_COUNT ) )
			{
				DbgLogPrint( 1, "LVCC Validity Failed(lvcc=%fvolt), Try next voltage(%fvolt)\n", lvcc, lvcc+step);
				HardwareReset();
				if( module == ASVM_CPU && lvcc >= m_TestConfig.armVoltEnd )
				{
					DbgLogPrint( 1, "===> Validation Failed !!! Exit Loop!!\n");
					lvcc = -1;
					break;
				}
				if( ((module==ASVM_VPU)||(module==ASVM_3D)) && lvcc >= m_TestConfig.coreVoltEnd )
				{
					DbgLogPrint( 1, "===> Validation Failed !!! Exit Loop!!\n");
					lvcc = -1;
					break;
				}
				lvcc += step;
			}
			else
			{
				break;
			}
		}
	}


	DbgLogPrint(1, "================ End %s %dMHz LVCC(%f) =====================\n", ASVModuleIDToStringSimple(module), frequency/1000000, lvcc);
	return lvcc;
}

#endif

void CASVTest::FindLVCCThread()
{
	unsigned int ECID[4];
	Scan(ECID);

	if( m_bChipIdMode )
		return;

	int freqIndex = 0;
	float lvcc = -1;
	ASV_EVT_DATA evtData;
	DWORD startTick, endTick;

	//	CPU Test
	unsigned int frequency = m_TestConfig.freqEnd;

	while( m_TestConfig.enableCpu && !m_bThreadExit && (frequency >= m_TestConfig.freqStart) )
	{
		startTick = GetTickCount();
		lvcc = FastTestLoop( ASVM_CPU, frequency );
		endTick = GetTickCount();
		if( m_cbEventFunc )
		{
			evtData.module = ASVM_CPU;
			evtData.frequency = frequency;
			evtData.lvcc = lvcc;
			evtData.time = endTick - startTick;
			m_cbEventFunc( m_pcbArg, ASVT_EVT_REPORT_RESULT, &evtData );
		}
		frequency -= m_TestConfig.freqStep;
	}

#if 0
	//	VPU Test
	freqIndex = 0;
	while( m_TestConfig.enableVpu && !m_bThreadExit && freqIndex<NUM_VPU_FREQ_TABLE )
	{
		startTick = GetTickCount();
		lvcc = FastTestLoop( ASVM_VPU, gVpuFreqTable[freqIndex] );
		endTick = GetTickCount();
		if( m_cbEventFunc )
		{
			evtData.module = ASVM_VPU;
			evtData.frequency = gVpuFreqTable[freqIndex];
			evtData.lvcc = lvcc;
			evtData.time = endTick - startTick;
			m_cbEventFunc( m_pcbArg, ASVT_EVT_REPORT_RESULT, &evtData );
		}
		freqIndex ++;
	}

	//	3D Test
	freqIndex = 0;
	while( m_TestConfig.enable3D && !m_bThreadExit && freqIndex<NUM_3D_FREQ_TABLE )
	{
		startTick = GetTickCount();
		lvcc = FastTestLoop( ASVM_3D, g3DFreqTable[freqIndex] );
		endTick = GetTickCount();

		if( m_cbEventFunc )
		{
			evtData.module = ASVM_3D;
			evtData.frequency = g3DFreqTable[freqIndex];
			evtData.lvcc = lvcc;
			evtData.time = endTick - startTick;
			m_cbEventFunc( m_pcbArg, ASVT_EVT_REPORT_RESULT, &evtData );
		}
		freqIndex ++;
	}
#else
	//	VPU Test
	freqIndex = 0;
	while( m_TestConfig.enableVpu && !m_bThreadExit && m_TestConfig.numVpuFreq>freqIndex )
	{
		startTick = GetTickCount();
		lvcc = FastTestLoop( ASVM_VPU, m_TestConfig.vpuFreqTable[freqIndex] );
		endTick = GetTickCount();
		if( m_cbEventFunc )
		{
			evtData.module = ASVM_VPU;
			evtData.frequency = m_TestConfig.vpuFreqTable[freqIndex];
			evtData.lvcc = lvcc;
			evtData.time = endTick - startTick;
			m_cbEventFunc( m_pcbArg, ASVT_EVT_REPORT_RESULT, &evtData );
		}
		freqIndex ++;
	}

	//	3D Test
	freqIndex = 0;
	while( m_TestConfig.enable3D && !m_bThreadExit && m_TestConfig.numVrFreq>freqIndex )
	{
		startTick = GetTickCount();
		lvcc = FastTestLoop( ASVM_3D, m_TestConfig.vrFreqTable[freqIndex] );
		endTick = GetTickCount();

		if( m_cbEventFunc )
		{
			evtData.module = ASVM_3D;
			evtData.frequency = m_TestConfig.vrFreqTable[freqIndex];
			evtData.lvcc = lvcc;
			evtData.time = endTick - startTick;
			m_cbEventFunc( m_pcbArg, ASVT_EVT_REPORT_RESULT, &evtData );
		}
		freqIndex ++;
	}
#endif
	DbgLogPrint(1, "End Thread\n");

	if( m_cbEventFunc )
		m_cbEventFunc( m_pcbArg, ASVT_EVT_DONE, NULL );

	_endthread();
}

bool FindLineFeed( char *str, int size, int *pos )
{
	int i=0;
	for( ; i<size ; i++ )
	{
		if( str[i] == '\n' )
		{
			*pos = i+1;
			return true;
		}
	}
	return false;
}

bool CASVTest::HardwareReset()
{
	DbgLogPrint(1, "\nReset Hardware!!!\n");
	//	Reset H/W
	m_pCom->ClearDTR();
	Sleep(HARDWARE_RESET_TIME);
	m_pCom->SetDTR();
	DbgLogPrint(1, "Wait Hardware Booting Message....\n");
	if( !WaitBootOnMessage() )
	{
		return false;
	}
	return true;
}


bool CASVTest::WaitBootOnMessage()
{
	DWORD waitResult;
	int pos;
	do{
		waitResult = WaitForSingleObject( m_hRxSem, HARDWARE_BOOT_DELAY );
		if( WAIT_OBJECT_0 == waitResult )
		{
			CNXAutoLock lock(&m_CritRxMsg);
			if( FindLineFeed( m_RxMessage, m_RxMsgLen, &pos ) )
			{
				if( !strncmp( m_RxMessage, "BOOT DONE", 9 ) )
				{
					m_RxMsgLen-=pos;
					memmove( m_RxMessage, m_RxMessage + pos, m_RxMsgLen );
					return true;
				}
				m_RxMsgLen-=pos;
				memmove( m_RxMessage, m_RxMessage + pos, m_RxMsgLen );
				continue;
			}
		}
		else if( WAIT_TIMEOUT == waitResult )
		{
			if( m_cbMessageFunc )
			{
				char s[128];
				sprintf( s, "WaitBootOnMessage : Timeout");
				m_cbMessageFunc( m_pcbArg, s, strlen(s));
				m_RxMessage[m_RxMsgLen] = '\n';
				m_RxMessage[m_RxMsgLen+1] = '\0';
				m_cbMessageFunc( m_pcbArg, m_RxMessage, 0 );
			}
			return false;
		}
	}while(1);
}


bool CASVTest::CheckCommandResponse()
{
	DWORD waitResult;
	int pos;
	do{
		waitResult = WaitForSingleObject( m_hRxSem, COMMAND_RESPONSE_DELAY );
		if( WAIT_OBJECT_0 == waitResult )
		{
			CNXAutoLock lock(&m_CritRxMsg);
			if( FindLineFeed( m_RxMessage, m_RxMsgLen, &pos ) )
			{
				if( !strncmp( m_RxMessage, "SUCCESS", 7 ) )
				{
					m_RxMsgLen-=pos;
					memmove( m_RxMessage, m_RxMessage + pos, m_RxMsgLen );
					return true;
				}
				else if( !strncmp( m_RxMessage, "FAIL", 4 ) )
				{
					m_RxMsgLen-=pos;
					memmove( m_RxMessage, m_RxMessage + pos, m_RxMsgLen );
					return false;
				}
				m_RxMsgLen-=pos;
				memmove( m_RxMessage, m_RxMessage + pos, m_RxMsgLen );
				continue;
			}
		}
		else if( WAIT_TIMEOUT == waitResult )
		{
			if( m_cbMessageFunc )
			{
				char s[128];
				sprintf( s, "CheckCommandResponse : Response Timeout");
				m_cbMessageFunc( m_pcbArg, s, strlen(s));
				m_RxMessage[m_RxMsgLen] = '\n';
				m_RxMessage[m_RxMsgLen+1] = '\0';
				m_cbMessageFunc( m_pcbArg, m_RxMessage, 0 );
			}
			return false;
		}
	}while(1);
}

bool CASVTest::GetECIDCmdResponse( unsigned int ecid[4] )
{
	DWORD waitResult;
	int pos;
	do{
		waitResult = WaitForSingleObject( m_hRxSem, COMMAND_RESPONSE_DELAY );
		if( WAIT_OBJECT_0 == waitResult )
		{
			CNXAutoLock lock(&m_CritRxMsg);
			if( FindLineFeed( m_RxMessage, m_RxMsgLen, &pos ) )
			{
				if( !strncmp( m_RxMessage, "SUCCESS", 7 ) )
				{
					CHIP_INFO chipInfo;
					ParseECID( ecid, &chipInfo );
					sscanf( m_RxMessage, "SUCCESS : ECID=%x-%x-%x-%x\n", &ecid[0], &ecid[1], &ecid[2], &ecid[3] );
					m_RxMsgLen-=pos;
					memmove( m_RxMessage, m_RxMessage + pos, m_RxMsgLen );
					return true;
				}
				else if( !strncmp( m_RxMessage, "FAIL", 4 ) )
				{
					m_RxMsgLen-=pos;
					memmove( m_RxMessage, m_RxMessage + pos, m_RxMsgLen );
					return false;
				}
				m_RxMsgLen-=pos;
				memmove( m_RxMessage, m_RxMessage + pos, m_RxMsgLen );
				continue;
			}
		}
		else if( WAIT_TIMEOUT == waitResult )
		{
			if( m_cbMessageFunc )
			{
				char s[128];
				sprintf( s, "GetECIDCmdResponse : Response Timeout");
				m_cbMessageFunc( m_pcbArg, s, strlen(s));
				m_RxMessage[m_RxMsgLen] = '\n';
				m_RxMessage[m_RxMsgLen+1] = '\0';
				m_cbMessageFunc( m_pcbArg, m_RxMessage, 0 );
			}
			return false;
		}
	}while(1);
	return true;
}

bool CASVTest::SetFrequency( ASV_MODULE_ID module, unsigned int frequency )
{
	char cmdBuf[MAX_CMD_STR];
	//	Set Target Voltage
	ASV_PARAM param;
	param.u32 = frequency;
	memset( cmdBuf, 0, sizeof(cmdBuf) );
	MakeCommandString( cmdBuf, sizeof(cmdBuf), ASVC_SET_FREQ, module, param );

	//	Send Command
	if( m_pCom )
	{
		DbgLogPrint(1, "Set Frequency (%dMHz)\n", frequency/1000000 );
		m_pCom->WriteData(cmdBuf, strlen(cmdBuf));
	}

	return CheckCommandResponse();
}

bool CASVTest::SetVoltage( ASV_MODULE_ID module, float voltage )
{
	char cmdBuf[MAX_CMD_STR];
	//	Set Target Voltage
	ASV_PARAM param;
	bool ret;
	param.f32 = voltage;
	memset( cmdBuf, 0, sizeof(cmdBuf) );
	MakeCommandString( cmdBuf, sizeof(cmdBuf), ASVC_SET_VOLT, module, param );

	//	Send Command
	if( m_pCom )
	{
		DbgLogPrint(1, "Set Voltage (%f volt)\n", voltage );
		m_pCom->WriteData(cmdBuf, strlen(cmdBuf));
	}
	ret = CheckCommandResponse();

	Sleep( 100 );
	return ret;
}

bool CASVTest::StartTest( ASV_MODULE_ID module, int retryCnt )
{
	int i=0;
	do{
		DbgLogPrint(1, "Start %s Module Test( Try Count = %d )\n", ASVModuleIDToStringSimple(module), ++i );
		char cmdBuf[MAX_CMD_STR];
		ASV_PARAM param;
		memset( cmdBuf, 0, sizeof(cmdBuf) );
		param.u64 = 0;
		MakeCommandString( cmdBuf, sizeof(cmdBuf), ASVC_RUN, module, param );
		//	Send Command
		if( m_pCom )
		{
			m_pCom->WriteData(cmdBuf, strlen(cmdBuf));
		}
		if( true != CheckCommandResponse() )
		{
			return false;
		}
		retryCnt --;
	}while( retryCnt > 0 && !m_bThreadExit );

	return true;
}


//
//	Low / High
//
float CASVTest::SelectNextVoltage( float low, float high, float step )
{
	int numStep;

	if( low == high || ((high-low)<(step*1.5)) )
		return -1;

	numStep = (int)((high - low)/step);
	if( numStep > 3 )
	{
		numStep /= 2;
		return (high - numStep*step);
	}
	return ((high - step) > low)?high-step : -1;
}

void CASVTest::RxComRxCallback( char *buf, int len )
{
	//	Send Event
	CNXAutoLock lock(&m_CritRxMsg);
	memcpy( m_RxMessage + m_RxMsgLen, buf, len );
	m_RxMsgLen += len;
	ReleaseSemaphore( m_hRxSem, 2, &m_nSem);
	DbgMsg( TEXT("m_nSem = %d\n"), m_nSem );
	if( m_cbMessageFunc )
	{
		if( len > 0 )
			m_cbMessageFunc( m_pcbArg, buf, len );
	}
}

void CASVTest::DbgLogPrint( int flag, char *fmt,... )
{
	va_list ap;
	va_start(ap,fmt);
	vsnprintf( m_DbgStr, 4095, fmt, ap );
	va_end(ap);
	if( m_cbMessageFunc )
	{
		m_cbMessageFunc( m_pcbArg, m_DbgStr, 0 );
	}
}

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

//	96~111	96	"0001011100100000" fix
//	112~127	112	"0010110001001000" fix

unsigned int ConvertMSBLSB( unsigned int data, int bits )
{
	unsigned int result = 0;
	unsigned int mask = 1;

	int i=0;
	for( i=0; i<bits ; i++ )
	{
		if( data&(1<<i) )
		{
			result |= mask<<(bits-i-1);
		}
	}
	return result;
}

void CASVTest::ParseECID( unsigned int ECID[4], CHIP_INFO *chipInfo)
{
	//	Read GUID
	chipInfo->lotID			= ConvertMSBLSB( ECID[0] & 0x1FFFFF, 21 );
	chipInfo->waferNo		= ConvertMSBLSB( (ECID[0]>>21) & 0x1F, 5 );
	chipInfo->xPos			= ConvertMSBLSB( ((ECID[0]>>26) & 0x3F) | ((ECID[1]&0x3)<<6), 8 );
	chipInfo->yPos			= ConvertMSBLSB( (ECID[1]>>2) & 0xFF, 8 );
	chipInfo->ids			= ConvertMSBLSB( (ECID[1]>>16) & 0xFF, 8 );
	chipInfo->ro				= ConvertMSBLSB( (ECID[1]>>24) & 0xFF, 8 );
	chipInfo->usbProductId	= ECID[3] & 0xFFFF;
	chipInfo->usbVendorId	= (ECID[3]>>16) & 0xFFFF;
}
