#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "spi.h"

#define DEV_NAME "/dev/spidev2.0"

CSPI::CSPI()
	: m_hSpi(-1)
	, m_Mode( SPI_MODE_3 | SPI_CPOL | SPI_CPHA )
	, m_Speed(1000000)	//	1 Mbps
	, m_Delay(0)		//	No Delay
	, m_BitsPerWord(8)
{
	m_hSpi = open(DEV_NAME, O_RDWR);
	int ret;
	ret = ioctl(m_hSpi, SPI_IOC_WR_MODE, &m_Mode);
	if (ret == -1)
	{
		printf("can't set spi mode\n");
		goto ErrorExit;
	}
	ret = ioctl(m_hSpi, SPI_IOC_RD_MODE, &m_Mode);
	if (ret == -1)
	{
		printf("can't get spi mode\n");
		goto ErrorExit;
	}
	ret = ioctl(m_hSpi, SPI_IOC_WR_MAX_SPEED_HZ, &m_Speed);
	if (ret == -1)
	{
		printf("can't set max speed hz\n");
		goto ErrorExit;
	}
	ret = ioctl(m_hSpi, SPI_IOC_RD_MAX_SPEED_HZ, &m_Speed);
	if (ret == -1)
	{
		printf("can't get max speed hz\n");
		goto ErrorExit;
	}
	return ;
ErrorExit:
	if( m_hSpi > 0 )
	{
		close( m_hSpi );
		m_hSpi = -1;
	}
}

CSPI::~CSPI()
{
	if( m_hSpi > 0 )
	{
		close( m_hSpi );
	}
}

#if 0
int eeprom_read(int fd, int offset, unsigned char * buf, int size)
{
    int ret;

    uint8_t * tx;
    uint8_t * rx;

    tx = (uint8_t *)malloc(size+CMD_BUF);
    rx = (uint8_t *)malloc(size+CMD_BUF);
    
    memset ( rx , 0xcc,size+CMD_BUF );
    memset ( tx , 0x0a,size+CMD_BUF );
    tx[0] =  READ_CMD;

    if(ADDR_BYTE == 3)
    {
        tx[1] = (offset >> 16);
        tx[2] = (offset >> 8);
        tx[3] = (offset );
    }
    else if(ADDR_BYTE == 2)
    {
        tx[1] = (offset >> 8);
        tx[2] = (offset);
    }
    
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = size + CMD_BUF,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = 8,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret == 1)
    {
        perror("can't send spi message");
        return -1;
    }
    memcpy(buf, rx+CMD_BUF,size);
   
    free(tx);
    free(rx);
    
    return ret;
}
#endif

int32_t CSPI::Read( uint8_t *buf, int32_t bufSize )
{
	if( m_hSpi < 0 )
		return -1;

	uint8_t *tx = (uint8_t*)calloc(bufSize, 1);
	uint8_t *rx = buf;
	struct spi_ioc_transfer spiIoc;

	spiIoc.tx_buf = (uint32_t)tx;
	spiIoc.rx_buf = (uint32_t)rx;
	spiIoc.len = (uint32_t)bufSize;
	spiIoc.delay_usecs = m_Delay;
	spiIoc.speed_hz = m_Speed;
	spiIoc.bits_per_word = m_BitsPerWord;
	spiIoc.cs_change = 0;
	spiIoc.pad = 0;

	int32_t ret = ioctl(m_hSpi, SPI_IOC_MESSAGE(1), &spiIoc);

	free(tx);
	if (ret == 1)
	{
		printf("can't send spi message\n");
		return -1;
	}

	return bufSize;
}

int32_t CSPI::Write( uint8_t *buf, int32_t len )
{
	if( m_hSpi < 0 )
		return -1;

	uint8_t *tx = buf;
	uint8_t *rx = (uint8_t*)malloc(len);
	struct spi_ioc_transfer spiIoc;

	spiIoc.tx_buf = (uint32_t)tx;
	spiIoc.rx_buf = (uint32_t)rx;
	spiIoc.len = (uint32_t)len;
	spiIoc.delay_usecs = m_Delay;
	spiIoc.speed_hz = m_Speed;
	spiIoc.bits_per_word = m_BitsPerWord;
	spiIoc.cs_change = 0;
	spiIoc.pad = 0;

	int32_t ret = ioctl(m_hSpi, SPI_IOC_MESSAGE(1), &spiIoc);
	free(rx);
	if (ret == 1)
	{
		printf("can't send spi message\n");
		return -1;
	}

	return len;
}
