#include <stdio.h>
#include <unistd.h>		//	access
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ecid.h"

int32_t GetECID( uint32_t ecid[4] )
{
	char buffer[128];
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

static unsigned int ConvertMSBLSB( uint32_t data, int bits )
{
	uint32_t result = 0;
	uint32_t mask = 1;

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

static const char gst36StrTable[36] = 
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z'
};

static void LotIDNum2String( uint32_t lotId, char str[6] )
{
	uint32_t value[3];
	uint32_t mad[3];

	value[0] = lotId / 36;
	mad[0] = lotId % 36;

	value[1] = value[0] / 36;
	mad[1] = value[0] % 36;

	value[2] = value[1] / 36;
	mad[2] = value[1] % 36;

	// Lot ID String
	// value[2] mad[2] mad[1] mad[0]
	str[0] = 'N';
	str[1] = gst36StrTable[ value[2] ];
	str[2] = gst36StrTable[ mad[2] ];
	str[3] = gst36StrTable[ mad[1] ];
	str[4] = gst36StrTable[ mad[0] ];
	str[5] = '\0';
}

void ParseECID( uint32_t ecid[4], CHIPINFO_TYPE *chipInfo )
{
	//	Read GUID
	chipInfo->lotID   = ConvertMSBLSB( ecid[0] & 0x1FFFFF, 21 );
	chipInfo->waferNo = ConvertMSBLSB( (ecid[0]>>21) & 0x1F, 5 );
	chipInfo->xPos    = ConvertMSBLSB( ((ecid[0]>>26) & 0x3F) | ((ecid[1]&0x3)<<6), 8 );
	chipInfo->yPos    = ConvertMSBLSB( (ecid[1]>>2) & 0xFF, 8 );
	chipInfo->ids     = ConvertMSBLSB( (ecid[1]>>16) & 0xFF, 8 );
	chipInfo->ro      = ConvertMSBLSB( (ecid[1]>>24) & 0xFF, 8 );
	chipInfo->pid     = ecid[3] & 0xFFFF;
	chipInfo->vid     = (ecid[3]>>16) & 0xFFFF;

	LotIDNum2String( chipInfo->lotID, chipInfo->strLotID );
	printf( "  Chip Info : %s\t%d\t%d\t%d\t%d\t%d, vid=0x%04x, pid=0x%04x\n",
		chipInfo->strLotID, chipInfo->waferNo,		//	Wafer Information
		chipInfo->xPos, chipInfo->yPos,				//	Chip Postion
		chipInfo->ids, chipInfo->ro,				//	IDS & RO
		chipInfo->vid, chipInfo->pid);				//	Vendor ID & Product ID
}
