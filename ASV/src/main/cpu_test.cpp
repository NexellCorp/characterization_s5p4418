#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>		//	getopt & optarg

#include <sys/time.h>	//	gettimeofday

#include "asv_type.h"
#include "zlib.h"			//	zlibrary
#include "dvfsutil.h"

#define	MAX_NUM_CPU_CORE	4
#define	MAX_INPUT_SIZE		(1024*1024*1)		//	1MB Source

class CCPUTest
{
public:
	CCPUTest(){}
	virtual ~CCPUTest(){}

	static CCPUTest *m_gstInstance;

	ASV_RESULT Run();
	ASV_RESULT Stop();
	ASV_RESULT Status();

	static CCPUTest* CreateInstance(){
		CCPUTest *test = new CCPUTest();
		return test;
	}

	bool	m_bExitThread;

private:
	static void *ThreadStub(void *pArg)
	{
		CCPUTest *pObj = (CCPUTest*)pArg;
		if( pObj )
		{
			pObj->CpuTestThread( pObj->m_ThreadNum++ );
		}
		return (void*)0xDeadDead;
	}
	void CpuTestThread( int id );

	int32_t			m_ThreadNum;
	pthread_t		m_hThread[MAX_NUM_CPU_CORE];
	ASV_RESULT		m_Result[MAX_NUM_CPU_CORE];
};

extern "C" int test_cpu_cache( void );

void CCPUTest::CpuTestThread( int id )
{
	int32_t i;
	uint8_t *pInBuf = (uint8_t *)malloc(MAX_INPUT_SIZE);
	uint8_t *pZipBuf = (uint8_t *)malloc(MAX_INPUT_SIZE);
	uint8_t *pUnzipBuf = (uint8_t *)malloc(MAX_INPUT_SIZE);

	uint32_t zipSize, unzipSize;

	uint64_t start, end;

	if( !pInBuf || !pZipBuf || !pUnzipBuf )
	{
		printf("Not enought memory(id=%d)\n", id);
	}

	//	Make Input Pattern
	for( i=0; i<MAX_INPUT_SIZE; i++ )
	{
		pInBuf[i] = i%256;
	}

	while( !m_bExitThread )
	{
		start = NX_GetTickCountUs();
#if 0		
		if( id == 0)
		{
			if( 0 != test_cpu_cache() )
			{
				printf("Cache Test Failed!!\n");
				goto ErrorExit;
			}
		}
		else
#endif
		{
			zipSize = MAX_INPUT_SIZE;
			if( Z_OK != compress( pZipBuf, (uLongf*)&zipSize, pInBuf, MAX_INPUT_SIZE ) )
			{
				goto ErrorExit;
			}
			unzipSize = MAX_INPUT_SIZE;
			if( Z_OK != uncompress( pUnzipBuf, (uLongf*)&unzipSize, pZipBuf, zipSize ) )
			{
				goto ErrorExit;
			}
			// Compare
			for( i=0 ; i<MAX_INPUT_SIZE ; i++ )
			{
				if( pUnzipBuf[i] != (i%256) )
					goto ErrorExit;
			}
		}
		end = NX_GetTickCountUs();
//		printf("CPU Test Time = %lldmsec\n", (end - start)/1000 );
	}

	m_Result[id] = ASV_RES_OK;
	free(pInBuf);
	free(pZipBuf);
	free(pUnzipBuf);
	return;
ErrorExit:
	m_Result[id] = ASV_RES_ERR;
	free(pInBuf);
	free(pZipBuf);
	free(pUnzipBuf);
	return;
}

CCPUTest *m_gstInstance = NULL;

ASV_RESULT CCPUTest::Run()
{
	int32_t i;
	m_ThreadNum = 0;
	m_bExitThread = false;
	for( i=0 ; i<MAX_NUM_CPU_CORE ; i++ )
	{
		m_Result[i] = ASV_RES_TESTING;
		if( pthread_create(&m_hThread[i], NULL, CCPUTest::ThreadStub, this ) < 0 )
		{
			return ASV_RES_ERR;
		}
	}
	return ASV_RES_OK;
}

ASV_RESULT CCPUTest::Stop()
{
	int32_t i;
	m_bExitThread = true;
	for( i=0 ; i<MAX_NUM_CPU_CORE ;  i++ )
	{
		pthread_join(m_hThread[i], NULL);
	}
	return ASV_RES_OK;
}

ASV_RESULT CCPUTest::Status()
{
	int32_t i, count = 0;
	for( i=0 ; i<MAX_NUM_CPU_CORE ; i++ )
	{
		if( m_Result[i] < 0 )
			return ASV_RES_ERR;
		else if( ASV_RES_OK == m_Result[i] )
			count ++;
	}

	if(count==MAX_NUM_CPU_CORE)
		return ASV_RES_OK;
	else
		return ASV_RES_TESTING;
}


//==============================================================
//
//						CPU Test API
//
//==============================================================
static ASV_RESULT CpuRun(void)
{
	if( m_gstInstance )
	{
		return m_gstInstance->Run();
	}
	return ASV_RES_ERR;
}

static ASV_RESULT CpuStop(void)
{
	if( m_gstInstance )
	{
		return m_gstInstance->Stop();
	}
	return ASV_RES_ERR;
}

static ASV_RESULT CpuStatus(void)
{
	if( m_gstInstance )
	{
		return m_gstInstance->Status();
	}
	return ASV_RES_ERR;
}

static ASV_TEST_MODULE gstCpuTestModule =
{
	"CPU Test Module",
	CpuRun,
	CpuStop,
	CpuStatus
};

extern "C" ASV_TEST_MODULE *GetCpuTestModule(void)
{
	if( m_gstInstance == NULL )
	{
		m_gstInstance = CCPUTest::CreateInstance();
	}
	return &gstCpuTestModule;
}

#ifdef __STD_ALONE__

#define	TYPICAL_CPU_VOLT	1.1

void print_usage( const char *appName )
{
	printf("\n");
	printf("Usage : %s -f [frequency(MHz)] -v [voltage] -t [typical]\n", appName);
	printf("     -f [frequency] : Test Frequency (MHz)\n");
	printf("     -v [volatage]  : Taget Voltage (Volt)\n");
	printf("     -t [typical]   : Typical Voltage (Volt)\n");
	printf("     -c [count]     : Test Try Count (default:3)\n");
	printf("     -s [sec]       : Test Time in Second(Default 9sec)\n");
	printf("     -h             : Help\n");
}

int32_t main( int32_t argc, char *argv[] )
{
	int32_t opt, i=0;
	uint32_t freq=0, tryCount=3;
	uint32_t microVolt=0, typicalVolt = TYPICAL_CPU_VOLT;
	uint64_t testTime = 9;
	uint64_t start, end;

	GetCpuTestModule();

	while( -1 != (opt=getopt(argc, argv, "hf:v:t:s:c:")))
	{
		switch( opt )
		{
			case 'h':
				print_usage(argv[0]);
				return 0;
			case 'f':
				freq = atoi(optarg) * 1000000;
				break;
			case 'v':
				microVolt = atof(optarg)*1000000;
				break;
			case 't':
				typicalVolt = atof(optarg)*1000000;
				break;
			case 'c':
				tryCount = atoi(optarg);
				break;
			case 's':
				testTime = ((uint64_t)atoi( optarg ) * 1000000 );
				break;
			default:
				break;
		}
	}

	printf("=======================================================\n");
	printf("  Frequency  : %d MHz\n", freq/1000000 );
	printf("  Micro Volt : %d uVolt(Typical=%d uVolt)\n", microVolt, typicalVolt );
	printf("  Try Count  : %d\n", tryCount );
	printf("  Test Time  : %d sec\n", (uint32_t)(testTime/1000000) );
	printf("=======================================================\n");

	if( typicalVolt > microVolt )
	{
		SetCPUFrequency( freq );
		SetCPUVoltage( microVolt );
		usleep(100000);
	}
	else
	{
		SetCPUVoltage( microVolt );
		usleep(100000);
		SetCPUFrequency( freq );
	}

	while( tryCount > 0 )
	{
		start = NX_GetTickCountUs();
		printf(" CPU Test Count %d\n", i++ );
		CpuRun();
		while( 1 )
		{
			if( ASV_RES_ERR == CpuStatus() )
			{
				printf("\nCpu Test Failed!!!\n");
				return -1;
			}
			end = NX_GetTickCountUs();
			if( (end-start) > testTime )
			{
				printf("\nCpu Test Success!!!(%d)\n", i );
				break;
			}
			usleep( 1000 );
		}
		CpuStop();
		tryCount --;
	}

	return 0;
}

#endif	//	__STD_ALONE__
