#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include "asb_protocol.h"
#include "ecid.h"

typedef struct TEST_ITEM_INFO{
	uint32_t	binNo;
	const char	appName[128];
	const char	descName[64];
	const char	args[128];
	int32_t		status;		//	0 : OK, 1 : Not tested, -1 : Failed
	uint64_t	testTime;
}TEST_ITEM_INFO;

#define	EN_CONNECT_ASB 1
#define DUMP 0
#define DUMP_LIMIT 100
const char *gstStatusStr[32] = {
	"FAILED",
	"OK",
	"NOT Tested"
};

//	BIN Number
enum {
	BIN_CPU_ID       =   8,
	BIN_USB_HOST     =   9,

	BIN_SPI          =  10,	//	SFR and Boot
	BIN_UART         =  11,
	BIN_I2S          =  12,
	BIN_I2C          =  13,
	BIN_SPDIF        =  14,
	BIN_MCU_S        =  19,	//	NAND
	BIN_ADC          =  22,
	BIN_VPU          =  24,
	BIN_DMA          =  27,	//	Tested in the I2S Test.
	BIN_TIMER        =  30,	//	Linux System.
	BIN_INTERRUPT    =  31,	//	Almost IP using interrupt.
	BIN_RTC          =  32,
	BIN_3D           =  34,

	BIN_MPEG_TS      =  38,	//	SFR
	BIN_MIPI_DSI     =  40,	//	SFR
	BIN_HDMI         =  41,	//	SFR
	BIN_SDMMC        =  47,

	BIN_CPU_ALIVE    =  56, //	Sleep/Wakeup
	BIN_MCU_A        =  61,	//	DRAM
	BIN_GPIO         =  64,

	BIN_WATCH_DOG    =  67,
	BIN_L2_CACHE     =  68,	//	All Program.
	BIN_VIP_0        =  70,	//	SFR

	BIN_ARM_DVFS_1   =  72, //	1.4 GHz
	BIN_ARM_DVFS_2   =  73, //	1.3 GHz
	BIN_ARM_DVFS_3   =  74, //	1.2 GHz
	BIN_ARM_DVFS_4   =  75, //	1.1 GHz
	BIN_ARM_DVFS_5   =  76, //	1.0 GHz
	BIN_ARM_DVFS_6   =  77, //	900 MHz
	BIN_ARM_DVFS_7   =  78, //	800 MHz
	BIN_ARM_DVFS_8   =  79, //	700 MHz
	BIN_ARM_DVFS_9   =  80, //	600 MHz
	BIN_ARM_DVFS_10  =  81, //	500 MHz
	BIN_ARM_DVFS_11  =  82, //	400 MHz

	BIN_USB_HSIC     = 126,
	BIN_VPP          = 160,	//	SFR

	// Nexell Specific Tests
	BIN_CPU_SMP      = 201,
	BIN_GMAC         = 202,
	BIN_VIP_1        = 203,	//	SFR , MIPI CSI & VIP 1
	BIN_PPM          = 204,
	BIN_PWM          = 205,	//	SFR
	BIN_PDM          = 206,	//	SFR
	BIN_AC97         = 207,	//	SFR
	BIN_OTG_HOST     = 208,
	BIN_OTG_DEVICE   = 209,	//	USB Host & USB HSIC
	BIN_MLC          = 210,	//	MLC -> DPC and MLC -> LVDS
	BIN_DPC          = 211,	//	BIN_MLC
	BIN_LVDS         = 212,	//	BIN_LVDS
	BIN_AES          = 213,	//	SFR
	BIN_CLOCK_GEN    = 214,	//	Almost IP use clock generator.
	BIN_RO_CHECK     = 215,	
};

static TEST_ITEM_INFO gTestItems[] =
{
	{ BIN_CPU_SMP    , "/root/cpuinfo_test",     "CPU Core Test   ", "-n 4",                            1, 0 },
	{ BIN_RO_CHECK    , "/root/ro_test",     	 "CPU RO Test     ", "",                            1, 0 },
	{ BIN_ADC        , "/root/adc_test",         "ADC Test        ", "-m ASB",                          1, 0 },
	{ BIN_GMAC       , "/root/gmac_test",        "GMAC Test       ", "",                                1, 0 },
	{ BIN_I2C        , "/root/i2c_test",         "I2C CH 0 Test   ", "-p 0 -a 64 -m1 -r 0 -v 0x0",      1, 0 },
	{ BIN_I2C        , "/root/i2c_test",         "I2C CH 1 Test   ", "-p 1 -a 8a -m1 -r 0 -v 0x0",      1, 0 },
	{ BIN_I2C        , "/root/i2c_test",         "I2C CH 2 Test   ", "-p 2 -a 64 -m1 -r 0 -v 0x0",      1, 0 },
	{ BIN_I2S        , "/root/i2s_test",         "I2S CH 0 <-> 1  ", "-p 0 -l 1",                       1, 0 },
	{ BIN_I2S        , "/root/i2s_test",         "I2S CH 0 <-> 2  ", "-p 0 -l 2",                       1, 0 },
	{ BIN_AC97       , "/root/sfr_test",         "AC97 Test       ", "-a c0058008 -w ffff -m ffff -t",	1, 0 },
	{ BIN_SPDIF      , "/root/spdif_test",       "SPDIF TX <-> RX ", "-l",                              1, 0 },
	{ BIN_SDMMC      , "/root/mmc",              "MMC Test        ", "",                                1, 0 },
	{ BIN_MCU_S      , "/root/nand_test",        "NAND Test       ", "",                                1, 0 },
	{ BIN_PPM        , "/root/pwm_ppm",          "PWM/PPM Test    ", "-p 0 -r 100 -d 50",             	1, 0 },
	{ BIN_PWM        , "/root/sfr_test",         "PWM CH 3 Test   ", "-a c0018030 -w ffff -m ffff -t"  ,1, 0 },
	{ BIN_PDM        , "/root/sfr_test",         "PDM Test        ", "-a c0014014 -w ffff -m ffff -t",  1, 0 },
	{ BIN_SPI        , "/root/sfr_test",         "SPI CH 1 Test   ", "-a c005c010 -w fe -m fe -t",      1, 0 },
	{ BIN_UART       , "/root/uart_test",        "UART CH 1 <-> 2 ", "-p /dev/ttyAMA1 -d /dev/ttyAMA2", 1, 0 },
	{ BIN_UART       , "/root/uart_test",        "UART CH 3 <-> 4 ", "-p /dev/ttyAMA3 -d /dev/ttyAMA4", 1, 0 },
	{ BIN_GPIO       , "/root/gpio_test",        "GPIO ALV5,A27   ", "-a 165 -b 27",                    1, 0 },
	{ BIN_GPIO       , "/root/gpio_test",        "GPIO E30,E31    ", "-a 158 -b 159",                   1, 0 },
	{ BIN_MPEG_TS    , "/root/sfr_test",         "MPEG-TS CH 0    ", "-a c005d000 -w f0 -m f0 -t", 		1, 0 },
	{ BIN_MPEG_TS    , "/root/sfr_test",         "MPEG-TS CH 1    ", "-a c005d004 -w f0 -m f0 -t", 		1, 0 },
	{ BIN_VPP        , "/root/sfr_test",         "Scaler Test     ", "-a c0066014 -w fff -m fff -t", 	1, 0 },
	{ BIN_MIPI_DSI   , "/root/sfr_test",         "MIPI DSI Test   ", "-a c00d0108 -w ffff -m ffff -t",	1, 0 },
	{ BIN_VIP_0      , "/root/sfr_test",         "VIP 0 Test      ", "-a c0064010 -w ffff -m ffff -t",	1, 0 },
	{ BIN_VIP_1      , "/root/mipi_test",        "VIP 1 Test     ", "",								    1, 0 },
	{ BIN_HDMI       , "/root/sfr_test",         "HDMI Test       ", "-a c0101014 -w ffff -m ffff -t",	1, 0 },
	{ BIN_AES        , "/root/sfr_test",         "Crypto Engine   ", "-a c0015000 -w 7 -m 7 -t", 		1, 0 },
	{ BIN_VPU        , "/root/asb_vpu",          "VPU Test        ", "",                                1, 0 },
	{ BIN_3D         , "/root/asb_graphic",      "VR 3D Test      ", "",                                1, 0 },
	{ BIN_USB_HOST   , "/root/usb_test",         "USB EHCI        ", "-d 1",                            1, 0 },
	{ BIN_USB_HSIC   , "/root/usb_test",         "USB HSIC        ", "-d 2",                            1, 0 },
	{ BIN_OTG_HOST   , "/root/usb_test",         "USB OTGHOST     ", "-d 3",                            1, 0 },
	{ BIN_RTC        , "/root/rtc_alarm_test",   "RTC Alarm Test  ", "-T 2",                            1, 0 },
	{ BIN_WATCH_DOG  , "/root/wdt_test",         "WatchDog Test   ", "-T 2 -t",                         1, 0 },
	{ BIN_MLC        , "/root/fb_pan_test",      "MLC/DPC/LVDS    ", "-d /dev/fb0",                     1, 0 },
	{ BIN_ARM_DVFS_1 , "/root/cpu_md_speed",     "DVFS Test       ", "-f  400000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_2 , "/root/cpu_md_speed",     "DVFS Test       ", "-f  500000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_3 , "/root/cpu_md_speed",     "DVFS Test       ", "-f  600000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_4 , "/root/cpu_md_speed",     "DVFS Test       ", "-f  700000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_5 , "/root/cpu_md_speed",     "DVFS Test       ", "-f  800000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_6 , "/root/cpu_md_speed",     "DVFS Test       ", "-f  900000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_7 , "/root/cpu_md_speed",     "DVFS Test       ", "-f 1000000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_8 , "/root/cpu_md_speed",     "DVFS Test       ", "-f 1100000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_9 , "/root/cpu_md_speed",     "DVFS Test       ", "-f 1200000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_10, "/root/cpu_md_speed",     "DVFS Test       ", "-f 1300000 -z -l 1 -d -75",       1, 0 },
	{ BIN_ARM_DVFS_11, "/root/cpu_md_speed",     "DVFS Test       ", "-f 1400000 -z -l 1 -d -75",       1, 0 },
	{ BIN_CPU_ALIVE  , "/root/rtc_suspend_test", "Suspend Test    ", "-T 2",                            1, 0 },
};


static const int gTotalTestItems = sizeof(gTestItems) / sizeof(TEST_ITEM_INFO);

#define	TTY_PATH 	"/dev/tty0"
#define	TTY_RED_MSG		"redirected message..."
#define	TTY_RES_MSG		"restored message..."


int stdout_redirect(int fd)
{
	int org, ret;
	if (0 > fd)
		return -EINVAL;

	org = dup(STDOUT_FILENO);
	ret = dup2(fd, STDOUT_FILENO);
	if (0 > ret) {
		printf("fail, stdout dup2 %s\n", strerror(errno));
		return -1;
	}
	return org;
}

void stdout_restore(int od_fd)
{
	dup2(od_fd, STDOUT_FILENO);
}


int TestItem( TEST_ITEM_INFO *info )
{
	int ret = -1;
	char path[1024];

	printf( "========= %s start\n", info->descName );
	memset(path,0,sizeof(path));
	strcat( path, info->appName );
	strcat( path, " " );
	strcat( path, info->args );
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


int main( int argc, char *argv[] )
{
	int32_t i=0;
	int32_t error=0;

	struct timeval start, end;
	struct timeval itemStart, itemEnd;
#if (DUMP)
	int32_t num = 0;
	int dump_fd = 0;
	int std_fd = 0;
	int32_t cnt = DUMP_LIMIT;

	static char outFileName[512];
#endif
	uint32_t ecid[4];
	CHIPINFO_TYPE chipInfo;
	GetECID( ecid );
	ParseECID( ecid, &chipInfo );

#if (DUMP)
	do{
		sprintf( outFileName, "/mnt/mmc0/TestResult_%s_%x_%dx%d_IDS%d_RO%d_%d.txt",
							chipInfo.strLotID, chipInfo.waferNo, chipInfo.xPos, chipInfo.yPos, chipInfo.ids, chipInfo.ro,num );
		num++;
	} while(!access( outFileName, F_OK) || !cnt--);
	printf("dump file : %s  \n",outFileName);
	dump_fd = open(outFileName,O_RDWR | O_CREAT | O_DIRECT | O_SYNC);
	std_fd =  stdout_redirect(dump_fd);
#endif

	system("/root/cpu_md_vol -t -d 0");

#if (EN_CONNECT_ASB)
	CASBProtcol *pProto = new CASBProtcol();
	pProto->GetBin();
	pProto->ChipInfo( BIN_CPU_ID );
#endif

	printf("\n\n================================================\n");
	printf("    Start ASB Test(%d items)\n", gTotalTestItems);
	printf("================================================\n");

	gettimeofday(&start, NULL);


	for( i=0 ; i<gTotalTestItems ; i++ )
	{
		gettimeofday(&itemStart, NULL);
#if (EN_CONNECT_ASB)
		pProto->SetData( gTestItems[i].binNo );
#endif
		if( 0 != TestItem(&gTestItems[i]) )
		{
			error = 1;

#if (EN_CONNECT_ASB)
			break;
#endif
		}
		gettimeofday(&itemEnd, NULL);
		gTestItems[i].testTime = (uint64_t) ((itemEnd.tv_sec - itemStart.tv_sec)*1000 + (itemEnd.tv_usec - itemStart.tv_usec)/1000);
		fflush(stdout);
		sync();
	}

#if (EN_CONNECT_ASB)
	pProto->SetResult( (error==0)?0x10:0x1 );
#endif

	gettimeofday(&end, NULL);


	printf("\n    End ASB Test\n");
	printf("================================================\n");

	//	Output

	printf("================================================\n");
	printf("   Test Report (%d)msec \n", (int32_t)((end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000));
	printf("  binNo.  Name                      Result\n");
	printf("================================================\n");
	for( i=0 ; i<gTotalTestItems ; i++ )
	{
		printf("  %3d     %s        : %s(%d, %lldmsec)\n",
			gTestItems[i].binNo,
			gTestItems[i].descName,
			gstStatusStr[gTestItems[i].status+1], gTestItems[i].status,
			gTestItems[i].testTime );
	}
	printf("================================================\n\n\n");

	GetECID( ecid );
	ParseECID( ecid, &chipInfo );
	//PrintECID();
	if( 0 != error )
	{
		//	Yellow
		printf("\033[43m");
		printf("\n\n\n ASB ERROR!!!");
		printf("\033[0m\r\n");
	}
	else
	{
		//	Green
#if (DUMP)
		remove(outFileName);
#endif
		printf("\033[42m");
		printf("\n\n\n ASB SUCCESS!!!");
		printf("\033[0m\r\n");
	}
#if (DUMP)
	sync();
	close(dump_fd);
	stdout_restore(std_fd);
#endif
#if (EN_CONNECT_ASB)
	delete pProto;
#endif
	return 0;
}
