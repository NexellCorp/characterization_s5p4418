#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h> 			/* error */

#include <linux/watchdog.h>

#define	WATCHDOG_DEV_FILE	"/dev/watchdog"
#define	WATCHDOG_SYSFS_FILE	"/sys/devices/platform/wdt/soft"

#define TEST_COUNT				(1)
#define DEFAULT_TIME			(3)

int watchdog_set_time(int time, int wait)
{
	int fd, ret = 0, on = 0;

	fd = open( WATCHDOG_DEV_FILE, O_RDONLY );
	if (0 > fd)
		return -1;

	ioctl(fd, WDIOC_SETTIMEOUT, &time);

	on = WDIOS_ENABLECARD;
	ioctl(fd, WDIOC_SETOPTIONS, &on);

	/* This blocks until the wdt timer causes an interrupt */
	if (wait) {
		ret = read(fd, &time, sizeof(unsigned long));
		// send keepalive
		ioctl(fd, WDIOC_KEEPALIVE, 0);
		if (0 > ret) {
			printf("error:read...\n");
			return -1;
		}

		/* Disable wdt timer interrupts */
		on = WDIOS_DISABLECARD;
		ioctl(fd, WDIOC_SETOPTIONS, &on);
	}	
	close(fd);
	return 0;
}

int watchdog_get_time(int *time)
{
	int fd;

	fd = open( WATCHDOG_DEV_FILE, O_RDONLY );
	if (0 > fd)
		return -1;

	ioctl(fd, WDIOC_GETTIMEOUT, time);
	close(fd);
	return 0;
}

int watchdog_keep_alive(void)
{
	int fd;

	fd = open( WATCHDOG_DEV_FILE, O_RDONLY );
	if (0 > fd)
		return -1;

	ioctl(fd, WDIOC_KEEPALIVE, 0);
	close(fd);
	return 0;
}

int watchdog_onoff(int on)
{
	int fd;

	fd = open( WATCHDOG_DEV_FILE, O_RDONLY );
	if (0 > fd)
		return -1;

	if (on)
		on = WDIOS_ENABLECARD;
	else
		on = WDIOS_DISABLECARD;

	ioctl(fd, WDIOC_SETOPTIONS, &on);

	close(fd);
	return 0;
}

//--------------------------------------------------------------------------------
void print_usage(void)
{
    printf( "usage: options\n"
			" -C	test count(default: %d) \n"			
            " -T	set watchdog time (sec)	\n"
            " -r	read time				\n"
            " -k	keepalive			 	\n"
            " -o	watchdog on			 	\n"
            " -f	watchdog off		 	\n"
            " -t	soft wdt test		 	\n"
			, TEST_COUNT
            );
}

int main(int argc, char **argv)
{
	int ioc_set= 0 , ioc_opt = 0;
	int wdt_time  = DEFAULT_TIME;
	int wdt_alive = 0;
	int wdt_on	  = 0;
	int opt, ret = 0;
	int test_count = TEST_COUNT, index = 0, test_on = 0;
	int fd;
	char p[10];

    while (-1 != (opt = getopt(argc, argv, "hC:T:rkoft"))) {
		switch(opt) {
        case 'h':   print_usage();  exit(0);   	break;
        case 'C':   test_count = atoi(optarg);	break;
        case 'T':   ioc_set   = 1;
        			wdt_time  = atoi(optarg); 	break;
        case 'k':   wdt_alive = 1;				break;
        case 'r':   							break;
        case 'o':   ioc_opt = 1, wdt_on = 1;	break;
        case 'f':   ioc_opt = 1, wdt_on = 0;	break;
		case 't':	test_on = 1;				break;
        default:
        	break;
         }
	}

	if (test_on) {
		fd = open( WATCHDOG_SYSFS_FILE, O_WRONLY );
		if (0 > fd) {
			printf("wdt sysfs open err\n");
			return -1;
		}

		sprintf(p, "%d", test_on);
		write(fd, p, sizeof(p));

		close(fd);

		for (index = 0; test_count > index; index++) {
		
			printf("Watchdog Set : %d sec\n", wdt_time);
			ret = watchdog_set_time(wdt_time, test_on);
			if (0 != ret)
				goto wdt_exit;
		}
	}
	else {
		if (ioc_set) {
			ret = watchdog_set_time(wdt_time, test_on);
			printf("%s: set watchdog %d sec ...\n", ret?"Fail":"Done", wdt_time);
		} else {
			ret = watchdog_get_time(&wdt_time);
			printf("%s: get watchdog %d sec ...\n", ret?"Fail":"Done", wdt_time);
		}
	
		if (! ret && wdt_alive) {
			watchdog_keep_alive();
			printf("%s: watchdog keep alive \n", ret?"Fail":"Done");
		}
	
		if (ioc_opt) {
			ret = watchdog_onoff(wdt_on);
			printf("%s: set watchdog ...\n", wdt_on?" ON ":"OFF ");
		}
	}

wdt_exit:

	return ret;
}
