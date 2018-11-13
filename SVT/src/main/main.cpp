#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>

#include "ecid.h"


extern int read_env(char  *str, char *rbuf);

//////////////////////////////////////////////////////////////////////////////
//
//			Define Grobal Variables & Types
//
#define	MAX_ARG		16
#define	MAX_STR		256

typedef struct TEST_ITEM_INFO{
	int			status;		//	0 : OK, 1 : Not tested, -1 : Failed
	const char	appName[512];
	const char	descName[512];
	const char	args[512];
} TEST_ITEM_INFO;


//
//	Test Item Status
//
const char *gstStatusStr[32] = {
	"FAILED",
	"OK",
	"NOT Tested"
};

//
//	Test List
//
static TEST_ITEM_INFO gTestItems[] =
{

//
//                 1         2         3         4         5         6         7         8         9
//        123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
//        ===============================================================================
//        | Application           | Description       | Options                         |
	{ 1, "/root/cpuinfo_test",      "CPU Core Test   ",	"-n 4",						      },
	{ 1, "/root/adc_test",          "ADC Test        ",	"-m VTK -t 10 -n 50 -v 1.2 -c 3"  },
	{ 1, "/root/sfr_test",          "GMAC Test       ",	"-a 0xc0060000 -w 8 -m 8 -t",     },
	{ 1, "/root/i2c_test",          "I2C CH 0 Test   ", "-p 0 -m 2 -a 34 -r 7c -v 0"      },
	{ 1, "/root/i2c_test",          "I2C CH 1 Test   ", "-p 1 -m 1 -a 70 -r 0 -v 0"       },
	{ 1, "/root/i2c_test",          "I2C CH 2 Test   ", "-p 2 -m 1 -a 64 -r 0 -v 0",      },
	{ 1, "/root/i2s_test",          "I2S CH 0 <-> 2  ", "-p 0 -l 2 -T 3",                 },
	{ 1, "/root/i2s_test",          "I2S CH 1        ", "-p 1 -T 3",                      },
	{ 1, "/root/sfr_test",          "AC97 Test       ", "-a c0058008 -w ffff -m ffff -t", },
	{ 1, "/root/spdif_test",        "SPDIF TX <-> RX ", "-l",                             },
	{ 1, "/root/mmc",               "MMC CH 1 Test   ",	"-d /dev/mmcblk1p1 -p /mnt/mmc"   },
	{ 1, "/root/mmc",               "MMC CH 2 Test   ",	"-d /dev/mmcblk2p1 -p /mnt/mmc"   },
	{ 1, "/root/nand_test",         "NAND Test       ",	"",							      },
	{ 1, "/root/pwm_ppm",           "PWM/PPM Test    ",	"-p 2 -r 100 -d 50",	          },
	{ 1, "/root/sfr_test",          "PWM CH 3 Test   ", "-a c0018030 -w ffff -m ffff -t", },
	{ 1, "/root/sfr_test",          "PDM Test        ", "-a c0014014 -w ffff -m ffff -t", },
	{ 1, "/root/spi_test",          "SPI CH 0 Test   ",	"-a 0x100000",				      },
	{ 1, "/root/sfr_test",          "SPI CH 1 Test   ", "-a c005c010 -w fe -m fe -t",     },
	{ 1, "/root/sfr_test",          "SPI CH 2 Test   ", "-a c005f010 -w fe -m fe -t",     },
	{ 1, "/root/uart_test",         "UART CH 1 <-> 4 ", "-p /dev/ttyAMA1 -d /dev/ttyAMA4" },
	{ 1, "/root/uart_test",         "UART CH 2 <-> 3 ", "-p /dev/ttyAMA2 -d /dev/ttyAMA3" },
	{ 1, "/root/gpio_test",         "GPIO 124 Test   ", "-p 124 -d 0 -v 0",               },
	{ 1, "/root/gpio_test",         "GPIO 125 Test   ", "-p 125 -d 0 -v 1",               },
	{ 1, "/root/gpio_test",         "GPIO 126 Test   ", "-p 126 -d 0 -v 0",               },
	{ 1, "/root/gpio_test",         "GPIO 127 Test   ", "-p 127 -d 0 -v 1",               },
	{ 1, "/root/gpio_test",         "GPIO 128 Test   ", "-p 128 -d 1 -v 0",               },
	{ 1, "/root/sfr_test",          "MPEG-TS CH 0    ", "-a c005d000 -w f0 -m f0 -t",     },
	{ 1, "/root/sfr_test",          "MPEG-TS CH 1    ", "-a c005d004 -w f0 -m f0 -t",     },
	{ 1, "/root/sfr_test",          "Scaler Test     ", "-a c0066014 -w fff -m fff -t",   },
	{ 1, "/root/sfr_test",          "MIPI DSI Test   ", "-a c00d0108 -w ffff -m ffff -t", },
	{ 1, "/root/sfr_test",          "VIP 0 Test      ", "-a c0064010 -w ffff -m ffff -t", },
	{ 1, "/root/svt_mipi_csi",      "MIPI CSI Test   ", "",                               },
	{ 1, "/root/svt_hdmi",          "HDMI Test       ", "",                               },
	{ 1, "/root/sfr_test",          "Crypto Engine   ", "-a c0015000 -w 7 -m 7 -t",       },
	{ 1, "/root/svt_vpu",           "VPU Test        ",	"",                               },
	{ 1, "/root/svt_graphic",       "VR 3D Test      ",	"",                               },
	{ 1, "/root/usb_test",          "USB EHCI        ", "-d 1",                           },
	{ 1, "/root/usb_test",          "USB HSIC        ", "-d 4",                           },
	{ 1, "/root/sfr_test",          "USB OTGHOST     ", "-a c0040404 -w ffff -m ffff -t", },
	{ 1, "/root/rtc_alarm_test",    "RTC Alarm Test  ",	"-T 2",                           },
	{ 1, "/root/wdt_test",          "WatchDog Test   ",	"-T 2 -t",	                      },
	{ 1, "/root/cpu_dvfs_test",     "DVFS Test       ",	"-z -l 5 -r -b -C 4",             },
    { 1, "/root/rtc_suspend_test",  "Suspend Test    ", "-T 2 -C 5",                      },
};


//
//	Find Total Number of Test Items
//
static const int gTotalTestItems = sizeof(gTestItems) / sizeof(TEST_ITEM_INFO);



//
//	Print Command Help & Command List
//
static void PrintHelp( int32_t index, bool listOnly )
{
	int32_t i;
//                   1         2         3         4         5         6         7         8
//          12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("\n");
	printf("===============================================================================\n");
	printf("| No. | Test Items            | Description                                   |\n");
	printf("-------------------------------------------------------------------------------\n");
	for( i=0 ; i<gTotalTestItems ; i++ )
	{
		printf("| %3d | %-21s | %-45s |\n", i, gTestItems[i].appName, gTestItems[i].descName );
	}
	if(!listOnly)
	{
		printf("-------------------------------------------------------------------------------\n");
		printf("| Commands                                                                    |\n");
		printf("-------------------------------------------------------------------------------\n");
		printf("| help ( h or ? )                       : Display All Command & List          |\n");
		printf("| exit ( q )                            : Exit                                |\n");
		printf("| run [No.] [time] [count]              : Run Single Test Item                |\n");
		printf("|                                         time  : (default=none, msec)        |\n");
		printf("|                                         count : (default=1)                 |\n");
		printf("| auto                                  : Auto test(Test All IP)              |\n");
		printf("| result [No. or all]                   : View test result                    |\n");
		printf("| list(or l)                            : View all result                     |\n");

		printf("| voltage(or v) [percent]               : Change Test Voltage                 |\n");
		printf("|                                         percent : 3 or 5 or -3 or -5        |\n");

//		printf("| debug [No.] [option string]           : View all result                     |\n");
	}
	printf("===============================================================================\n");
	printf("\n");
}


//
//	Print Result
//
static void ShowResult( int32_t index )
{
	int32_t i;
	if( index == -1 )
	{
		//	Print All Help
//                       1         2         3         4         5         6         7         8
//              12345678901234567890123456789012345678901234567890123456789012345678901234567890
		printf("\n");
		printf("===============================================================================\n");
		printf("| No. | Test Items            | Description                                   |\n");
		printf("-------------------------------------------------------------------------------\n");
		for( i=0 ; i<gTotalTestItems ; i++ )
		{
			printf("| %3d | %-21s | %-45s |\n", i, gTestItems[i].appName, gTestItems[i].descName );
		}
		printf("===============================================================================\n");
		printf("\n");
		return;
	}
	else
	{
		if( index >= 0 && index < gTotalTestItems )
		{
			printf("%d : %s(%s) Status = %s\n", index, gTestItems[index].appName, gTestItems[index].descName, gstStatusStr[gTestItems[index].status+1] );
		}
	}
}

//
//	Test Single Test Item
//
int32_t TestItem( TEST_ITEM_INFO *info, uint32_t testTime, uint32_t count )
{
	int32_t ret = -1;
	char path[1024];

	printf( "========= %s start\n", info->descName );
	memset(path,0,sizeof(path));
	strcat( path, info->appName );
	strcat( path, " " );
	strcat( path, info->args );

	if( testTime != 0 )
	{
		char timeStr[16];
		sprintf(timeStr, " -T %s", testTime);
		strcat( path, timeStr );
	}
	if( count > 1 )
	{
		char cntStr[16];
		sprintf(cntStr, " -C %s", count);
		strcat( path, cntStr );
	}

	ret = system(path);

	if( 0 != ret )
	{
		info->status = -1;
	}
	else
	{
		info->status = 0;
	}

	printf( "========= %s test result %s\n", info->descName, (0==info->status)?"OK":"NOK" );
	return info->status;
}


//
//		Argument Parser
//
static int32_t GetArgument( char *pSrc, char arg[][MAX_STR] )
{
	int32_t i, j;

	// Reset all arguments
	for( i=0 ; i<MAX_ARG ; i++ )
	{
		arg[i][0] = 0;
	}

	for( i=0 ; i<MAX_ARG ; i++ )
	{
		// Remove space char
		while( *pSrc == ' ' )
			pSrc++;
		// check end of string.
		if( *pSrc == 0 || *pSrc == '\n' )
			break;

		j=0;
		while( (*pSrc != ' ') && (*pSrc != 0) && *pSrc != '\n' )
		{
			arg[i][j] = *pSrc++;
			j++;
			if( j > (MAX_STR-1) )
				j = MAX_STR-1;
		}
		arg[i][j] = 0;
	}
	return i;
}


//
//		Test Shell
//
void ShellMain( int32_t argc, const char *argv[] )
{
	static char cmdstring[MAX_ARG * (MAX_STR+1)];
	static char cmd[MAX_ARG][MAX_STR];
	int32_t number;
	int32_t cmdCnt;
	int32_t i;
	struct timeval start, end;
	PrintHelp( -1, false );
	while(1)
	{
		printf("\n cmd> ");
		memset(cmd, 0, sizeof(cmd));
		fgets( cmdstring, sizeof(cmdstring), stdin );
		cmdCnt = GetArgument( cmdstring, cmd );

		if( !strcasecmp("run", cmd[0]) || !strcasecmp("run", cmd[0]) )
		{
			uint32_t runTime=0, count=1;
			if( cmdCnt < 2 )
			{
				printf("Invalid command : Usage : run [No.]\n");
				continue;
			}
			number = atoi( cmd[1] );
			if( cmdCnt > 2 )
			{
				runTime = atoi(cmd[2]);
			}
			if( cmdCnt > 3 )
			{
				count = atoi(cmd[3]);
			}
			if( number >= gTotalTestItems || number < 0 )
			{
				printf("Invalid test number : %d\n", number);
				continue;
			}
			TestItem(&gTestItems[number], runTime, count);
			continue;
		}
		else if( !strcasecmp("auto", cmd[0] ) )
		{
			for( number=0 ; number<gTotalTestItems ; number++ )
			{
				if( 0 != TestItem(&gTestItems[number], 0, 1) )
				{
					//break;
				}
			}
		}
		else if( !strcasecmp("result", cmd[0] ) )
		{
			if( cmdCnt==1 || !strcasecmp("all", cmd[1]) )
			{
				ShowResult( -1 );
				continue;
			}
			else
			{
				number = atoi( cmd[1] );
				if( number >= gTotalTestItems || number < 0 )
				{
					printf("Invalid test number : %d\n", number);
					continue;
				}
				ShowResult( number );
				continue;
			}
		}
		else if( !strcasecmp("voltage", cmd[0] ) || !strcasecmp("volt", cmd[0] ) || !strcasecmp("v", cmd[0] ) )
		{
			int32_t percent = 0;
			if( cmdCnt > 1 )
			{
				percent = atoi(cmd[1]);
			}

			printf("Percent = %d\n", percent);
		}
		else if( !strcasecmp("list", cmd[0] ) || !strcasecmp("l", cmd[0] ) )
		{
			PrintHelp( -1, true );
		}
		// else if( !strcasecmp("debug", cmd[0] ) )
		// {
		// }
		else if( !strcasecmp("help", cmd[0] ) || !strcasecmp("h", cmd[0] ) || !strcasecmp("?", cmd[0] ) )
		{
			PrintHelp( -1, false );
		}
		else if( !strcasecmp("exit", cmd[0]) || !strcasecmp("q", cmd[0]) )
		{
			printf("\nBye bye ~~~~\n");
			return;
		}
	}
}


//
//		SVT Main Function
//
int main( int argc, const char *argv[] )
{
	int i=0;
	int error=0;
	struct timeval start, end;
	int isVolParam = 0;
	int volMargin = 0;
	static char outFileName[512];
	static char envParamBuf[512];

#if 0
	if( argc > 1 )
	{
		ShellMain( argc, argv );
	}
#endif
	if( 0 == read_env("margin", envParamBuf) )
	{
		isVolParam = 1;
		volMargin = atoi(envParamBuf);
	}

	printf("\n\n================================================\n");
	printf("    Start SVT Test(%d items)\n", gTotalTestItems);
	printf("           Margine(%d\%)\n", isVolParam?volMargin:0);
	printf("================================================\n");

	if( isVolParam )
	{
		char cmdMargin[64] = {0,};
		printf("================================================\n");
		sprintf(cmdMargin, "/root/cpu_md_vol -t -b");
		system(cmdMargin);
		printf(" Excute Margin Command : %s\n", cmdMargin);
		printf("================================================\n");
	}

	gettimeofday(&start, NULL);

	for( i=0 ; i<gTotalTestItems ; i++ )
	{
		if( 0 != TestItem(&gTestItems[i], 0, 1) )
		{
			error = 1;
			//break;
		}
	}
	gettimeofday(&end, NULL);

	printf("\n    End SVT Test\n");
	printf("================================================\n\n");


	//-----------------------------------------------------------------------------------
	uint32_t ecid[4];
	CHIPINFO_TYPE chipInfo;
	GetECID(ecid);
	ParseECID( ecid, &chipInfo );
	sprintf( outFileName, "/mnt/mmc0/TestResult_%s_%x_%dx%d_IDS%d_RO%d.txt",
		chipInfo.strLotID, chipInfo.waferNo, chipInfo.xPos, chipInfo.yPos, chipInfo.ids, chipInfo.ro );

	FILE *outFile = fopen(outFileName, "wb");
	if( NULL == outFile )
	{
		outFile = stdout;
	}

	fprintf(outFile, "================================================\n");
	fprintf(outFile, "   Chip : LotID(%s), WaferNo(%x), Pos(%dx%d), IDS(%d), RO(%d)\n",
		chipInfo.strLotID, chipInfo.waferNo, chipInfo.xPos, chipInfo.yPos, chipInfo.ids, chipInfo.ro );
	fprintf(outFile, "   Margine : %d \%\n", isVolParam?volMargin:0);
	fprintf(outFile, "================================================\n");
	fprintf(outFile, "   Test Report (%d)msec \n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - end.tv_usec)/1000);
	fprintf(outFile, "================================================\n");
	for( i=0 ; i<gTotalTestItems ; i++ )
		fprintf(outFile, "    %s        : %s(%d)\n", gTestItems[i].descName, gstStatusStr[gTestItems[i].status+1], gTestItems[i].status);
	fprintf(outFile, "================================================\n");

	if( stdout != outFile )
	{
		fclose( outFile );
		system("sync");
	}

	printf("================================================\n");
	printf("   Chip : LotID(%s), WaferNo(%x), Pos(%dx%d), IDS(%d), RO(%d)\n",
		chipInfo.strLotID, chipInfo.waferNo, chipInfo.xPos, chipInfo.yPos, chipInfo.ids, chipInfo.ro );
	printf("   Margine : %d \%\n", isVolParam?volMargin:0);
	printf("================================================\n");
	printf("   Test Report (%d)msec \n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - end.tv_usec)/1000);
	printf("================================================\n");
	for( i=0 ; i<gTotalTestItems ; i++ )
		printf("    %s        : %s(%d)\n", gTestItems[i].descName, gstStatusStr[gTestItems[i].status+1], gTestItems[i].status);
	printf("================================================\n");

	if( 0 != error )
	{
		//	Yellow
		printf("\033[43m");
		printf("\n\n\n SVT ERROR!!!");
		system("/root/fb_draw -f 0xff0000");
		printf("\033[0m\r\n");
	}
	else
	{
		//	Green
		printf("\033[42m");
		printf("\n\n\n SVT SUCCESS!!!");
		system("/root/fb_draw -f 0x00ff00");
		printf("\033[0m\r\n");
	}

	return 0;
}


