#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asv_command.h"


int main(int argc, char *argv[])
{
	char buffer[MAX_CMD_STR];
	ASV_COMMAND retCmd;
	ASV_MODULE_ID retModuleId;
	ASV_PARAM retParam;
	ASV_PARAM param;
	ASV_RESULT res;
	param.u32 = 400000000;
	//param.f32 = 0.125;
	res = MakeCommandString(buffer, sizeof(buffer), ASVC_SET_FREQ, ASVM_3D, param );
	if( ASV_RES_OK == res )
	{
		printf("Command String : %s\n", buffer);
	}
	else
	{
		printf("Error!!!\n");
	}

	{
		ASV_COMMAND retCmd = ASVC_SET_VOLT;
		ASV_MODULE_ID retModuleId = 0;
		ASV_PARAM retParam;
		if( ASV_RES_OK == ParseStringToCommand( buffer, sizeof(buffer), &retCmd, &retModuleId, &retParam ) )
		{
			if(retCmd == ASVC_SET_VOLT)
				printf("Parse Result : %s %s %f\n", ASVCommandToString(retCmd), ASVModuleIDToString(retModuleId), retParam.f32 );
			else if(retCmd == ASVC_SET_FREQ)
				printf("Parse Result : %s %s %d\n", ASVCommandToString(retCmd), ASVModuleIDToString(retModuleId), retParam.u32 );
			else
				printf("Parse Result : %s %s\n", ASVCommandToString(retCmd), ASVModuleIDToString(retModuleId) );
		}
	}


	getch();
	return 0;
}
