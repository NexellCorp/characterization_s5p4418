#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* strerror */
#include <unistd.h>
#include <errno.h>   /* errno */

#define CPU_NUM					(4)
#define TEST_COUNT				(1)

void print_usage(void)
{
    printf("usage: options\n"
    	   " -n cpu numbers to check (default %d)\n"
		   " -C test count (default %d)\n"
		   " -T not support\n"
		   , CPU_NUM
		   , TEST_COUNT
		   );
}

int main(int argc, char* argv[])
{
    FILE *fp = NULL;
    char cmd[100] = "grep processor /proc/cpuinfo | wc -l";
    char buf[16] = { 0, };
    int opt;
    size_t len = 0;
	int num = 0, max = CPU_NUM;
	int   test_count = TEST_COUNT, index = 0;

    while (-1 != (opt = getopt(argc, argv, "hn:C:T"))) {
		switch(opt) {
        case 'h': print_usage();	exit(0);		break;
        case 'n': max = strtol(optarg, NULL, 10);	break;
        case 'C': test_count = atoi(optarg);		break;					
		case 'T': 									break;
        default : goto err_out; 					break;
      	}
	}

	for (index = 0; test_count > index; index++) {
	    fp = popen(cmd, "r");
	    if(NULL == fp) {
	        printf("error [%d:%s]\n", errno, strerror(errno));
	        return -errno;
	    }

	    len = fread((void*)buf, sizeof(char), sizeof(buf), fp);
	    if(0 == len) {
	        pclose(fp);
	        printf("error [%d:%s]\n", errno, strerror(errno));
	        return -errno;
	    }
	    pclose(fp);

		num	= strtol(buf, NULL, 10);

		if (num != max) {
			printf("error: smp cpus %d not equal %d !!!\n", num, max);
			return -EIO;
		}
		printf("smp cpus:%d (request %d)\n", num, max);
	} // end for loop (test count)
	return 0;

err_out:
	print_usage();
    return -EINVAL;
}
