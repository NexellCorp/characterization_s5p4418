#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asv_command.h"

#pragma warning (disable:4996)

static const char *gStrAsvCmdStr[] = {
	"ASVC_SET_FREQ",
	"ASVC_SET_VOLT",
	"ASVC_GET_ECID",
	"ASVC_RUN",
	"ASVC_STATUS",
	"ASVC_ON",			//	PC Application Only
	"ASVC_OFF",			//	PC Application Only
	"ASVC_MAX",
};

static const char *gStrAsvModuleIdStr[] = {
	"ASVM_CPU",
	"ASVM_VPU",
	"ASVM_3D",
	"ASVM_LDO_SYS",
	"ASVM_MAX",
};

static const char *gStrAsvModuleIdStrSimple[] = {
	"CPU",
	"VPU",
	"3D",
	"MAX",
};

const char *ASVCommandToString( ASV_COMMAND cmd )
{
	if( (cmd >= ASVC_MAX) || (cmd < ASVC_SET_FREQ) )
	{
		return NULL;
	}
	return gStrAsvCmdStr[cmd];
}

ASV_COMMAND ASVStringToCommand( char *cmdStr )
{
	ASV_COMMAND i;
	for( i=ASVC_SET_FREQ ; i<ASVC_MAX ; i++ )
	{
		if( !strncmp(gStrAsvCmdStr[i], cmdStr, strlen(gStrAsvCmdStr[i])) )
		{
			return (ASV_COMMAND)i;
		}
	}
	return ASVC_MAX;
}

const char *ASVModuleIDToString(ASV_MODULE_ID id)
{
	if( (id >= ASVM_MAX) || (id < ASVM_CPU) )
	{
		return NULL;
	}
	return gStrAsvModuleIdStr[id];
}

const char *ASVModuleIDToStringSimple(ASV_MODULE_ID id)
{
	if( (id >= ASVM_MAX) || (id < ASVM_CPU) )
	{
		return NULL;
	}
	return gStrAsvModuleIdStrSimple[id];
}


ASV_MODULE_ID ASVStringModuleID( char *idStr)
{
	ASV_MODULE_ID i;
	for( i=ASVM_CPU ; i<ASVM_MAX ; i++ )
	{
		if( !strncmp(gStrAsvModuleIdStr[i], idStr, strlen(gStrAsvModuleIdStr[i])) )
		{
			return (ASV_MODULE_ID)i;
		}
	}
	return ASVM_MAX;
}

ASV_RESULT MakeCommandString( char *outBuf, int32_t outSize, ASV_COMMAND cmd, ASV_MODULE_ID id, ASV_PARAM param )
{
	const char *cmdStr=NULL, *idStr=NULL;
	char paramStr[64];
	memset( paramStr, 0 , sizeof(paramStr) );

	cmdStr = ASVCommandToString(cmd);
	idStr = ASVModuleIDToString(id);

	if( NULL == cmdStr || NULL == idStr )
		return ASV_RES_ERR;

	strcpy(outBuf, cmdStr);
	strcat(outBuf, " ");			//	Add Space
	strcat(outBuf, idStr);

	if( ASVC_SET_FREQ == cmd )
	{
		strcat(outBuf, " ");			//	Add Space
		sprintf( paramStr, "%d", param.u32 );
		strcat(outBuf, paramStr);
	}
	else if( ASVC_SET_VOLT == cmd )
	{
		strcat(outBuf, " ");			//	Add Space
		sprintf( paramStr, "%f", param.f32 );
		strcat(outBuf, paramStr);
	}
	outBuf[strlen(outBuf)] = '\n';
	return ASV_RES_OK;
}


//
ASV_RESULT ParseStringToCommand( char *inBuf, int32_t intSize, ASV_COMMAND *cmd, ASV_MODULE_ID *id, ASV_PARAM *param )
{
	ASV_COMMAND asvCmd;
	ASV_MODULE_ID moduleId;
	char cmds[MAX_CMD_ARG][MAX_CMD_STR];
	int32_t cmdCnt;
	
	memset(cmds, 0, sizeof(cmds));
	
	cmdCnt = GetArgument( inBuf, cmds );

	if( cmdCnt < 2 )
	{
		return ASV_RES_ERR;
	}

	asvCmd = ASVStringToCommand( cmds[0] );
	if( asvCmd >= ASVC_MAX )
	{
		return ASV_RES_ERR;
	}
	moduleId = ASVStringModuleID( cmds[1] );
	if( moduleId == ASVM_MAX )
	{
		return ASV_RES_ERR;
	}

	switch( asvCmd )
	{
		case ASVC_SET_FREQ:
			param->u32 = atoi(cmds[2]);
			break;
		case ASVC_SET_VOLT:
#ifdef WIN32
			_atoflt((_CRT_FLOAT*)&param->f32, cmds[2]);
#else
			param->f32 = strtof( cmds[2], NULL );
#endif
			break;
		case ASVC_GET_ECID:
			break;
		case ASVC_RUN:
			break;
		default:
			return ASV_RES_ERR;
	}
	*cmd = asvCmd;
	*id = moduleId;
	return ASV_RES_OK;
}


//
//
//	String Util Functions
//
//
int32_t GetArgument( char *pSrc, char arg[][MAX_CMD_STR] )
{
	int32_t	i, j;
	// Reset all arguments
  	for( i=0 ; i<MAX_CMD_ARG ; i++ ) 
  	{
  		arg[i][0] = 0;
  	}
  	for( i=0 ; i<MAX_CMD_ARG ; i++ )
	{
		// Remove space char
  		while( *pSrc == ' ' ) 	pSrc++;

		// check end of string.
	  	if( *pSrc == 0 )  		break;

	 	j=0;
		while( (*pSrc != ' ') && (*pSrc != 0) )
		{
		 	arg[i][j] = *pSrc++;
		 	if( arg[i][j] >= 'a' && arg[i][j] <= 'z' ) 		// to upper char
		 		arg[i][j] += ('A' - 'a');

		 	j++;
		 	if( j > (MAX_CMD_STR-1) ) 	j = MAX_CMD_STR-1;
	  	}
	  	arg[i][j] = 0;
  	}
	return i;
}
