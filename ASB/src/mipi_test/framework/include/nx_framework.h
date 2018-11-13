//------------------------------------------------------------------------------
//
//	Copyright (C) 2005 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module     :
//	File       :
//	Description:
//	Author     :
//	Export     :
//	History    :
//------------------------------------------------------------------------------

#ifndef _NX_FRAMEWORK_H_
#define	_NX_FRAMEWORK_H_

#include <nx_type.h>
#include <nx_debug.h>

#include <stdarg.h>

#include <cfg_framework.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// System PAD manager
//------------------------------------------------------------------------------
CBOOL			NX_PAD_Init(void);
CBOOL			NX_PAD_Deinit(void);
void			NX_PAD_SetPadFunctionEnable( U32 PADIndex, U32 ModeIndex );

//------------------------------------------------------------------------------
// System clock/control manager
//------------------------------------------------------------------------------
CBOOL			NX_SYS_Init(void);
CBOOL			NX_SYS_Deinit(void);

//------------------------------------------------------------------------------
// System timer manager
//------------------------------------------------------------------------------
CBOOL			NX_TIMER_Init(void);
CBOOL			NX_TIMER_Deinit(void);
U32				NX_TIMER_GetTickCount(void);
void			NX_TIMER_TickCountDelay(U32 ms);

//------------------------------------------------------------------------------
// System Console manager
//------------------------------------------------------------------------------
CBOOL			NX_CONSOLE_Init(void);
CBOOL			NX_CONSOLE_Deinit(void);
void			NX_CONSOLE_PutString(const char *buffer);
void 			NX_CONSOLE_Printf(const char *FormatString, ...);

//------------------------------------------------------------------------------
// System Non C allocator 1D/2D
//------------------------------------------------------------------------------
///	@brief 1D memory information structure
typedef struct
{
	U32 MemoryHandle;	///< memory handle for internal (do not modified it)
	U32 HeapID;         ///< heap id
	U32 Address;        ///< virtual address of memory block (16byte aligned)
	U32 Size;           ///< byte size of memory block

} NX_Memory1D;

///	@brief 2D memory information structure
typedef struct
{
	U32 MemoryHandle;	///< memory handle for internal (do not modified it)
	U32 HeapID;         ///< heap id
	U32 Address;        ///< virtual address of memory block
	U32 Stride;         ///< bytes per line
	U32 X;              ///< x position in 2D memory heap (unit: 8bit)
	U32 Y;              ///< y position in 2D memory heap
	U32 Width;          ///< memory block width (unit: 8bit)
	U32 Height;         ///< memory block height

} NX_Memory2D;

CBOOL 	NX_Malloc1D(U32 HeapID, U32 ByteSize, NX_Memory1D* pMemory1D);
void  	NX_Free1D(NX_Memory1D* pMemory1D);
CBOOL 	NX_Malloc2D(U32 HeapID, U32 Width, U32 Height, U32 AlignX, U32 AlignY,NX_Memory2D* pMemory2D);
void  	NX_Free2D(NX_Memory2D* pMemory2D);

//------------------------------------------------------------------------------
// ARM CPU manager
//------------------------------------------------------------------------------
void 	NX_ARM_EnableIRQ(void);
void	NX_ARM_DisableIRQ(void);

void 	NX_ARM_EnterCriticalSection( void );
void 	NX_ARM_LeaveCriticalSection( void );

U32		NX_ARM_GetCurMode( void );


//------------------------------------------------------------------------------
// SIMIO
//------------------------------------------------------------------------------
void  NX_SIMIO_SetTimeOutValue( U32 KClockValue );
void  NX_SIMIO_DebugBreak( void );
void  NX_SIMIO_Exit( void );

CBOOL NX_SIMIO_QueryPerformanceFrequency( U32* piFrequency );
CBOOL NX_SIMIO_QueryPerformanceCounter( U32* piCounter );

CBOOL NX_SIMIO_FillMemory( void* const pMemory, U8 Data, U32 ByteSize );

U32   NX_SIMIO_LoadHexFile( const char* const FileName, void* const pMemory );
CBOOL NX_SIMIO_LoadImgFile( const char* const FileName, U32 Stride, U32* pWidth, U32* pHeight, U32* pBPP, void* const pMemory );

CBOOL NX_SIMIO_SaveHexFile( const char* const FileName, void* const pMemory, U32 ByteSize );
CBOOL NX_SIMIO_SaveImgFile( const char* const FileName, U32 Stride, U32  Width, U32  Height, U32 BPP, void* const pMemory );

CBOOL NX_SIMIO_CompareMemory( const void* const pMemory0, const void* const pMemory1, U32 ByteSize );
CBOOL NX_SIMIO_CompareImage( const void* Source0, U32 Source0Stride, const void* Source1, U32 Source1Stride, U32 Width, U32 Height, U32 BPP );

U32		NX_SIMIO_LoadCodecBitCode	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
U32		NX_SIMIO_LoadCodecYUV	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
U32		NX_SIMIO_LoadCodecStream	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
CBOOL NX_SIMIO_SaveCodecYUV	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
CBOOL NX_SIMIO_SaveCodecStream	( const char* const FileName, void* const pMemory, U32 ByteSize, U32 PrevSize );
CBOOL NX_SIMIO_MEMCOPY	( const void* const pMemory0, const void* const pMemory1, U32 ByteSize );
CBOOL NX_SIMIO_CMPBINFILE	( const char* const FileName0, const char* const FileName1, U32 ByteSize );

//@choiyk
U32   NX_SIMIO_TD_READ ( U32  Addr );
CBOOL NX_SIMIO_TD_WRITE( U32  Addr, U32 Data );

//------------------------------------------------------------------------------
// CLKGEN
//------------------------------------------------------------------------------
//void		NX_CLKGEN_SetClockSource( U32 ModuleIndex, U32 Index, U32 ClkSrc );
//U32			NX_CLKGEN_GetClockSource( U32 ModuleIndex, U32 Index );
//void		NX_CLKGEN_SetClockDivisor( U32 ModuleIndex, U32 Index, U32 Divisor );
//U32			NX_CLKGEN_GetClockDivisor( U32 ModuleIndex, U32 Index );
//void		NX_CLKGEN_SetClockDivisorEnable( U32 ModuleIndex, CBOOL Enable );
//CBOOL		NX_CLKGEN_GetClockDivisorEnable( U32 ModuleIndex );
typedef enum
{
	NX_CLK14P7456MHZ , //  14.7456 Mhz  UART CLOCK
	NX_CLK12P288MHZ  , //  12.288  Mhz  I2S, AC97
	NX_CLK50MHZ      , //  50.000  Mhz  SDMMC,
	NX_CLK100MHZ     , // 100.000  Mhz  SDMMC (DDR MODE), DISPLAY (1280x1024), TIMER, SSP, VIP
	NX_CLK125MHZ     , // 125.000  Mhz  GMAC (ethernet)
	NX_CLK147P456MHZ , // 147.456  Mhz
} NX_CLK;

void		NX_CLOCK_SetClockEnable( U32 ClockIndex,  CBOOL	Enable );
CBOOL		NX_CLOCK_GetClockEnable( U32 ClockIndex );
void		NX_CLOCK_SetClockDivisorEnable( U32 ClockIndex, CBOOL Enable );
CBOOL		NX_CLOCK_GetClockDivisorEnable( U32 ClockIndex, U32 Index );
CBOOL		NX_CLOCK_SetClockDivisor( U32 ClockIndex, U32 Index, NX_CLK Freq, float* pFreq );

//------------------------------------------------------------------------------
// RSTCON
//------------------------------------------------------------------------------
void		NX_RESET_EnterReset ( U32 RSTIndex );
void		NX_RESET_LeaveReset ( U32 RSTIndex );
CBOOL		NX_RESET_GetStatus  ( U32 RSTIndex ); // returns CTRUE, if it's in reset status.

//------------------------------------------------------------------------------
// SWITCHDEVICE
//------------------------------------------------------------------------------
void 	NX_SWITCHDEVICE_Set_Switch_Enable( U32 index );
CBOOL 	NX_SWITCHDEVICE_Get_Switch_Status( U32 index );
















































#ifdef __cplusplus
}
#endif

#endif
