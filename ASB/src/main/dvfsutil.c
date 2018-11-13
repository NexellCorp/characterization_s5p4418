//------------------------------------------------------------------------------
//
//  Copyright (C) 2013 Nexell Co. All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      :
//  File        :
//  Description :
//  Author      : 
//  Export      :
//  History     :
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>		// malloc
#include <unistd.h>		// write
#include <fcntl.h>		// open
#include <assert.h>		// assert
#include <stdint.h>
#include <pthread.h>
#include <sys/poll.h>

#include "dvfsutil.h"

#define	MAX_FILE_PATH	256
#define	MAX_FILE_DATA	128

#define	MAX_NUM_CPU_CORE	4

// #> echo userspace  > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor	//	1. user mode 설정
// #> echo 1000000 >  /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed	//	2. 원하는 freq설정
#define	CPU_FREQ_FILE_PREFIX	"/sys/devices/system/cpu/cpu"

// DCDC1(ARM) :  /sys/class/regulator/regulator.1 # echo 1100000 > microvolts
// DCDC2(CORE) : /sys/class/regulator/regulator.2
// DCDC3(SYS) :   /sys/class/regulator/regulator.3
#define	PMIC_SYS_FILE_PREFIX	"/sys/class/regulator/regulator"
#define	PMIC_SYS_VOLTS_FILE		"microvolts"

// /sys/devices/platform/pll/pll.2
#define	VPU_3D_FREQ_FILE		"/sys/devices/platform/pll/pll.2"


static int32_t _WriteSysInterface( const char *file, const char *buffer, int32_t bufSize )
{
	int32_t fd;
	if( 0 != access(file, F_OK) ) {
		printf("Cannot access file (%s).\n", file);
		return -1;
	}
	fd = open(file, O_WRONLY);
	if( fd < 0 )
	{
		printf("Cannot open file (%s).\n", file);
		return -1;
	}

	printf("[[[[[ Write %s", buffer);
	if( 0 > write(fd, buffer, bufSize) ) {
		printf("Write failed. (file=%s, data=%s)\n", file, buffer);
		close(fd);
		return -1;
	}
	close(fd);

#if 1
	{
		char readBuf[64];
		fd = open(file, O_RDONLY);
		if( fd < 0 )
		{
			printf("Cannot open file (%s).\n", file);
			return -1;
		}

		if( 0 > read(fd, readBuf, sizeof(readBuf)) ) {
			printf("Read failed. (file=%s, data=%s)\n", file, buffer);
			close(fd);
			return -1;
		}
		printf("[[[[[ Read %s", readBuf);
		close(fd);
	}
#endif

	return 0;
}


//
//	Freqeuncy Control APIs
//
#if 0
int32_t SetCPUFrequency( uint32_t freq )
{
	int32_t i, len;
	char fileName[MAX_FILE_PATH];
	char dataStr[MAX_FILE_DATA];

	for( i=0 ; i<MAX_NUM_CPU_CORE ; i++ )
	{
		//	Set to User Mode
		snprintf(fileName, sizeof(fileName), "%s%d/cpufreq/scaling_governor", CPU_FREQ_FILE_PREFIX, i);
		len = snprintf( dataStr, sizeof(dataStr), "%s", "userspace" );
		if( _WriteSysInterface( fileName, dataStr, len ) < 0 )
		{
			return -1;
		}
		//	Set Frequency
		snprintf(fileName, sizeof(fileName), "%s%d/cpufreq/scaling_setspeed", CPU_FREQ_FILE_PREFIX, i);
		len = snprintf( dataStr, sizeof(dataStr), "%d", freq/1000 );
		if( _WriteSysInterface( fileName, dataStr, len ) < 0 )
		{
			return -1;
		}
	}
	return 0;
}
#else
#define CPU_FREQ_FILE	"/sys/devices/platform/pll/pll.1"
int32_t SetCPUFrequency( uint32_t freq )
{
	char dataStr[MAX_FILE_DATA];
	sprintf(dataStr, "%d", freq/1000);
	return _WriteSysInterface( CPU_FREQ_FILE, dataStr, strlen(dataStr) );
}
#endif

int32_t Set3DFrequency( uint32_t freq )
{
	char dataStr[MAX_FILE_DATA];
	sprintf(dataStr, "%d", freq/500);
	return _WriteSysInterface( VPU_3D_FREQ_FILE, dataStr, strlen(dataStr) );
}

int32_t SetVPUFrequency( uint32_t freq )
{
	char dataStr[MAX_FILE_DATA];
	sprintf(dataStr, "%d", freq/500);
	return _WriteSysInterface( VPU_3D_FREQ_FILE, dataStr, strlen(dataStr) );
}


//
//	Voltage Control APIs
//
int32_t SetCPUVoltage( uint32_t microVolt )			//	CPU
{
	return SetRegulatorVoltage(REGUL_ID_CPU, microVolt);
}

int32_t SetCoreVoltage( uint32_t microVolt )		//	MPEG/3D
{
	return SetRegulatorVoltage(REGUL_ID_CORE, microVolt);
}

int32_t SetSystemVoltage( uint32_t microVolt )		//	System(3.3v)
{
	return SetRegulatorVoltage(REGUL_ID_SYSTEM, microVolt);
}

int32_t SetSystemLDOVoltage( uint32_t microVolt )		//	LDO Number 4
{
	return SetRegulatorVoltage(REGUL_ID_LDO_SYS, microVolt);
}

int32_t SetRegulatorVoltage( REGUL_ID_TYPE id, uint32_t microVolt )
{
	char fileName[MAX_FILE_PATH];
	char dataStr[MAX_FILE_DATA];
	uint32_t cutOffVolt = microVolt/100;
	// Cut off 100 micro volt.
	cutOffVolt = cutOffVolt*100;

	if( cutOffVolt != microVolt )
	{
		printf("[Regulator %d] : CutOffVolt=%d, microVolt=%d\n", id, cutOffVolt, microVolt);
	}

	sprintf(fileName, "%s.%d/%s", PMIC_SYS_FILE_PREFIX, id, PMIC_SYS_VOLTS_FILE);
	sprintf(dataStr, "%d", microVolt);
	return _WriteSysInterface( fileName, dataStr, strlen(dataStr) );
}


//
//
#define	ECID_FILE_NAME	"/sys/devices/platform/cpu/uuid"
int32_t GetECID( uint32_t ecid[4] )
{
	char *buffer[128];
	int32_t fd;
	if( 0 != access(ECID_FILE_NAME, F_OK) ) {
		printf("Cannot access file (%s).\n", ECID_FILE_NAME);
		return -1;
	}
	fd = open(ECID_FILE_NAME, O_RDONLY);
	if( fd < 0 )
	{
		printf("Cannot open file (%s).\n", ECID_FILE_NAME);
		return -1;
	}

	if( 0 > read(fd, buffer, sizeof(buffer)) ) {
		printf("read failed. (file=%s, data=%s)\n", ECID_FILE_NAME, buffer);
		close(fd);
		return -1;
	}
	close(fd);

	sscanf(buffer, "%x:%x:%x:%x\n", &ecid[0], &ecid[1], &ecid[2], &ecid[3]);
	return 0;
}

uint64_t NX_GetTickCountUs( void )
{
	uint64_t ret;
	struct timeval	tv;
	gettimeofday( &tv, NULL );
	ret = ((uint64_t)tv.tv_sec)*1000000 + tv.tv_usec;
	return ret;
}
