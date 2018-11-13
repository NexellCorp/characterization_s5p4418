#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <wchar.h>
#include <time.h>    
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define	READ_CMD		0x03
#define	WRITE_CMD		0x02
#define	WRITE_ENABLE	0x06
#define	RDSR			0x05

#define SECTOR_REASE		0x20
#define	BLOCK_32K_ERASE		0x52
#define	BLOCK_64K_ERASE		0xd8

#define	ADDR_BYTE	3
#define	CMD_LEN		1

#define	CMD_BUF ADDR_BYTE + CMD_LEN

#define PAGE_SIZE 256 

#define WIP		1<<1
#define	BUSY	1<<1
#define WEL 	1<<2

#define RDSR_MAX_CNT	50

#define DEFAULT_ADDR	0x40000000


//#define DEBUG

#define ENV_OFF		32 * 1024 
#define ENV_SIZE	32 * 1024

const char *device = "/dev/spidev0.0";

uint8_t mode = SPI_MODE_3 | SPI_CPOL | SPI_CPHA;
uint8_t bits = 8;
uint32_t speed = 1000000;
uint16_t delay = 0;

static int eeprom_init(int fd)
{
	int ret;
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		perror("can't set spi mode");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
	{
		perror("can't get spi mode");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		perror("can't set max speed hz");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		perror("can't get max speed hz");
		return ret;
	}
	//printf("spi mode: %d\n", mode);
	//printf("bits per word: %d\n", bits);
	//printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	return ret;
}


static int eeprom_read(int fd,int offset, char * buf, int size)
{
	int ret;

	uint8_t * tx;
	uint8_t * rx;

	tx = (uint8_t *)malloc(size+CMD_BUF);
	rx = (uint8_t *)malloc(size+CMD_BUF);
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

	struct spi_ioc_transfer tr;
	tr.tx_buf = (uint32_t)tx;
	tr.rx_buf = (uint32_t)rx;
	tr.len = size + CMD_BUF;
	tr.delay_usecs = delay;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;
	tr.cs_change = 0;
	tr.pad = 0;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
	{
		perror("can't send spi message");
		free(tx);		
		free(rx);		
		return -1;
	}
	memcpy(buf, rx+CMD_BUF+4,size);
	free(tx);		
	free(rx);		
	return ret;
}

int read_env(char  *str, char *rbuf)
{
	int fd;
	char *Rx;
	char *env=NULL;
	static char cmd[128];
	static char param[1024];

	int l = 0;
    int total =0; 
	int ret = -1;
	bits = 8;
	speed = 500000;
	fd = open(device, O_RDWR);
	
	if (fd < 0){
		printf("fail open spi \n");
		return -1;
	}
	eeprom_init(fd);
	
	Rx =(char *) malloc(ENV_SIZE);
	if(eeprom_read(fd,ENV_OFF,Rx,1024)<0)
	{
		printf("fail read : spi eeprom env \n");
		return -1;
	}
	env = Rx;
	memset(rbuf,0x00,128);

	do{
    	sscanf(env,"%[^=]=%[^\n];",cmd,param);
    	if(!strcmp(str,cmd)){
			printf(" %s , %d\n", param, strlen(param) );
			param[strlen(param)] = 0;
			memcpy(rbuf,param,strlen(param)+1);
			printf(" %s \n", rbuf );
			ret = 0;
		    break;

		}   
  		l = strlen(env);
		if(l==0)
			l=1;
		 env=env+l;
    	 total += l;
		 memset(param,0x00,128);
    }while(total<1024);


	free(Rx);
	close(fd);

	return ret;
}

#if 0
int main(int argc, char **argv)
{
	unsigned char *test= malloc(128);

	read_env("baudrate",test);
	printf("%s\n",test);

	return 0;
}
#endif
