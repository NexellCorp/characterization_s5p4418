#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h> 			/* error */

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/*
 * mknod /dev/i2c-0 c 89 0
 */

//--------------------------------------------------------------------------------
#if 0
/* i2c.h */
struct i2c_msg {
	__u16 					addr;	/* slave address			*/
	__u16 					flags;

#define I2C_M_TEN			0x0010	/* this is a ten bit chip address */
#define I2C_M_RD			0x0001	/* read data, from slave to master */
#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */

	__u16 					size;	/* msg length				*/
	__u8 				*	data;	/* pointer to msg data			*/
};

/* i2c-dev.h */
struct i2c_rdwr_ioctl_data {
	struct i2c_msg __user *	msgs;	/* pointers to i2c_msgs */
	__u32 					nmsgs;	/* number of i2c_msgs */
};
#endif

//--------------------------------------------------------------------------------
unsigned int atoh(char *a)
{
	char ch = *a;
	unsigned int cnt = 0, hex = 0;

	while(((ch != '\r') && (ch != '\n')) || (cnt == 0))	{
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

//--------------------------------------------------------------------------------
void print_usage(void)
{
    printf( "usage: options\n"
            "-w	write, default read 		\n"
            "-p	port i2c-n, default i2c-0 	\n"
            "-a	HEX address					\n"
            "-r	register   					\n"
            "-n	no stop	(when read)	 		\n"
            "-i	ignore nak			 		\n"
            "-C	loop count			 		\n"
            "-T	Test Time not support 		\n"
    		"-m mode select, 0 = W: addr/data						\n"
    		"                    R: addr/data						\n"
    		"                1 = W: addr/reg/data					\n"
    		"                    R: addr/reg     [s] addr/data		\n"
    		"                2 = W: addr/reg/data/data				\n"
    		"                    R: addr/reg     [s] addr/data/data	\n"
    		"                3 = W: addr/reg/reg/data/data			\n"
    		"                    R: addr/reg/reg [s] addr/data/data	\n"
            );
}

int main(int argc, char **argv)
{
  	int  fd = 0, opt;
  	char path[20];

  	int   port   =  0; 				/* 0, 1, 2 */
	char  addr   =  0; 				/* The I2C slave address */
	int   w_mode =  0;
	int   n_stop =  0;
	int   mode   = -1;
	int	  count  =  1;
	int	  ignore =  0;
	int   verify =  0;
  	short reg = -1, dat = -1;
  	int   w_len = 0, r_len = 0, i = 0;
  	char  buf[256];
	int   readval = 0;
	int   ret = -1;
	/* ioctl rw data */
 	struct i2c_rdwr_ioctl_data rw_arg;
 	struct i2c_msg ctl_msgs[2];

 	rw_arg.msgs = ctl_msgs;

    while (-1 != (opt = getopt(argc, argv, "hw:p:a:r:m:niv:C:T:"))) {
		switch(opt) {
        case 'h':   print_usage();  exit(0);   	break;
        case 'w':   w_mode = 1;
        			dat    = atoh(optarg); 		break;
        case 'p':   port   = atoi(optarg);		break;
        case 'a':   addr   = atoh(optarg);		break;
        case 'r':   reg    = atoh(optarg);		break;
		case 'm':   mode   = atoi(optarg);		break;
		case 'C':   count  = atoi(optarg);		break;
		case 'n':   n_stop = 1;					break;
		case 'i':   ignore = 1;					break;
		case 'v':   verify = atoh(optarg); 		break;
        default:
        	break;
         }
	}

	//----------------------------------------------------------------------------
	/* check input params */
	if (-1 == mode){
    	printf("no i2c-%d invalid mode %d \n", port, mode);
    	print_usage();
		goto __end;
	}

	if (!mode && -1 == reg) {
    	printf("no i2c-%d register	\n", port);
    	print_usage();
		goto __end;
	}

	if (0 >= count)
		count = 1;

	//----------------------------------------------------------------------------
  	snprintf(path, 19, "/dev/i2c-%d", port);
  	fd = open(path, O_RDWR, 644);	// O_RDWR, O_RDONLY
  	if (0 > fd) {
    	printf("fail, %s open, %s\n", path, strerror(errno));
    	exit(1);
  	}

	//----------------------------------------------------------------------------
	if (w_mode) {

		for (i = 0; count > i; i++) {

			if (0 == mode) {
				printf("[%s],[%s] I2C-%d, ADDR=0x%02x, DATA=0x%02x\n",
					w_mode ? "WRITE" : "READ", n_stop?"NO STOP":"Nomal",
					port, addr, dat&0xFF);

				buf[0] = (dat >> 0) & 0xFF;
				w_len  = 1;
			}

			if (1 == mode) {
				printf("[%s],[%s] I2C-%d, ADDR=0x%02x, REG=0x%02x, DATA=0x%02x\n",
					w_mode ? "WRITE" : "READ", n_stop?"NO STOP":"Nomal",
					port, addr, reg&0xFF, dat&0xFF);

				buf[0] = (reg >> 0) & 0xFF;
				buf[1] = (dat >> 0) & 0xFF;
				w_len  = 2;
			}

			if (2 == mode) {
				printf("[%s],[%s] I2C-%d, ADDR=0x%02x, REG=0x%02x, DATA=0x%02x%02x\n",
					w_mode ? "WRITE" : "READ", n_stop?"NO STOP":"Nomal",
					port, addr, (reg>>0)&0xFF, (dat>>8)&0xFF, (dat>>0)&0xFF);

				buf[0] = (reg >> 0) & 0xFF;
				buf[1] = (dat >> 8) & 0xFF;
				buf[2] = (dat >> 0) & 0xFF;
				w_len  = 3;
			}

			if (3 == mode) {
				printf("[%s],[%s] I2C-%d, ADDR=0x%02x, REG=0x%04x, DATA=0x%02x%02x\n",
					w_mode ? "WRITE" : "READ", n_stop?"NO STOP":"Nomal",
					port, addr, reg, (dat>>8)&0xFF, (dat>>0)&0xFF);

				buf[0] = (reg >> 8) & 0xFF;
				buf[1] = (reg >> 0) & 0xFF;
				buf[2] = (dat >> 8) & 0xFF;
				buf[3] = (dat >> 0) & 0xFF;
				w_len  = 4;
			}

			/*
 	 	 	 *	write : 1 step = <W> [ADDR, REG, DAT(MSB), DAT(LSB)...]
	 	 	 */
 			rw_arg.nmsgs 			= 1;

			/* 1 setp : write [addr, reg] */
 			rw_arg.msgs[0].addr 	= (addr>>1);
 			rw_arg.msgs[0].flags	= !I2C_M_RD;		/* I2C_M_RD = write */
			if(ignore)
// 			rw_arg.msgs[0].flags	|= I2C_M_IGNORE_NAK;		/* I2C_M_RD = write */
 			rw_arg.msgs[0].len   	= w_len;				/* data size = { REG(MSB), REG(LSB), DAT(MSB), DAT(LSB) } */
 			rw_arg.msgs[0].buf 		= (__u8 *)buf;

			if (0 > ioctl(fd, I2C_RDWR,	&rw_arg)) {
				printf("Fail, %s IoCtl(0x%x), %s\n", path, I2C_RDWR, strerror(errno));
				continue;
 			}
			usleep(100000);
 		}

	} else {

		for (i = 0; count > i; i++) {

			if (0 == mode) {
				printf("[%s],[%s] I2C-%d, ADDR=0x%02x, DAT=0x%02x ",
					w_mode ? "WRITE" : "READ", n_stop?"NO STOP":"Nomal", port, addr, (dat>>0)&0xFF);

				rw_arg.nmsgs		= 1;

				/* 1 setp : read [ADDR, DAT] */
				buf[0] = 0;
				w_len  = 0;
				r_len  = 1;
			}

			if (1 == mode) {
				printf("[%s],[%s] I2C-%d, ADDR=0x%02x, REG=0x%02x ",
					w_mode ? "WRITE" : "READ", n_stop?"NO STOP":"Nomal", port, addr, (reg>>0)&0xFF);

				/*
 	 		 	 *	read  : 2 step = <W> [ADDR, REG],
	 		 	 *					 <R> [ADDR, DAT]
	    	 	 */
				rw_arg.nmsgs		= 2;

				/* 1 setp : write [ADDR, REG] */
				buf[0] = (reg >> 0) & 0xFF;
				w_len  = 1;
				r_len  = 1;
			}

			if (2 == mode) {
				printf("[%s],[%s] I2C-%d, ADDR=0x%02x, REG=0x%04x ",
					w_mode ? "WRITE" : "READ", n_stop?"NO STOP":"Nomal", port, addr, (reg>>0)&0xFF);

				/*
 	 		 	 *	read  : 2 step = <W> [ADDR, REG],
	 		 	 *					 <R> [ADDR, DAT(MSB), DAT(LSB)]
	    	 	 */
				rw_arg.nmsgs		= 2;

				/* 1 setp : write [ADDR, REG] */
				buf[0] = (reg >> 0) & 0xFF;
				w_len  = 1;
				r_len  = 2;
			}

			if (3 == mode) {
				printf("[%s],[%s] I2C-%d, ADDR=0x%02x, REG=0x%04x ",
					w_mode ? "WRITE" : "READ", n_stop?"NO STOP":"Nomal", port, addr, reg);

				/*
 	 		 	 *	read  : 2 step = <W> [ADDR, REG(MSB), REG(LSB)],
	 		 	 *					 <R> [ADDR, DAT(MSB), DAT(LSB)]
	    	 	 */
				rw_arg.nmsgs		= 2;

				/* 1 setp : write [ADDR, REG(MSB), REG(LSB)] */
				buf[0] = (reg >> 8) & 0xFF;
				buf[1] = (reg >> 0) & 0xFF;
				w_len  = 2;
				r_len  = 2;
			}

			if (1 == rw_arg.nmsgs) {
				/* 2 setp : read [ADDR, DAT(MSB), DAT(LSB)] */
				rw_arg.msgs[0].addr  = (addr>>1);
				rw_arg.msgs[0].flags = I2C_M_RD;		/* I2C_M_RD = read */
			if(ignore)
// 			rw_arg.msgs[0].flags	|= I2C_M_IGNORE_NAK;		/* I2C_M_RD = write */
 				rw_arg.msgs[0].len   = r_len;			/* data size */
 				rw_arg.msgs[0].buf 	 = (__u8 *)buf;
			}

			if (2 == rw_arg.nmsgs) {
 				rw_arg.msgs[0].addr	 = (addr>>1);
 				rw_arg.msgs[0].flags = !I2C_M_RD;						/* I2C_M_RD = write */
// 				rw_arg.msgs[0].flags|= (n_stop ? I2C_M_NOSTART : 0);	/* write and no stop */
			if(ignore)
 			rw_arg.msgs[0].flags	|= I2C_M_IGNORE_NAK;		/* I2C_M_RD = write */
 				
				rw_arg.msgs[0].len   = w_len;							/* data size = reg */
	 			rw_arg.msgs[0].buf 	 = (__u8 *)buf;

				/* 2 setp : read [ADDR, DAT(MSB), DAT(LSB)] */
				rw_arg.msgs[1].addr  = (addr>>1);
				rw_arg.msgs[1].flags = I2C_M_RD;		/* I2C_M_RD = read */
 				rw_arg.msgs[1].len   = r_len;			/* data size */
 				rw_arg.msgs[1].buf 	 = (__u8 *)buf;
			}

			if (0 > ioctl(fd, I2C_RDWR,	&rw_arg)) {
				printf("Fail, %s IoCtl(0x%x), %s\n", path, I2C_RDWR, strerror(errno));
				if (fd)
					close(fd);
				return -1;
 			}

			if (2 == r_len) {
				dat = (buf[0]<<8 & 0xFF00) | (buf[1] & 0xFF);
				printf("DATA=0x%04x\n", dat & 0xFFFF);
			} else {
				dat = buf[0];
				printf("DATA=0x%02x\n", dat & 0xFF);
			}

			readval  = ( dat & 0x0000ffff );
			if (verify == 0) {
				ret = 0;
				continue;
			}	

			if(readval  == verify)
			{
				printf(" read : %x verify: %x is same.\n\n", dat, verify);
				ret =  0;
			}
			else{
				printf("err!  read :%x verify : %x \n", dat, verify);
				return -1;
			}
			usleep(100000);
		}
 	}
	
__end:
	if (fd)
		close(fd);

  	return ret;
}
