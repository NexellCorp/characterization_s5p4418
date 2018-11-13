#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#define	RTC_DEV_NAME 			"/dev/rtc0"
#define	ALARM_WAKEUP_TIME		(3)		// sec
#define TEST_COUNT				(1)

void print_usage(void)
{
    printf( "usage: options\n"
            " -d	device name, default %s \n"
			" -C	test_count (default %d)\n"
            " -T	alarm time, will be occurs alarm after input sec (default %d sec)\n"
            , RTC_DEV_NAME
            , ALARM_WAKEUP_TIME
			, TEST_COUNT
            );
}

static int set_alarm(const char *rtc, long sec, int wait, int debug)
{
	struct rtc_time rtm;
	int fd, ret;

	fd = open(rtc, O_RDONLY);
	if (0 > fd) {
		fprintf(stderr, "error: rtc open %s\n", rtc);
		return -errno;
	}

	/* Read the RTC time/date */
	ret = ioctl(fd, RTC_RD_TIME, &rtm);
	if (0 > ret) {
		fprintf(stderr, "error: ioctl - RTC_RD_TIME\n");
		goto alarm_exit;
	}

	if (debug)
		fprintf(stdout, "Time : %02d-%02d-%04d, %02d:%02d:%02d \n",
			rtm.tm_mday, rtm.tm_mon + 1, rtm.tm_year + 1900,
			rtm.tm_hour, rtm.tm_min, rtm.tm_sec);

	/* Set the alarm, and check for rollover */
	rtm.tm_sec += (sec%60);
	if (rtm.tm_sec >= 60) {
		rtm.tm_sec %= 60;
		rtm.tm_min++;
	}

	rtm.tm_min += (sec/60)%60;
	if (rtm.tm_min >= 60) {
		rtm.tm_min %= 60;
		rtm.tm_hour++;
	}

	rtm.tm_hour += (sec/(60*60))%24;
	if (rtm.tm_hour >= 24) {
		rtm.tm_hour %= 24;
		rtm.tm_mday++;
	}

	/* Set the alarm */
	ret = ioctl(fd, RTC_ALM_SET, &rtm);
	if (0 > ret) {
		if (errno == ENOTTY) {
			fprintf(stderr, "alarm irqs not supported.");
			close(fd);
			return 0;
		}
		fprintf(stderr, "error: ioctl - RTC_ALM_SET\n");
		goto alarm_exit;
	}

	/* Read the current alarm settings */
	ret = ioctl(fd, RTC_ALM_READ, &rtm);
	if (0 > ret) {
		fprintf(stderr, "error: ioctl - RTC_ALM_READ\n");
		return -errno;
	}

	if (debug)
		fprintf(stdout, "Wake : %02d-%02d-%04d, %02d:%02d:%02d \n",
			rtm.tm_mday, rtm.tm_mon + 1, rtm.tm_year + 1900,
			rtm.tm_hour, rtm.tm_min, rtm.tm_sec);

	/* Enable alarm interrupts */
	ret = ioctl(fd, RTC_AIE_ON, 0);
	if (0 > ret) {
		fprintf(stderr, "error: ioctl - RTC_AIE_ON\n");
		goto alarm_exit;
	}

	fflush(stdout);

	/* This blocks until the alarm ring causes an interrupt */
	if (wait) {
		ret = read(fd, &sec, sizeof(unsigned long));
		if (0 > ret) {
			fprintf(stderr, "error: read...\n");
			goto alarm_exit;
		}

		/* Disable alarm interrupts */
		ret = ioctl(fd, RTC_AIE_OFF, 0);
		if (0 > ret) {
			fprintf(stderr, "error: ioctl - RTC_AIE_OFF\n");
			goto alarm_exit;
		}
	}

alarm_exit:

	close(fd);
	if (ret)
		return -errno;

	return 0;
}

int main(int argc, char **argv)
{
	int opt, ret;
	char path[16] = RTC_DEV_NAME;
	long alarm = ALARM_WAKEUP_TIME;
	struct timeval tv1, tv2;
	long us = 0, rang;
	int   test_count = TEST_COUNT, index = 0;

    while(-1 != (opt = getopt(argc, argv, "hd:C:T:"))) {
	switch(opt) {
        case 'h': print_usage();  exit(0);   		break;								
        case 'd': strcpy(path, optarg);				break;
        case 'C': test_count = atoi(optarg);		break;
        case 'T': alarm = atoi(optarg);				break;
        default : goto err_out; break;
		}
	}

	for (index = 0; test_count > index; index++) {
		fprintf(stdout, "Alarm: %ld min %ld sec\n", alarm/60, alarm%60);

		gettimeofday(&tv1, NULL);

		ret = set_alarm(path, alarm, 1, 0);
		if (0 != ret)
			return ret;

		gettimeofday(&tv2, NULL);
		us = tv2.tv_usec - tv1.tv_usec;
		if(0 > us) {
			us += 1000000;
			tv2.tv_sec--;
		}
		rang = (tv2.tv_sec-tv1.tv_sec);

		if (abs(alarm - rang) > 1) {
			fprintf(stdout, "Rang : %ld min %ld.%06ld sec \n", rang/60, rang%60, us);
			return -EBADMSG;
		}

		fprintf(stdout, "Rang : %ld min %ld.%06ld sec \n", rang/60, rang%60, us);
	} // end for loop (test count)
	return 0;

err_out:
	print_usage();
    return -EINVAL;
}

