#ifndef __ASB_PROTOCOL_H__
#define __ASB_PROTOCOL_H__

#include <stdint.h>
#include "nx_gpio.h"
#include "spi.h"

#define	DUMP_PROTOCOL		0

#define	NUM_OF_SUB_MODULE	8

#define	GPIO_ASB_READY		GPIOA3		//	ASB Ready IN
#define	GPIO_DUT_READY		ALIVE2		//	DUT Ready OUT

enum{
	CMD_ASB_TO_DUT = 0xC,
	CMD_DUT_TO_ASB = 0xA,
};

typedef enum{
	OP_GETBIN	= 0xa0,
	OP_SETBIN	= 0xa1,
	OP_SETDATA	= 0xa2,
	OP_SETID	= 0xa3,
	OP_CHIPINFO	= 0xa4,
} ASB_CTRL_OPCODE;


typedef struct ASB_CTRL_PACKET{
	uint32_t	type:4;
	uint32_t	opcode:12;
	uint32_t	crc:16;
	uint32_t	reserved;
	//uint16_t	data[NUM_OF_SUB_MODULE];
	uint32_t	data[NUM_OF_SUB_MODULE];
} ASB_CTRL_PACKET;


class CASBProtcol{
public:
	CASBProtcol();
	virtual ~CASBProtcol();

	bool GetBin();
	bool SetResult( uint32_t result );
	bool SetData( uint32_t bin );
	bool SetId();
	bool ChipInfo( uint32_t bin );

private:
	CSPI *m_hSpi;
	bool InitGPIO();
	NX_GPIO_HANDLE m_hAsbReady;
	NX_GPIO_HANDLE m_hDutReady;
};


#endif	// __ASB_PROTOCOL_H__
