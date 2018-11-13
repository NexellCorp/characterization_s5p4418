#ifndef __ECID_H__
#define	__ECID_H__

#include <stdint.h>

#define	ECID_FILE_NAME	"/sys/devices/platform/cpu/uuid"

typedef struct CHIPINFO_TYPE{
	uint32_t lotID;
	uint32_t waferNo;
	uint32_t xPos, yPos;
	uint32_t ids, ro;
	uint32_t vid, pid;
	char strLotID[6];	//	Additional Information
} CHIPINFO_TYPE;

int32_t GetECID( uint32_t ecid[4] );
void ParseECID( uint32_t ecid[4], CHIPINFO_TYPE *chipInfo );

#endif	//	__ECID_H__