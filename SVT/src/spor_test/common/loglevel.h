#include <stdio.h>

/* ******************************************************** */
/*                          LogLevel                        */
/* ******************************************************** */

#define LogDebug		3
#define LogInfo			2
#define LogError		1
#define LogNone			0

#ifndef NX_LogLevel
#define NX_LogLevel		LogDebug
#endif

#define no_print		do {} while(0)

#if NX_LogLevel > LogInfo
//#define MK_FMT(fmt)				"%s@%s:%d| " fmt , __func__, __FILE__, __LINE__
#define MK_FMT(fmt)				fmt
#else
#define MK_FMT(fmt)				fmt
#endif

#if NX_LogLevel > LogInfo
#define nx_debug(fmt, ...)		printf(MK_FMT(fmt), ##__VA_ARGS__)
#define nx_info(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#define nx_error(fmt, ...)		printf(fmt, ##__VA_ARGS__)

#elif NX_LogLevel > LogNorm
#define nx_debug(fmt, ...)		no_print
#define nx_info(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#define nx_error(fmt, ...)		printf(fmt, ##__VA_ARGS__)

#elif NX_LogLevel > LogPrint
#define nx_debug(fmt, ...)		no_print
#define nx_info(fmt, ...)		no_print
#define nx_error(fmt, ...)		printf(fmt, ##__VA_ARGS__)

#elif NX_LogLevel == LogNone
#define nx_debug(fmt, ...)		no_print
#define nx_info(fmt, ...)		no_print
#define nx_error(fmt, ...)		no_print

#else
#error "set correct loglevel"
#endif
