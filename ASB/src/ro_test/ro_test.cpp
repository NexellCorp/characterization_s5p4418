#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include "ecid.h"

int main(int argc, char *argv[] )
{
	uint32_t ecid[4];
	CHIPINFO_TYPE chipInfo;

	GetECID( ecid );
	ParseECID( ecid, &chipInfo );


	printf(" %d \n ", chipInfo.ro );
	if( chipInfo.ro > 40)
		return 0;
	else 
		return -1;
}
					
