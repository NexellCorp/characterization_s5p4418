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

#ifndef	__DVFSUTIL_H__
#define	__DVFSUTIL_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum{
	REGUL_ID_CPU    = 1,
	REGUL_ID_CORE   = 2,
	REGUL_ID_SYSTEM = 3,
	REGUL_ID_LDO_SYS	= 9,
}REGUL_ID_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

int32_t SetCPUFrequency( uint32_t freq );
int32_t Set3DFrequency( uint32_t freq );
int32_t SetVPUFrequency( uint32_t freq );

int32_t SetCPUVoltage( uint32_t microVolt );		//	CPU
int32_t SetCoreVoltage( uint32_t microVolt );		//	MPEG/3D
int32_t SetSystemVoltage( uint32_t microVolt );		//	System(3.3v)
int32_t SetSystemLDOVoltage( uint32_t microVolt );	//	LDO Number 4

int32_t SetRegulatorVoltage( REGUL_ID_TYPE id, uint32_t microVolt );

int32_t GetECID( uint32_t ecid[4] );

uint64_t NX_GetTickCountUs( void );

#ifdef __cplusplus
}
#endif

#endif	//	__NX_GPIO_H__
