#include "spi_test.h"

//#define DEBUG

#define ENV_OFF		32 * 1024 
#define ENV_SIZE	32 * 1024

const char *device = "/dev/spidev0.0";

uint8_t mode = SPI_MODE_3 | SPI_CPOL | SPI_CPHA;
uint8_t bits = 8;
uint32_t speed = 1000000;
uint16_t delay = 0;

int eeprom_init(int fd)
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


int eeprom_read(int fd,int offset, char * buf, int size)
{
	int ret;

	uint8_t * tx;
	uint8_t * rx;

	tx = (uint8_t *)malloc(size+CMD_BUF);
	rx = (uint8_t *)malloc(size+CMD_BUF);
	//memset ( rx , 0xcc,size+CMD_BUF );
	//memset ( tx , 0x0a,size+CMD_BUF );
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
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
	{
		perror("can't send spi message");
		return -1;
	}
	memcpy(buf, rx+CMD_BUF+4,size);
	free(tx);		
	free(rx);		
	return ret;
}

static int read_env(char  *str, char *rbuf)
{
	int fd;
	char *Rx;
	char *env=NULL;
	char cmd[128];
	char param[128];

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
		//return -1;
	}
	env = Rx;
	do{
    	sscanf(env,"%[^=]=%[^\n];",cmd,param);
    	if(!strcmp(str,cmd)){
			memcpy(rbuf,param,strlen(env));
			ret = 0;
		    break;

		}   
  		l = strlen(env);
		if(l==0)
			l=1;
		 env=env+l;
    	 total += l;
    }while(total<1024);

	free(Rx);
	close(fd);
	return ret;
}

int main(int argc, char **argv)
{
	unsigned char *test= malloc(128);

	read_env("baudrate",test);
	printf("%s\n",test);

	return 0;
}
