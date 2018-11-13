#ifndef __SPI_H__
#define	__SPI_H__

#include <stdint.h>

class CSPI{
public:
	CSPI();
	~CSPI();

	int32_t Read( uint8_t *buf, int32_t bufSize );
	int32_t Write( uint8_t *buf, int32_t len );
private:
	int32_t m_hSpi;
	uint8_t m_Mode;
	uint32_t m_Speed;
	uint32_t m_Delay;
	uint8_t m_BitsPerWord;
};

#endif	//	__SPI_H__