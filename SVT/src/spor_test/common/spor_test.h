/* *******************************************************
 *
 *    DEFINE MESSAGE TYPES
 *
 * *******************************************************/

#define POWER_ALIVE			0x10
#define CHECK_START			0x20
#define CHECK_SUCCESS		0x21
#define CHECK_FAILURE		0x22
#define TEST_START			0x30
#define TEST_SUCCESS		0x31
#define TEST_FAILURE		0x32

char *spor_msg[] = {
	[POWER_ALIVE]   =       "POWER:ALIVE",
	[CHECK_START]   =       "CHECK:START",
	[CHECK_SUCCESS] =       "CHECK:SUCCESS",
	[CHECK_FAILURE] =       "CHECK:FAILURE",
	[TEST_START]    =       "TEST:START",
	[TEST_SUCCESS]  =       "TEST:SUCCESS",
	[TEST_FAILURE]  =       "TEST:FAILURE"
};


