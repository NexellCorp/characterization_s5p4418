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
#include "nx_lib.h"
#include "spor_test.h"

#define TTY_NAME "/dev/ttyAMA4"
#define DEFAULT_BOOT_WAITING		25

/* TODO - parsing automatically version.c */
#define VERSION_STRING	"2015.03.08 v1.0"



static struct sig_param s_par;

struct termios newtio;
struct termios oldtio;

struct sig_param *par = &s_par;


char msgbuf[1024] = { 0, };


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

void print_usage(void)
{
	nx_error("Usage: ./PROGRAM -p /dev/ttyAMA# or /dev/ttySAC#\n");
	nx_error("        [-m message port]           (default:%s\n", TTY_NAME);
	nx_error("        [-k boot waiting seconds]   (default:%s\n", DEFAULT_BOOT_WAITING);
	nx_error("        [-h]                        (This message)\n");
}


int system_cmd(const char *cmd)
{
	FILE *fp;
	char str[1024];
	int ret;
	int status;


	fp = nx_popen(cmd, "r");
	if (!fp)
	{
		fprintf(stderr,
				"incorrect parameters or too many files.\n");
		return (EXIT_FAILURE);
	}

	while(!feof(fp))
	{
		if (fgets(str, sizeof(str), fp) == NULL)
			break;
		//printf("%s", str);
	}

	ret = 0;

	status = nx_pclose(fp);
	if (WIFEXITED(status))
	{
		//nx_debug("Exit!\n");
		ret = WEXITSTATUS(status);
	}
	else if (WIFSIGNALED(status))
	{
		nx_debug("Signaled! (%d)\n", WTERMSIG(status));
	}

	nx_debug("ret:%d\n", (char)ret);

	return ret;
}



int write_msg(int fd, char *msg)
{
	int ret;
	char status = 0;

	if (!strcmp(msg, spor_msg[POWER_ALIVE]))
		status = POWER_ALIVE;
	else if (!strcmp(msg, spor_msg[CHECK_START]))
		status = CHECK_START;
	else if (!strcmp(msg, spor_msg[CHECK_SUCCESS]))
		status = CHECK_SUCCESS;
	else if (!strcmp(msg, spor_msg[CHECK_FAILURE]))
		status = CHECK_FAILURE;
	else if (!strcmp(msg, spor_msg[TEST_START]))
		status = TEST_START;
	else if (!strcmp(msg, spor_msg[TEST_SUCCESS]))
		status = TEST_SUCCESS;
	else if (!strcmp(msg, spor_msg[TEST_FAILURE]))
		status = TEST_FAILURE;

	ret = write(fd, &status, 1);
	if (ret < 0)
	{
		nx_error("write error\n");
		return -1;
	}

	return 0;
}

int run_test(int fd)
{
	int ret;

	tcflush  (fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);


	/* avoid garbage data sending */
	usleep(100000);

	
	write_msg(fd, spor_msg[POWER_ALIVE]);
	write_msg(fd, spor_msg[CHECK_START]);

	//for logic testing// ret = system_cmd("sleep 2");
	//ret = system_cmd("dt -r -c 1 -p /data/org");
	ret = system_cmd("dt -r -c 10 -p /data/org");
	if (ret == EXIT_SUCCESS)
	{
		nx_debug("[SPOR] origin data test passed.\n");
		write_msg(fd, spor_msg[CHECK_SUCCESS]);
	}
	else
	{
		nx_debug("[SPOR] origin data test failed.\n");
		write_msg(fd, spor_msg[CHECK_FAILURE]);

		return -1;
	}


	nx_debug("[SPOR] new data test start.\n");
	write_msg(fd, spor_msg[TEST_START]);

	tcflush  (fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &oldtio);

	//for logic testing// ret = system_cmd("sleep 5");
	//ret = system_cmd("dt -r -w -c 1 -p /data/new");
	ret = system_cmd("dt -r -w -c 20 -p /data/new");
	if (ret == EXIT_SUCCESS)
	{
		nx_debug("[SPOR] new data test passed.\n");
		write_msg(fd, spor_msg[TEST_SUCCESS]);
	}
	else
	{
		nx_debug("[SPOR] new data test failed.\n");
		write_msg(fd, spor_msg[TEST_FAILURE]);
	}


	tcflush  (fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &oldtio);

	fsync(fd);
	close(fd);

	return 0;
}


int main(int argc, char **argv)
{
	int fd;
	int opt;
	speed_t speed;

	char ttypath[128] = TTY_NAME;
	char *pbuf = NULL;

	int boot_waiting = DEFAULT_BOOT_WAITING;
	
	int op_baudrate	= TTY_BAUDRATE;
	int op_canoical	= NON_CANOICAL;				//Canonical mode Default non
	int op_charnum	= TTY_NC_CHARNUM;
	int op_timeout	= TTY_NC_TIMEOUT;
	int op_bufsize	= 128;
	int op_flow		= 0;
	int op_stop		= 0;
	int op_parity	= 0;
	int op_odd		= 0;
	int op_data		= 8;
	int i			= 0;


	nx_debug("[SPOR] run SPOR test target program.\n");
	nx_debug("[SPOR]     version: %s\n", VERSION_STRING);

	while (-1 != (opt = getopt(argc, argv, "hm:k:"))) {
		switch(opt) {
		case 'm':	sprintf(ttypath, "%s", optarg);		break;
		case 'h':	print_usage() ; exit(0);			break;
		case 'k':	boot_waiting = atoi(optarg);		break;
        default:
             break;
        }
	}

	nx_debug("[SPOR] open message port : %s\n", ttypath);


	pbuf = malloc(op_bufsize);
	if (NULL == pbuf) {
		nx_error("Fail, malloc %d, %s\n", op_bufsize, strerror(errno));
		return -1;
		exit(1);
	}

	fd = open(ttypath, O_RDWR| O_NOCTTY);	// open TTY Port

	if (0 > fd) {
  		nx_error("Fail, open '%s', %s\n", ttypath, strerror(errno));
  		free(pbuf);
  		return -1;
 	}

	///////////////////////////////////////////////////////////////////////
	//
	// Waiting for booting up...
	//
	///////////////////////////////////////////////////////////////////////
	if (boot_waiting < 0)
		boot_waiting = DEFAULT_BOOT_WAITING;
	nx_debug("[SPOR] Sleeping %d seconds...\n", boot_waiting);
	for(i = 0; i < boot_waiting; i++) {
		nx_debug(".");
		fflush(stdout);
		sleep(1);
	}
	nx_debug("\n");


 	/*
	 * save current termios
	 */
	tcgetattr(fd, &oldtio);

	par->fd = fd;
 	memcpy(&par->tio, &oldtio, sizeof(struct termios));

 	speed = get_baudrate(op_baudrate);
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
			nx_info("Not Support  %d data len, Setting 8bit Data len. \n",op_data);
			newtio.c_cflag |= CS8;
			break;
	}

	//newtio.c_iflag 	|= IGNBRK|IXOFF;
	newtio.c_iflag 	|= 0;

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
	 		newtio.c_cc[VTIME] 	= op_timeout;
	 		newtio.c_cc[VMIN] 	= op_charnum;
	}


	///////////////////////////////////////////////////////////////////////
	//
	// RUN TEST
	//
	///////////////////////////////////////////////////////////////////////
	run_test(fd);

	
	nx_info("[EXIT '%s']\n", ttypath);

	/*
	 * restore old termios
	 */
	tcflush  (fd, TCIFLUSH);
 	tcsetattr(fd, TCSANOW, &oldtio);
	if(fd)
		close(fd);

	if (pbuf)
		free(pbuf);	

	return 0;
}
