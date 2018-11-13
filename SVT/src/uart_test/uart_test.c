#include "uart_test.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <errno.h>
#include <time.h>
#include <linux/kernel.h>

#define UART_MSG(msg...)        { printf("[Serial MSG] : " msg); }
#define UART_ERR(msg...)		{ printf("[Serial ERR] : " msg); }

#if (0)
#define UART_DBG(msg...)		{ printf("[Serial DBG]: " msg); }
#else
#define UART_DBG(msg...)		do{} while(0);
#endif

#define DUMP	0

#define TEST_SIZE	256
struct test_data
{
	unsigned char * pbuf;
	int 			fd;
};

static struct sig_param s_par;

int op_size = 256;

struct termios newtio;
struct termios oldtio;

//struct sigaction saio;           /* signal action의 정의 */

struct sig_param *par = &s_par;

/* CPU Utilization rate*/

pthread_t m_thread;
pthread_mutex_t mutex;
pthread_cond_t cond;

void print_usage(void)
{
	 printf("usage: options\n"
            " -p tty name                       , default %s 			\n"
            " -d tty name                       , default %s 			\n"
            " -C test counit                    , default 1 			\n"
            " -h print this message \n"
        	, TTY_NAME
        	, TTY_NAME
        	);
}

static int get_baudrate(int op_baudrate)
{
	int speed;

	switch (op_baudrate) {
		case 0    	: speed =       B0; break;
		case 50   	: speed =      B50; break;
		case 75   	: speed =      B75; break;
		case 110  	: speed =     B110; break;
		case 134  	: speed =     B134; break;
		case 150  	: speed =     B150; break;  
		case 200  	: speed =     B200; break;
		case 300  	: speed =     B300; break;
		case 600  	: speed =     B600; break;
		case 1200 	: speed =    B1200; break;
		case 1800 	: speed =    B1800; break;
		case 2400 	: speed =    B2400; break;
		case 4800 	: speed =    B4800; break;
		case 9600 	: speed =    B9600; break;
		case 19200	: speed =   B19200; break;
		case 38400	: speed =   B38400; break;
	  	case 57600  : speed =   B57600; break;
  		case 115200 : speed =  B115200; break;
  		case 230400 : speed =  B230400; break;
  		case 460800 : speed =  B460800; break;
  		case 500000 : speed =  B500000; break;
  		case 576000 : speed =  B576000; break;
  		case 921600 : speed =  B921600; break;
  		case 1000000: speed = B1000000; break;
  		case 1152000: speed = B1152000; break;
  		case 1500000: speed = B1500000; break;
  		case 2000000: speed = B2000000; break;
  		case 2500000: speed = B2500000; break;
  		case 3000000: speed = B3000000; break;
  		case 3500000: speed = B3500000; break;
  		case 4000000: speed = B4000000; break;
		default:
			printf("Fail, not support op_baudrate, %d\n", op_baudrate);
			return -1;
	}
	return speed;
}

unsigned int recivedata(int fd, unsigned char * pbuf, int bufsize,int readsize)
{
	int len;
	struct timeval tv;
	fd_set readfds;
	int ret;
	unsigned int idx=0;
	int size=0;
	int timeout = 1;
		
 	while(size < bufsize)
	{
		FD_ZERO (&readfds);
		FD_SET (fd, &readfds);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		len =0;

		ret = select(fd + 1, &readfds, NULL, NULL, &tv);
		       if (ret == -1) 
        {
           printf ("select(): error\n");
        } 
        else if (!ret) 
        {
        	printf ("%d seconds elapsed.\n", timeout );
        	return -1;
        }
        if (FD_ISSET (fd, &readfds)) 
        {
     
            len = read(fd, pbuf+idx, readsize);            
            if (len == 0) 
			{
				UART_DBG("NULL\n");
				continue;
			}
			if (0 > len) 
			{
				printf("Read error, %s\n", strerror(errno));
				break;
			}
			size += len;
			idx += len;
			
			if( (bufsize - size) < readsize )
				readsize = bufsize - size ;
		}
	}
	return 0;
}
int test_flag = 0;
void *write_test_thread(void *data)
{
	struct test_data * wdata =  (struct test_data *)data;
	unsigned char * tbuf = wdata->pbuf;
	unsigned int 	size,idx,sendsize;	
	int fd = wdata->fd;
	int ret;

	size = 256;
	
	while(test_flag==0);

	pthread_yield();

	sendsize = 16;
	size = 256;
	idx = 0;

	do{
		ret = write(fd,tbuf+idx,sendsize);
		if(ret <0)
		{
			printf("Write Err %d : %s \n",ret,strerror(errno));
		}
		size -= sendsize;
		idx  += sendsize;
		if(size < sendsize)
			sendsize = size;
	}while(size);
	
	UART_DBG("write end\n");		
	return 0;
}

int test_transfer(int fd, int mode,int sbaud,int ebaud, int fd1)
{
	int ret;
	unsigned char * rbuf=NULL;
	unsigned char * tbuf=NULL;
	int size,i;
		
	struct test_data data;
	
	size = TEST_SIZE;
	#if defined ( DUMP )
	int fd_d;		
	static char s[256] = {0, };
	#endif
	tbuf = (unsigned char *)malloc(size*2);

	UART_DBG("%s\n",__func__);		
	if(tbuf == NULL)
	{
		UART_ERR("Fail,aclloate tx buffer :  %d, %s\n",size , strerror(errno));
		goto _fail;
	}
	UART_DBG("tbuf=0x%p \n", tbuf);

	data.pbuf = tbuf;
	data.fd = fd;

	/* Make Test data for Tx */
	srand(time(NULL));
	
		for(i=0;i<size;i++)
		{
			tbuf[i]= rand()%255;

			if(tbuf[i] <22)
				tbuf[i] = tbuf[i]+22;

			if(tbuf[i] ==126 || tbuf[i] ==127)
				tbuf[i] = tbuf[i]+2;
			
			if(i%1024 == 1023 )	
			{
				tbuf[i] = 10;	
			}

			if(i == size -2)
			{
				tbuf[i] = '\r';
			}
			if(i == size -1)
			{
				tbuf[i] = '\n';
			}
		}
	

	/* READ BUFFER Allocate For Rx Test */
	rbuf = (unsigned char *)malloc(size* 2); 
	memset(rbuf,0,size*2);
	if(rbuf == NULL)
	{
		UART_ERR("Fail, malloc %d, %s\n",size , strerror(errno));
		goto _fail;
		
	}
	UART_DBG("rbuf=0x%p \n", rbuf);
	
	if(pthread_create(&m_thread,NULL, write_test_thread, (void *)&data) < 0)
	{
		UART_ERR("Fail Create Thread");
		goto _fail;
	}

	test_flag = 1;

	//	Wait Message Data for 3seconds
	ret = recivedata(fd1,rbuf,size, 256);
	if(!ret)
	{	
		UART_DBG("test end \n");
		UART_DBG("buf=0x%p:0x%p \n", tbuf,rbuf);
		
			ret = memcmp(tbuf, rbuf, size);
			UART_DBG("buf=0x%px0x%p \n", tbuf,rbuf);
			#if defined ( DUMP )
			if(ret)
			{	
				fd_d = open("/mnt/mmc0/uart_test.txt",O_RDWR | O_APPEND);
				for(i=0;i<size;i++)
				{
					UART_DBG("[%3d = 0x%02x  0x%02x ]\n",i, tbuf[i], rbuf[i]);
					sprintf(s,"[%3d = 0x%02x  0x%02x ]\n",i, tbuf[i], rbuf[i]);
					write(fd_d,s,strlen(s) );
				}
							
				close(fd_d);
				sync();
			}
			#endif
		
		UART_MSG(" Test ret = %d\n",ret);
	}

	pthread_join(m_thread,NULL);
	

	if(rbuf)
		free(rbuf);
	if(tbuf)
		free(tbuf);

if(ret == 0)
	return 0;
else 
	return -1;
 _fail:

 	if(rbuf)
		free(rbuf);
	if(tbuf)
		free(tbuf);
	
	pthread_cancel(m_thread);

	return -1;
}


int main (int argc, char **argv)
{
	int fd,fd1;
	int opt;
	speed_t speed;

	char ttypath[64] = TTY_NAME;
	char ttypath1[64] = TTY_NAME;
	char *pbuf = NULL;
	
	int op_baudrate = TTY_BAUDRATE;
	int op_canoical = NON_CANOICAL;				//Canonical mode Default non
	char op_mode = READ_MODE;
	//int op_test = TEST_MASTER;
	int op_charnum   = TTY_NC_CHARNUM;
	int op_timeout   = TTY_NC_TIMEOUT;
	int op_bufsize	 = 128;
	int op_flow		 = 0;
	int op_sbaud = 115200;
	int op_ebaud = 115200;
	int op_stop = 0;
	int op_parity = 0;
	int op_odd =0;
	int op_data = 8;
	int ret=0;
	int i=0,count=1;

	pthread_mutex_init( &mutex, NULL );
	pthread_cond_init( &cond, NULL );

	
	//while(-1 != (opt = getopt(argc, argv, "hp:b:m:ctafol:d:u:n:i:"))) {
	while(-1 != (opt = getopt(argc, argv, "hp:d:C:"))) {
		switch(opt) {
		case 'p':	strcpy(ttypath, optarg);			break;
		case 'd':	strcpy(ttypath1, optarg);			break;
		case 'C':	count = atoi(optarg) ;			break;
		case 'h':	print_usage() ; exit(0);			break;
        default:
             break;
        }
	}

	if(count == 0)
		count=1;
	pbuf = malloc(op_bufsize);
	if (NULL == pbuf) {
		printf("Fail, malloc %d, %s\n", op_bufsize, strerror(errno));
		return -1;
		exit(1);
	}

	fd = open(ttypath, O_RDWR| O_NOCTTY);	// open TTY Port
	fd1 = open(ttypath1, O_RDWR| O_NOCTTY);	// open TTY Port

	if (0 > fd) {
  		printf("Fail, open '%s', %s\n", ttypath, strerror(errno));
  		free(pbuf);
  		return -1;
  		//exit(1);
 	}

 	/*
	 * save current termios
	 */
	tcgetattr(fd, &oldtio);

	par->fd = fd;
 	memcpy(&par->tio, &oldtio, sizeof(struct termios));

 	speed = get_baudrate( op_baudrate);

 	if(!speed)
 		goto _exit;
	memcpy(&newtio, &oldtio, sizeof(struct termios));

 	newtio.c_cflag &= ~CBAUD;	// baudrate mask
 	newtio.c_iflag 	&= ~ICRNL;
	newtio.c_cflag |=  speed; 	
	
	newtio.c_cflag &=   ~CS8;
	switch (op_data)
	{	
		case 5:
			newtio.c_cflag |= CS5; 
			break;
		case 6:
			newtio.c_cflag |= CS6;
			break;
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
		default :
			printf("Not Support  %d data len, Setting 8bit Data len. \n",op_data);
			newtio.c_cflag |= CS8;
			break;
	}

	newtio.c_iflag 	|= IGNBRK|IXOFF;

	newtio.c_cflag 	&= ~HUPCL;
	if(op_flow)
		newtio.c_cflag |= CRTSCTS;   /* h/w flow control */
    else
    	newtio.c_cflag &= ~CRTSCTS;  /* no flow control */
  
	newtio.c_iflag 	|= 0;	// IGNPAR;
	newtio.c_oflag 	|= 0;	// ONLCR = LF -> CR+LF
	newtio.c_oflag 	&= ~OPOST;

	newtio.c_lflag 	= 0;
	newtio.c_lflag 	= op_canoical ? newtio.c_lflag | ICANON : newtio.c_lflag & ~ICANON;	// ICANON (0x02)
	
	newtio.c_cflag	= op_stop ? newtio.c_cflag | CSTOPB : newtio.c_cflag & ~CSTOPB;	//Stop bit 0 : 1 stop bit 1 : 2 stop bit
	
	newtio.c_cflag	= op_parity ? newtio.c_cflag | PARENB : newtio.c_cflag & ~PARENB; //0: No parity bit, 1: Parity bit Enb , 
	newtio.c_cflag	= op_odd ? newtio.c_cflag | PARODD : newtio.c_cflag & ~PARODD; //0: No parity bit, 1: Parity bit Enb , 


	if (!(ICANON & newtio.c_lflag)) {
	 		newtio.c_cc[VTIME] 	= op_timeout; 	// time-out °ªÀ¸·Î »ç¿ëµÈ´Ù. time-out = TIME* 100 ms,  0 = ¹«ÇÑ
	 		newtio.c_cc[VMIN] 	= op_charnum; 	// MINÀº read°¡ ¸®ÅÏµÇ±â À§ÇÑ ÃÖ¼ÒÇÑÀÇ ¹®ÀÚ °³¼ö.
	}
	
	//dump_termios(1, &oldtio, "OLD TERMIOS");
	//dump_termios(1, &newtio, "NEW TERMIOS");
	for(i=0;i<count;i++){
		tcflush  (fd, TCIFLUSH);
		tcsetattr(fd, TCSANOW, &newtio);
	
		tcflush  (fd1, TCIFLUSH);
		tcsetattr(fd1, TCSANOW, &newtio);
		//show_termios(&newtio, ttypath ,stdout );
	
		ret = test_transfer(fd,op_mode,op_sbaud,op_ebaud ,fd1);
		UART_DBG("ret = %d \n",ret);
		if(ret < 0)
			goto _exit;
		
		tcflush  (fd, TCIFLUSH);
	 	tcsetattr(fd, TCSANOW, &oldtio);
		tcflush  (fd1, TCIFLUSH);
	 	tcsetattr(fd1, TCSANOW, &oldtio);
	
		close(fd);
		close(fd1);
		
		fsync(fd);
		fsync(fd1);
		fd = open(ttypath, O_RDWR| O_NOCTTY);	// open TTY Port
		fd1 = open(ttypath1, O_RDWR| O_NOCTTY);	// open TTY Port
	
		tcflush  (fd, TCIFLUSH);
		tcsetattr(fd, TCSANOW, &newtio);
	
		tcflush  (fd1, TCIFLUSH);
		tcsetattr(fd1, TCSANOW, &newtio);
	
		ret = test_transfer(fd1,op_mode,op_sbaud,op_ebaud ,fd);
		UART_DBG("ret = %d \n",ret);
		printf("test result %s , try %d \n", ret ? "fail" : "ok", i);
	}
	goto _exit;

_exit:
	printf("[EXIT '%s']\n", ttypath);

	/*
	 * restore old termios
	 */
	tcflush  (fd, TCIFLUSH);
 	tcsetattr(fd, TCSANOW, &oldtio);
	tcflush  (fd1, TCIFLUSH);
 	tcsetattr(fd1, TCSANOW, &oldtio);
	if(fd)
		close(fd);

	if(fd1)
		close(fd1);

	if (pbuf)
		free(pbuf);	

	printf("===> ret = %d\n", ret );
	if( ret < 0 )
		exit(-1);

	return ret;
}


