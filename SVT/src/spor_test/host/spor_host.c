#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

#include <termios.h>
#include <linux/serial.h> 
#include <fcntl.h>

#include <sys/ioctl.h>

#include <ctype.h>

#include "uart.h"
#include "loglevel.h"
#include "spor_test.h"

/* Power Control */
#define PORT_CONTROL		"/dev/ttyUSB1"
/* Message Port */
#define PORT_MESSAGE		"/dev/ttyUSB3"

#define UNKNOWN_COUNT_MAX		10
#define SPOR_TEST_MAX			1000




/********************************************************
 * TERMINAL CONFIG
 ********************************************************/

struct termios newtio;
struct termios oldtio;

char port_control[128] = PORT_CONTROL;
char port_message[128] = PORT_MESSAGE;

int op_baudrate	= TTY_BAUDRATE;
int op_canoical	= NON_CANOICAL;				//Canonical mode Default non
//char op_mode	= READ_MODE;
//int op_test	= TEST_MASTER;
int op_charnum	= TTY_NC_CHARNUM;
int op_timeout	= TTY_NC_TIMEOUT;
int op_bufsize	= 128;
int op_flow		= 0;
int op_stop		= 0;
int op_parity	= 0;
int op_odd		= 0;
int op_data		= 8;
int ret			= 0;



char msgbuf[1024] = { 0, };


/********************************************************
 * TARGET
 ********************************************************/
/* target status */
#define TS_UNKNOWN		-1
#define TS_INIT			0
#define TS_BOOTING		1
#define TS_POWERON		2
#define TS_POWEROFF		3
#define TS_CHECKING		4
#define TS_TESTING		5
#define TS_FAILED		6


struct target {
	int fd;
	int status;
};

#define INIT_TARGET()		\
{							\
	.fd = -1,				\
	.status = TS_UNKNOWN	\
}


struct target target = INIT_TARGET();



/*
 * Setting UART DTR pin.
 *
 * on:
 *	0    - clear
 *	else - set
 */
int setdtr(int fd, int on)
{
	int controlbits = TIOCM_DTR;

	if (on) {
		nx_info("[-->TARGET] Setting DTR\n");
		return (ioctl(fd, TIOCMBIS, &controlbits));
	}
	else {
		nx_info("[-->TARGET] Clearing DTR\n");
		return (ioctl(fd, TIOCMBIC, &controlbits));
	}

	return 0;
} 


int target_init(void)
{
	target.fd = -1;
	target.status = TS_INIT;

	return 0;
}


int target_poweron(void)
{
	target.fd = open(port_control, O_WRONLY);
	if (target.fd < 0)
	{
		fprintf(stderr,"Couldn't open %s\n", port_control);
		return -1;
	}

	setdtr(target.fd, 1);
	target.status = TS_BOOTING;

	return 0;
}

int target_poweroff(void)
{
	/* need target.status check? */

	target.status = TS_POWEROFF;
	setdtr(target.fd, 0);

	close(target.fd);
	target.fd = -1;

	return 0;
}



/********************************************************
 * TERMINAL
 ********************************************************/

int open_term(void)
{
	int fd;
	speed_t speed;


	fd = open(port_message, O_RDWR | O_NOCTTY);	// open TTY Port

	if (0 > fd) {
  		nx_error("Fail, open '%s', %s\n", port_message, strerror(errno));
  		return -1;
 	}

 	/*
	 * save current termios
	 */
	tcgetattr(fd, &oldtio);

 	speed = get_baudrate(op_baudrate);
	if (!speed) {
		nx_error("Fail, get baudrate\n");
		close(fd);
		return -1;
	}

	memcpy(&newtio, &oldtio, sizeof(struct termios));

 	newtio.c_cflag &= ~CBAUD;	// baudrate mask
 	newtio.c_iflag &= ~ICRNL;
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
			nx_info("Not Support  %d data len, Setting 8bit Data len. \n",op_data);
			newtio.c_cflag |= CS8;
			break;
	}

//	newtio.c_iflag 	|= IGNBRK|IXOFF;
	newtio.c_iflag 	|= 0;

	newtio.c_cflag 	&= ~HUPCL;
	if(op_flow)
		newtio.c_cflag |= CRTSCTS;   /* h/w flow control */
    else
    	newtio.c_cflag &= ~CRTSCTS;  /* no flow control */
  
	newtio.c_iflag 	|= IGNPAR;	// IGNPAR;
	newtio.c_oflag 	|= 0;	// ONLCR = LF -> CR+LF
	newtio.c_oflag 	&= ~OPOST;

	newtio.c_lflag 	= 0;
	newtio.c_lflag 	= op_canoical ? newtio.c_lflag | ICANON : newtio.c_lflag & ~ICANON;	// ICANON (0x02)
	
	newtio.c_cflag	= op_stop ? newtio.c_cflag | CSTOPB : newtio.c_cflag & ~CSTOPB;	//Stop bit 0 : 1 stop bit 1 : 2 stop bit
	
	newtio.c_cflag	= op_parity ? newtio.c_cflag | PARENB : newtio.c_cflag & ~PARENB; //0: No parity bit, 1: Parity bit Enb , 
	newtio.c_cflag	= op_odd ? newtio.c_cflag | PARODD : newtio.c_cflag & ~PARODD; //0: No parity bit, 1: Parity bit Enb , 


	if (!(ICANON & newtio.c_lflag)) {
	 		newtio.c_cc[VTIME] 	= op_timeout;
	 		newtio.c_cc[VMIN] 	= op_charnum;
	}
	

	fsync(fd);

	tcflush  (fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	
	return fd;
}

int close_term(int fd)
{
	nx_info("[SPOR] EXIT '%s'\n", port_message);

	tcflush  (fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &oldtio);

	fsync(fd);
	close(fd);

	return 0;
}







/********************************************************
 * DUMP
 ********************************************************/
#if 0
static int line_len = 0;

void dump(char *msgbuf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		if (isascii(msgbuf[i]))
			printf("%c", msgbuf[i]);
		else
			printf(".");

		//line_len++;
		//if ((line_len+1) % 16 == 0)
		//	printf("\n");
	}
}
#endif


int receivedata(int fd, char *msgbuf, int bufsize)
{
	int len = 0;
	//struct timeval tv;
	fd_set readfds;
	int ret = -1;


	while (1)		/* msg */
	{
		int sel_r;

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		//tv.tv_sec = timeout;
		//tv.tv_usec = 0;
		len = 0;

		//sel_r = select(fd + 1, &readfds, NULL, NULL, &tv);
		sel_r = select(fd + 1, &readfds, NULL, NULL, NULL);
		if (sel_r == -1) 
		{
			nx_debug("select(): error\n");
			break;
		} 
		else if (!sel_r) 
		{
			//nx_info("%d seconds elapsed.\n", timeout);
			continue;
		}

		if (FD_ISSET(fd, &readfds)) 
		{
			memset(msgbuf, 0x00, bufsize);
			len = read(fd, msgbuf, 1);
			if (len == 0)
			{
				continue;
			}
			else if (len < 0)
			{
				nx_debug("Read error, %s\n", strerror(errno));
				break;
			}
			else
			{
				ret = 0;
				//nx_info("read len:%d\n", len);
				//dump(msgbuf, len);
				break;
			}
		}
	}

	return ret;
}


int spor_test(void)
{
	int cnt = 0;
	//int unknown_cnt = 0;

	while (1)
	{
		int ret;
		int fd;

		nx_info("[SPOR] Test count: %d\n", ++cnt);

		target_init();


		nx_info("[SPOR] target power on ...\n");
		target_poweron();

		/**
		 * need delay until power is stable
		 **/
		sleep(3);

		fd = open_term();
		if (fd < 0)
		{
			nx_error("open terminal failed\n");
			return -1;
		}


		do {
			tcflush  (fd, TCIFLUSH);
			ret = receivedata(fd, msgbuf, 256);
			if (ret < 0)
				continue;

			/**
			 * target communication
			 *     status change...
			 **/

			if (msgbuf[0] == POWER_ALIVE)
			{
				nx_info("[<--TARGET] POWER:ALIVE\n");
				target.status = TS_POWERON;
			}
			else if (msgbuf[0] == CHECK_START)
			{
				nx_info("[<--TARGET] CHECK:START\n");
				target.status = TS_CHECKING;
			}
			else if (msgbuf[0] == CHECK_SUCCESS)
			{
				nx_info("[<--TARGET] CHECK:SUCCESS\n");
				target.status = TS_TESTING;
				break;
			}
			else if (msgbuf[0] == CHECK_FAILURE)
			{
				nx_info("[<--TARGET] CHECK:FAILED\n");
				target.status = TS_FAILED;
				break;
			}
			else
			{
				nx_error("[<--TARGET] UNKNOWN\n");
				target.status = TS_UNKNOWN;
				break;
			}
		} while(1);

		if (target.status == TS_TESTING)
		{
			sleep(20);
		}
		else if (target.status == TS_FAILED)
		{
			/* origin data broken. stop testing. */ 
			break;
		}
		else if (target.status == TS_UNKNOWN)
		{
			//if (++unknown_cnt < UNKNOWN_COUNT_MAX)
				continue;
		}

		close_term(fd);

		nx_info("[SPOR] target power off ...\n");
		target_poweroff();

		sleep(5);
	}

	return 0;
}



void print_usage(void)
{
	nx_error("Usage: ./PROGRAM -c /dev/ttyUSB# -p /dev/ttyUSB#\n");
	nx_error("                 -c control port\n");
	nx_error("                 -m message port\n");
}


int main(int argc, char **argv)
{
	int opt;

	
	while(-1 != (opt = getopt(argc, argv, "hc:m:"))) {
		switch(opt) {
		case 'c':	sprintf(port_control, "%s", optarg);	break;
		case 'm':	sprintf(port_message, "%s", optarg);	break;
		case 'h':	print_usage();  exit(0);				break;
        default:
             break;
        }
	}
	nx_debug("[SPOR] open message port : %s\n", port_message);
	nx_debug("[SPOR]      control port : %s\n", port_control);


	///////////////////////////////////////////////////////////////////////
	//
	// SPOR Test Start
	//
	///////////////////////////////////////////////////////////////////////

	spor_test();


	return 0;
}
