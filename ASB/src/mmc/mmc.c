#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sched.h>          /* schedule */
#include <sys/resource.h>
#include <linux/sched.h>    /* SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH */
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>       /* stat */
#include <sys/vfs.h>        /* statfs */
#include <errno.h>          /* error */
#include <sys/time.h>       /* gettimeofday() */
#include <sys/times.h>      /* struct tms */
#include <time.h>           /* ETIMEDOUT */
#include <mntent.h>
#include <sys/mount.h>

#include "gpio.h"

#define RETRYCNT				200;
#define KBYTE           (1024)
#define MBYTE           (1024 * KBYTE)
#define GBYTE           (1024 * MBYTE)


#define MMC1_DET_CON	PAD_GPIO_C + 26
#define MMC2_DET_CON	PAD_GPIO_C + 1 
#define MMC_MUX_SEL		PAD_GPIO_C + 4

#define MMC_SEL_OFF		0
#define MMC_SEL_CH1		1
#define MMC_SEL_CH2		2

#define DEVICE_PATH     "/dev/mmcblk1"



#if (0)
#define DBG_MSG(msg...)        { printf("[mmc test]: " msg); }
#else
#define DBG_MSG(msg...)        do{} while(0);
#endif
void print_usage(void)
{
	printf("usage: options\n"
		   " -h print this message\n"

		  );
}
static void mmc_sel(int ch)
{
	if(ch == MMC_SEL_CH1)
	{
		gpio_set_value(MMC_MUX_SEL , 0);
		gpio_set_value(MMC2_DET_CON, 1);
		gpio_set_value(MMC1_DET_CON, 0);
	}
	else if(ch == MMC_SEL_CH2 )
	{
		gpio_set_value(MMC_MUX_SEL , 1);
		gpio_set_value(MMC1_DET_CON, 1);
		gpio_set_value(MMC2_DET_CON, 0);
	}
	else if(ch == MMC_SEL_OFF)
	{
		gpio_set_value(MMC_MUX_SEL , 1);
		gpio_set_value(MMC1_DET_CON, 1);
		gpio_set_value(MMC2_DET_CON, 1);
	}
}

static int det_init(void)
{	
	if(gpio_export(MMC1_DET_CON))
	{	
		printf("init CD Det pin err\n");
		return -1;
	}	
	if(gpio_dir_out(MMC1_DET_CON))
	{
		printf("init CD Det pin err\n");
		return -1;
	}
	if(gpio_export(MMC2_DET_CON))
	{	
		printf("init CD Det pin err\n");
		return -1;
	}	
	if(gpio_dir_out(MMC2_DET_CON))
	{
		printf("init CD Det pin err\n");
		return -1;
	}
	if(gpio_export(MMC_MUX_SEL))
	{	
		printf("init MUX_SEL CON pin err\n");
		return -1;
	}	
	if(gpio_dir_out(MMC_MUX_SEL))
	{
		printf("init MUX_SEL CON pin err\n");
		return -1;
	}
	mmc_sel(MMC_SEL_OFF);
	return 0;
}

static int det_deinit(void)
{
	gpio_unexport(MMC1_DET_CON);
	gpio_unexport(MMC2_DET_CON);
	return 0;
}



static void mmc_insert(int ch)
{
	int cnt = RETRYCNT;
	mmc_sel(0);
	while((!access(DEVICE_PATH,F_OK))&&cnt--)
	{
		usleep(10000);
  }
  
  mmc_sel(ch);
  cnt = RETRYCNT;
	while(access(DEVICE_PATH,F_OK)&&cnt--)
	{
		usleep(10000);
	}
	
}

static mmc_rw_test(int fd, unsigned char *w, unsigned char *r, int size )
{
	int ret;
		
	fd = open(DEVICE_PATH,O_RDWR);
	if(fd <0)
	{
		printf("open err\n ");
	}
	lseek(fd,512,0);
	ret = write(fd,w,size);
	DBG_MSG("write ret = %d \n",ret);
	close(fd);

	fd = open(DEVICE_PATH,O_RDWR);
	lseek(fd,512,0);
	ret = read(fd,r,size);
	DBG_MSG("read ret = %d \n",ret);
	ret = memcmp(w,r,size);
	DBG_MSG("ret %x\n", ret);
	close(fd);
	return ret;
}

int main(int argc, char **argv)
{
	int opt,i, ret;
	unsigned char *wbuf = NULL;
	unsigned char *readbuf = NULL;
	int fd;
	int size = 1024;
	
	while(-1 != (opt = getopt(argc, argv, "hp:d:s:"))) {
		switch(opt) {
		 	 case 'h':   print_usage(); 	exit(0);	 
		}
	}

	det_init();
	wbuf = malloc(size);
	
	readbuf = malloc(size);
	if(wbuf == NULL  || readbuf== NULL)
	{
		printf("Alloc buffer error\n");
		if(wbuf)
		  free(wbuf);
		if(readbuf)
	   	free(readbuf);
		return -1;
}

	DBG_MSG("fill buffer\n");
	srand(time(NULL));

	for(i=0;i<size;i++)
	{
		wbuf[i] = rand()%0xff;
#if 0
		if(i% 16 ==0)
			printf("\n");
		printf("%2x ",wbuf[i] );
#endif

	}

	mmc_insert(1);
	ret = mmc_rw_test(fd,wbuf,readbuf,size);
	if(ret != 0)
	{
		printf("mmc 1 test fail\n");
		goto out;
	}
	
	mmc_insert(2);
	ret = mmc_rw_test(fd,wbuf,readbuf,size);
	if(ret != 0)
	{
		printf("mmc 2 test fail\n");
	}
	
out:
	printf("MMC TEST result %s\n", ret ? "fail":"ok");
	free(wbuf);
	free(readbuf);
	mmc_sel(0);
	det_deinit();
	
	return ret;
}
