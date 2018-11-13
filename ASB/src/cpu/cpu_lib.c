#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>  /* strerror */
#include <fcntl.h>
#include <unistd.h>
#include <asm/unistd.h> 	// __NR_gettid

/*
 *	sys APIs
 */
int sys_read(const char *file, char *buffer, int buflen)
{
	int fd, ret;

	if (0 != access(file, F_OK)) {
		printf("cannot access file (%s).\n", file);
		return -errno;
	}

	fd = open(file, O_RDONLY);
	if (0 > fd) {
		printf("cannot open file (%s).\n", file);
		return -errno;
	}

	ret = read(fd, buffer, buflen);
	if (0 > ret) {
		printf("failed, read (file=%s, data=%s)\n", file, buffer);
		close(fd);
		return -errno;
	}

	close(fd);

	return ret;
}

int sys_write(const char *file, const char *buffer, int buflen)
{
	int fd;

	if (0 != access(file, F_OK)) {
		printf("cannot access file (%s).\n", file);
		return -errno;
	}

	fd = open(file, O_RDWR|O_SYNC);
	if (0 > fd) {
		printf("cannot open file (%s).\n", file);
		return -errno;
	}

	if (0 > write(fd, buffer, buflen)) {
		printf("failed, write (file=%s, data=%s)\n", file, buffer);
		close(fd);
		return -errno;
	}
	close(fd);

	return 0;
}

/*
 *	Check ASV IDS/RO
 */
#define	ECID_PATH	"/sys/devices/platform/cpu/uuid"

int get_sys_ecid(unsigned int ecid[4])
{
	char buffer[128];
	int fd;

	if (0 != access(ECID_PATH, F_OK)) {
		printf("cannot access file (%s).\n", ECID_PATH);
		return -errno;
	}

	fd = open(ECID_PATH, O_RDONLY);
	if (0 > fd) {
		printf("cannot open file (%s).\n", ECID_PATH);
		return -errno;
	}

	if(0 > read(fd, buffer, sizeof(buffer))) {
		printf("failed, read(file=%s, data=%s)\n", ECID_PATH, buffer);
		close(fd);
		return -errno;
	}
	close(fd);

	sscanf(buffer, "%x:%x:%x:%x\n", &ecid[0], &ecid[1], &ecid[2], &ecid[3]);
	return 0;
}

static inline unsigned int MtoL(unsigned int data, int bits)
{
	unsigned int result = 0, mask = 1;
	int i = 0;
	for (i = 0; i<bits ; i++) {
		if (data&(1<<i))
			result |= mask<<(bits-i-1);
	}
	return result;
}

unsigned int get_ecid_ids(unsigned int ecid[4])
{
	return MtoL((ecid[1]>>16) & 0xFF, 8);
}

unsigned int get_ecid_ro(unsigned int ecid[4])
{
	return MtoL((ecid[1]>>24) & 0xFF, 8);
}

/*
 *	Frequency APIs (Unit is KHZ)
 *  NOTE
 * 		커널에서 DVFS 할 경우 regulator 에 대한 제어가 안됨.
 */
#if 0
#define CPUFREQ_PATH	"/sys/devices/system/cpu"
int set_cpu_speed(long khz)
{
	char path[128] = { 0, };
	char data[32] = { 0, };

	/*
	 * change governor to userspace
	 */
	sprintf(path, "%s/cpu0/cpufreq/scaling_governor", CPUFREQ_PATH);
	if (0 > sys_write(path, "userspace", strlen("userspace")))
		return -errno;

	/*
	 *	change cpu frequency
	 */
	sprintf(path, "%s/cpu0/cpufreq/scaling_setspeed", CPUFREQ_PATH);
	sprintf(data, "%ld", khz);
	if (0 > sys_write(path, data, strlen(data)))
		return -errno;

	return 0;
}

unsigned long get_cpu_speed(void)
{
	char path[128] = { 0, };
	char data[128];
	int ret;

	sprintf(path, "%s/cpu0/cpufreq/scaling_cur_freq", CPUFREQ_PATH);
	ret = sys_read(path, data, sizeof(data));
	if (0 > ret)
		return ret;
	return strtol(data, NULL, 10);	 /* khz */
}

#else
#define CPUFREQ_PATH	"/sys/devices/platform/pll/pll.1"
int set_cpu_speed(long khz)
{
	char data[128];
	sprintf(data, "%ld", khz);
	return sys_write(CPUFREQ_PATH, data, strlen(data));
}

unsigned long get_cpu_speed(void)
{
	char data[128];
	int ret = sys_read(CPUFREQ_PATH, data, sizeof(data));
	if (0 > ret)
		return ret;

	return strtol(data, NULL, 10); /* khz */
}
#endif

#define	AVAILABLE_FREQ_PATH 	"/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies"
int cpu_avail_freqnr(void)
{
	const char *path = AVAILABLE_FREQ_PATH;
	char buff[256] = { 0, };
	int no = 0, ret;
	char *s = buff;

	ret = sys_read(path, buff, sizeof(buff));
	if (0 > ret)
		return ret;

	while (s) {
		if (*s != '\n' && *s != ' ')
			no++;
		s = strchr(s, ' ');
		if (!s)
			break;
		s++;
	}
	return no;
}

int cpu_avail_freqs(long *khz, int size)
{
	const char *path = AVAILABLE_FREQ_PATH;
	char buff[256] = { 0, };
	int i = 0, no = 0, ret;
	char *s = buff;

	if (NULL == khz)
		return -EINVAL;

	ret = sys_read(path, buff, sizeof(buff));
	if (0 > ret)
		return ret;

	while (s) {
		if (*s != '\n' && *s != ' ')
			no++;
		s = strchr(s, ' ');
		if (!s)
			break;
		s++;
	}

	s = buff;
	for (i = 0; size>i && no>i; i++, s++) {
		khz[i] = strtol(s, NULL, 10);
		s = strchr(s, ' ');
		if (!s)
			break;
	}
	return no;
}

/*
 *	Voltage APIs (Unit is uV)
 */
#define	REGULATOR_PATH	"/sys/class/regulator/regulator"
int set_cpu_voltage(int id, long uV)
{
	char file[128];
	char data[32];
	unsigned int off_uV = uV/100;

	/* Cut off 100 micro volt. */
	off_uV = off_uV*100;

	sprintf(file, "%s.%d/%s", REGULATOR_PATH, id, "microvolts");
	sprintf(data, "%ld", uV);

	return sys_write(file, data, strlen(data));
}

unsigned long get_cpu_voltage(int id)
{
	char file[128];
	char data[128];
	int ret;

	sprintf(file, "%s.%d/%s", REGULATOR_PATH, id, "microvolts");

	ret = sys_read(file, data, sizeof(data));
	if (0 > ret)
		return ret;

	return strtol(data, NULL, 10);
}

#define	AVAILABLE_VOL_PATH 		"/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_voltages"
int cpu_avail_volts(long *uV, int size)
{
	const char *path = AVAILABLE_VOL_PATH;
	char buff[256] = { 0, };
	int i = 0, no = 0, ret;
	char *s = buff;

	if (NULL == uV)
		return -EINVAL;

	ret = sys_read(path, buff, sizeof(buff));
	if (0 > ret)
		return ret;

	while (s) {
		if (*s != '\n' && *s != ' ')
			no++;
		s = strchr(s, ' ');
		if (!s)
			break;
		s++;
	}

	s = buff;
	for (i = 0; size>i && no>i; i++, s++) {
		uV[i] = strtol(s, NULL, 10);
		s = strchr(s, ' ');
		if (!s)
			break;
	}
	return no;
}

/*
 *	CPU cores for SMP
 */
int get_cpu_nr(void)
{
    FILE *fp = NULL;
    char cmd[100] = "grep processor /proc/cpuinfo | wc -l";
    char buf[16] = { 0, };
    size_t len = 0;

    fp = popen(cmd, "r");
    if(NULL == fp) {
        printf("failed %s\n", cmd);
        return -errno;
    }

    len = fread((void*)buf, sizeof(char), sizeof(buf), fp);
    if(0 == len) {
        pclose(fp);
        printf("failed read for %s\n", cmd);
        return -errno;
    }
    pclose(fp);

	return strtol(buf, NULL, 10);
}

pid_t gettid(void)
{
	return (pid_t)syscall(__NR_gettid);
}

