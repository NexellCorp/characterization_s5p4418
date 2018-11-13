//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		: 
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "asv_type.h"
#include "dvfsutil.h"
#include "asv_command.h"

#define	ENABLE_EMUL			0
#define MAX_STRING_SIZE		128
#define	MAX_TX_STRING		1024

#define	DBG_COMMAND			1

static void send_data( int fd, const char *msg )
{
	if( fd >0 )
	{
		write( fd, msg, strlen(msg) );
	}
}

static void write_msg( int fd, char *fmt... )
{
	char str[MAX_TX_STRING];

	va_list ap;

	va_start( ap, fmt );
	vsnprintf( str, MAX_TX_STRING-1, fmt, ap );
	va_end(ap);

#if DBG_COMMAND
	printf( str );
	fflush( stdout );
#endif
	send_data( fd, str );
}


void RUN_TEST( int fd, ASV_MODULE_ID module )
{
#if (!ENABLE_EMUL)
	ASV_RESULT res;
	uint32_t count = 800;	//	10msec * 900 ==> 9 sec
	ASV_TEST_MODULE *test;
	uint64_t start, cur;

	if( ASVM_CPU == module )
		test = GetCpuTestModule();
	else if( ASVM_VPU == module )
		test = GetVpuTestModule();
	else if( ASVM_3D == module )
		test = Get3DTestModule();

	start = NX_GetTickCountUs();

	res = test->run();
	if( res < 0 )
	{
		write_msg(fd, "FAIL : ASVC_RUN %s\n", ASVModuleIDToString(module));
		return;
	}

	while( count-->0 )
	{
		usleep(10000);
		res = test->status();
		if( res < 0 )
		{
			write_msg(fd, "FAIL : ASVC_RUN %s\n", ASVModuleIDToString(module));
			test->stop();
			return;
		}
		else if( res == ASV_RES_OK )
		{
			write_msg(fd, "SUCCESS : ASVC_RUN %s\n", ASVModuleIDToString(module));
			test->stop();
			return;
		}

		cur = NX_GetTickCountUs();

		if( cur - start > 9000000 )
		{
			break;
		}

	}
	res = test->stop();
	if( res == ASV_RES_OK )
	{
		write_msg(fd, "SUCCESS : ASVC_RUN %s\n", ASVModuleIDToString(module));
		return;
	}
	write_msg(fd, "FAIL : ASVC_RUN %s\n", ASVModuleIDToString(module));
#else
	long r = random();
	if( r % 10 == 1 )
	{
		write_msg(fd, "FAIL : ASVC_RUN %s\n", ASVModuleIDToString(module));
	}
	else
	{
		write_msg(fd, "SUCCESS : ASVC_RUN %s\n", ASVModuleIDToString(module));
	}
#endif
}


int test_main(void)
{
	ASV_COMMAND asvCmd;
	ASV_MODULE_ID asvModuleID;
	ASV_PARAM asvParam;

	char cmd[MAX_STRING_SIZE];

	int fd = open("/dev/ttyAMA2", O_RDWR | O_NOCTTY);

	struct termios	newtio, oldtio;
	tcgetattr(fd, &oldtio);
	memcpy( &newtio, &oldtio, sizeof(newtio) );
	newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD | IXOFF;
	newtio.c_iflag = IGNPAR | ICRNL;
	
	newtio.c_oflag = 0;
	newtio.c_lflag = ICANON;

	printf("Default VMIN : %d, VTIME : %d\n", newtio.c_cc[VMIN], newtio.c_cc[VTIME]);

	newtio.c_cc[VMIN] = 0;
	newtio.c_cc[VTIME] = 10;

	tcflush( fd, TCIFLUSH );
	tcsetattr( fd, TCSANOW, &newtio );

	write_msg( fd, "\nBOOT DONE.\n");

	while(1)
	{
		memset(cmd, 0, sizeof(cmd));
		read( fd, cmd, sizeof(cmd));
#if DBG_COMMAND
		printf( cmd );
		fflush( stdout );
#endif
		if( ASV_RES_OK == ParseStringToCommand( cmd, sizeof(cmd), &asvCmd, &asvModuleID, &asvParam ) )
		{
			switch( asvCmd )
			{
				case ASVC_SET_FREQ:
				{
					if( ASVM_CPU == asvModuleID )
					{
						if( 0 != SetCPUFrequency(asvParam.u32) )
						{
							write_msg(fd, "FAIL : ASVC_SET_FREQ ASVM_CPU %dHz\n", asvParam.u32);
						}
						else
						{
							write_msg(fd, "SUCCESS : ASVC_SET_FREQ ASVM_CPU %dHz\n", asvParam.u32);
						}
					}
					else if( ASVM_VPU == asvModuleID )
					{
						if( 0 != SetVPUFrequency(asvParam.u32) )
						{
							write_msg(fd, "FAIL : ASVC_SET_FREQ ASVM_VPU %dHz\n", asvParam.u32);
						}
						else
						{
							write_msg(fd, "SUCCESS : ASVC_SET_FREQ ASVM_VPU %dHz\n", asvParam.u32);
						}
					}
					else if( ASVM_3D == asvModuleID )
					{
						if( 0 != Set3DFrequency(asvParam.u32) )
						{
							write_msg(fd, "FAIL : ASVC_SET_FREQ ASVM_3D %dHz\n", asvParam.u32);
						}
						else
						{
							write_msg(fd, "SUCCESS : ASVC_SET_FREQ ASVM_3D %dHz\n", asvParam.u32);
						}
					}
					else
					{
						write_msg(fd, "FAIL : Unknown Module(%d)\n", asvModuleID);
					}
					break;
				}
				case ASVC_SET_VOLT:
				{
					uint32_t microVolt = asvParam.f32 * 1000000;
					if( ASVM_CPU == asvModuleID )
					{
						if( 0 == SetCPUVoltage( microVolt ) )
						{
							write_msg(fd, "SUCCESS : ASVC_SET_VOLT ASVM_CPU %fv\n", (float)((microVolt)/1000000.));
						}
						else
						{
							write_msg(fd, "FAIL : ASVC_SET_VOLT ASVM_CPU %fv\n", (float)((microVolt)/1000000.));
						}
					}
					else if( ( ASVM_VPU == asvModuleID ) || ( ASVM_3D == asvModuleID ) )
					{
						if( 0 == SetCoreVoltage( microVolt ) )
						{
							write_msg(fd, "SUCCESS : ASVC_SET_VOLT %s %fv\n", ASVModuleIDToString(asvModuleID), (float)((microVolt)/1000000.));
						}
						else
						{
							write_msg(fd, "FAIL : ASVC_SET_VOLT %s %fv\n", ASVModuleIDToString(asvModuleID), (float)((microVolt)/1000000.));
						}
					}
					else if( ASVM_LDO_SYS == asvModuleID )
					{
						if( 0 == SetSystemLDOVoltage( microVolt ) )
						{
							write_msg(fd, "SUCCESS : ASVC_SET_VOLT %s %fv\n", ASVModuleIDToString(asvModuleID), (float)((microVolt)/1000000.));
						}
						else
						{
							write_msg(fd, "FAIL : ASVC_SET_VOLT %s %fv\n", ASVModuleIDToString(asvModuleID), (float)((microVolt)/1000000.));
						}
					}
					else
					{
						write_msg(fd, "FAIL : Unknown Module(%d)\n", asvModuleID);
					}
					break;
				}
				case ASVC_GET_ECID:
				{
					uint32_t ecid[4];
					GetECID( ecid );
					write_msg( fd, "SUCCESS : ECID=%08x-%08x-%08x-%08x\n", ecid[0], ecid[1], ecid[2], ecid[3] );
					break;
				}
				case ASVC_RUN:
				{
					RUN_TEST(fd, asvModuleID);
					break;
				}
				default:
					write_msg(fd, "FAIL : Unknown Command(%d)\n", asvCmd);
					break;
			}
		}
		else
		{
			printf("Command Parsing Error!!! : %s\n", cmd );
		}
	}

	return 0;
}

int main ( int argc, char *argv[] )
{
//	RUN_TEST( 0, ASVM_3D );
//	return 0;
	test_main();
	return 0;
}
