#include "spi_test.h"

//#define DEBUG

char *device = "/dev/spidev0.0";

uint8_t mode = SPI_MODE_3 | SPI_CPOL | SPI_CPHA;
uint8_t bits = 8;
uint32_t speed = 10000000;
uint16_t delay = 0;


void print_usage(void)
{
	    printf( "usage: options\n"
	            "-s size (Byte)          default : 1024 byte \n"
	            "-a wirte,read address   default : 0x0 \n"
	            "-d device node          default : /dev/spidev0.0   \n"
	            "-h ptint this message   \n"
		);
}

unsigned int atoh(char *a)
{
    char ch = *a;
    unsigned int cnt = 0, hex = 0;

    while(((ch != '\r') && (ch != '\n')) || (cnt == 0)) {
        ch  = *a;

        if (! ch)
            break;

        if (cnt != 8) {
            if (ch >= '0' && ch <= '9') {
                hex = (hex<<4) + (ch - '0');
                cnt++;
            } else if (ch >= 'a' && ch <= 'f') {
                hex = (hex<<4) + (ch-'a' + 0x0a);
                cnt++;
            } else if (ch >= 'A' && ch <= 'F') {
                hex = (hex<<4) + (ch-'A' + 0x0A);
                cnt++;
            }
        }
        a++;
    }
    return hex;
}


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
int eeprom_read_rdsr(int fd)
{
	int ret;
	uint8_t tx[4]={0,0,0,0};	
	uint8_t rx[4];

	tx[0] =  RDSR;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
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
			
	return rx[1];
}


int eeprom_read(int fd,int offset, unsigned char * buf, int size)
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
		.bits_per_word = bits,
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
int eeprom_blk_erase(int fd, char cmd, int offset)
{
	int ret,cnt = RDSR_MAX_CNT;
	uint8_t tx[4];	
	uint8_t rx[4];

	tx[0] =  cmd;

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

		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
	{
		return -1;
	}	
	
	do{	
	usleep(1000);
	ret = eeprom_read_rdsr(fd);
	ret = ret & BUSY;
	}	while(ret != 0 && cnt--);

	return 0;
}


int eeprom_write_enable(int fd)
{
	int ret;
	uint8_t tx[4] ={ 6,6};	
	uint8_t rx[4] = {0xaa,0xaa,};
	int cnt = RDSR_MAX_CNT;
	tx[0] =  WRITE_ENABLE;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 1,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	 do{ 
	     usleep(10000);
	     ret = eeprom_read_rdsr(fd);
     }   while(ret != 2  && cnt--);

	return ret;
}

int eeprom_write(int fd, int dst,unsigned char * buf, int size)
{
	int ret = 0,offset=0,cnt = RDSR_MAX_CNT;
	uint8_t tx[PAGE_SIZE+CMD_BUF];	
	uint8_t rx[PAGE_SIZE+CMD_BUF];

	int write_size =0;
	//int cmp=0;
	tx[0] =  WRITE_CMD;

	do{
		eeprom_write_enable(fd);
		if(size > PAGE_SIZE)
		{
			write_size = PAGE_SIZE - (dst%PAGE_SIZE);
		}
		else
		{	
			if((PAGE_SIZE - (dst%PAGE_SIZE))  > size)
				write_size = size;
			else 
				write_size = (PAGE_SIZE - (dst%PAGE_SIZE));	
		}	

		if(ADDR_BYTE == 3)
		{	
			tx[1] = (dst >> 16);
			tx[2] = (dst >> 8);
			tx[3] = (dst );
		}
		else if(ADDR_BYTE == 2)
		{
			tx[1] = (dst >> 8);
			tx[2] = (dst );
		}
		
		memcpy(tx+CMD_BUF,&buf[offset],write_size);

		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = write_size+CMD_BUF,
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

		size -= write_size;
		offset += write_size;
		dst += write_size;
			
		do{	
			usleep(10000);
			ret = eeprom_read_rdsr(fd);
			ret = ret & (WEL | BUSY);
		}while(ret != 0  && cnt-- );
				
	}while(size > 0);


	return 0;

}

int main(int argc, char **argv)
{
	int ret,i, opt;
	int size = 128 ;
	int  addr = DEFAULT_ADDR ;

	uint8_t *read_buf = NULL;
	uint8_t *write_buf =  NULL; 
	int fd;
	while (-1 != (opt = getopt(argc, argv, "s:a:d:h"))) {
		switch(opt) {
		case 's' : 		size	= atoi(optarg); break;
		case 'a' : 		addr	= atoh(optarg); break;
		case 'd' :		device	= strdup(optarg); break;
		case 'h' :		print_usage(); exit(0); break;
 		}
	}
	bits = 8;
	
	speed = 500000;
	fd = open(device, O_RDWR);
	
	if (fd < 0){
		printf("fail open spi \n");
		return -1;
	}

	if( 0 !=eeprom_init(fd))
	{
		printf("fail init eeprom \n");
		return -1;
	}
	srand(time(NULL));

	read_buf = malloc(size);
	if(read_buf==NULL)
	{	
		printf("buffer read allocation err!\n");
		return -1;
	}

	write_buf =malloc(size);
	if(write_buf==NULL)
	{	
		printf("buffer write allocation err!\n");
		return -1;
	}
	for(i= 0; i <size; i++)
	{	
		write_buf[i]=rand()%255;
#ifdef DEBUG
		if(i % 16 == 0)
			printf("\n");
		printf("%02x ", write_buf[i]);
#endif
	}

	eeprom_write_enable(fd);
	eeprom_read_rdsr(fd);
	eeprom_write_enable(fd);
	eeprom_blk_erase(fd ,BLOCK_64K_ERASE ,addr);	//Block Erase 64KB
	eeprom_write_enable(fd);
#ifdef DEBUG  
	printf("write\n");
#endif
	eeprom_write(fd,addr,write_buf,size);	//Write Data
#ifdef DEBUG  
	printf("read\n");
#endif
	eeprom_read(fd,addr,read_buf,size);	//Read Data
#ifdef DEBUG  
	for(i= 0; i <size; i++)
	{	
		if(i % 16 == 0)
		printf("\n");
		printf("%02x ", read_buf[i]);
	}

#endif
	ret = memcmp(write_buf,read_buf,size);
	printf("\n\ncompare %s %d \n",ret ? "fail":"ok",ret);
	
	printf("\n");
 
	close(fd);
	
if(ret == 0)
	return 0;
else
	return -1;
}
