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


#define KBYTE           (1024)
#define MBYTE           (1024 * KBYTE)
#define GBYTE           (1024 * MBYTE)



#define DISK_PATH       "/mnt/mmc"
#define DEVICE_PATH     "/dev/mmcblk0p1"

void print_usage(void)
{
	printf("usage: options\n"
		   " -d device node				default : %s\n" 
		   " -p directory path		    default : %s\n" 
		   " -C Test Count			    default : 1 \n" 
		   " -h print this message\n"
		   , DEVICE_PATH
		   , DISK_PATH 

		  );
}



char *path   = DISK_PATH;
char *device = "/dev/mmcblk0p1";

int main(int argc, char **argv)
{
	int opt,i, ret;

	FILE *fp;
	struct mntent *fs;
	int mountstate = 0;
	char *wbuf = NULL;
	char *readbuf = NULL;
	int size = 1 * KBYTE;
	int	fd;
	char *s = NULL;
	int count =1, retry=0;
	while(-1 != (opt = getopt(argc, argv, "hp:d:s:C:"))) {
		switch(opt) {
		 case 'p':   path =  optarg;    break;
		 case 'd':   device =  optarg;  break;
	 	 case 'h':   print_usage(); 	exit(0);	 
		 case 's':	size = atoi(optarg) * MBYTE ; break;		 
		 case 'C':	count = atoi(optarg) ; break;		 
		}
	}
	if(count == 0)
		count = 1;
	printf("path : %s\n",path);
	printf("device : %s\n",device);
	printf("size = %d",size);

	if(access(device, F_OK) != 0)
	{
		printf("not found device node %s \n", device);
		return -ENODEV;
	}
	if(access(path, F_OK))
	{
		if(mkdir(path,0755) == 0)
		printf("make directory %s \n", path);
		else 
		{
			printf("not found directory & make fail %s \n", path);
			return -ENOENT;
		}
				
	}

   fp = setmntent("/proc/mounts","r");
   if(fp == NULL)
   {   
    	perror("open fail");
   }   
	        
   while((fs = getmntent(fp)) != NULL)
   {   
	   //printf("%s,   %s,   \n",fs->mnt_fsname,fs->mnt_dir);
       if((strcmp(fs->mnt_dir,path)) == 0)
       {   
			printf(" tset\n");

		   umount(path);
	    //   mountstate = 1; 
	   }   
    }   
    endmntent(fp);

	if(mountstate == 0)
    {
		if((mount(device,path,"vfat",MS_SYNCHRONOUS,NULL)) == 0)
    	{
			printf("mount ok\n");
			mountstate = 1;
	    }
        else
		{
			perror("mount fail");
			return -ENODEV;
		}
	 }
		

	wbuf = malloc(size);
	
	readbuf = malloc(size);
	if(wbuf == NULL  || readbuf== NULL)
	{
		printf("Alloc buffer error\n");
		return -1;
	}
	for(retry=0;retry<count;retry++){
		printf("fill buffer\n");
		srand(time(NULL));
		for(i=0;i<size;i++)
		{
			wbuf[i] = rand()%0xff;
		}
	
		s = malloc(128);
		sprintf(s,"%s/test.txt",path);
	
		if(access(s,F_OK))
		{
			remove(s);		
		}
			
		fd = open(s,O_RDWR|O_CREAT|O_TRUNC);
		if(fd < 0)
		{
		printf("file open fail\n");
		return -EBADFD;
		}

		write(fd,wbuf,size );

		close(fd);
	
		fd = open(s,O_RDONLY);
		read(fd,readbuf,size);
		
		ret = memcmp(wbuf,readbuf,size);
		printf("compare  %s , retry %d \n\n",ret ? "fail" : "ok", i+1);
	}

	free(wbuf);
	free(s);
	free(readbuf);
	close(fd);
    umount(path);
	return ret;
}
