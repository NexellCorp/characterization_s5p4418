#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <time.h>		// ctime

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define __DUMMY_READ__		10		// wait for stable...

#include "adc_test.h"

#define nx_debug(fmt, ...)			\
		if (g_debug == 1)			\
			printf(fmt, ##__VA_ARGS__)


int g_repeat = DEF_REPEAT;


enum _test_mode 		{ MODE_ASB = 100, MODE_VTK = 200 };
typedef enum _test_mode TEST_MODE;

TEST_MODE g_mode;					/* ASB, VTK */ 
double g_involt;					/* voltage */
int g_torelance;					/* percentage */
int g_chan = ADC_MAX_CH;			/* adc channel : ADC_MAX_CH => all, 0~ADC_MAX_CH-1 => channel */
int g_sample;						/* sample count */

int g_debug = 0;					/* print debug message. hidden */


void print_info(struct stat *sb)
{

	if (!sb) return;

	printf("File type:                ");

	switch (sb->st_mode & S_IFMT) {
		case S_IFBLK:  printf("block device\n");            break;
		case S_IFCHR:  printf("character device\n");        break;
		case S_IFDIR:  printf("directory\n");               break;
		case S_IFIFO:  printf("FIFO/pipe\n");               break;
		case S_IFLNK:  printf("symlink\n");                 break;
		case S_IFREG:  printf("regular file\n");            break;
		case S_IFSOCK: printf("socket\n");                  break;
		default:       printf("unknown?\n");                break;
	}

	printf("I-node number:            %ld\n", (long) sb->st_ino);

	printf("Mode:                     %lo (octal)\n",
			(unsigned long) sb->st_mode);

	printf("Link count:               %ld\n", (long) sb->st_nlink);
	printf("Ownership:                UID=%ld   GID=%ld\n",
			(long) sb->st_uid, (long) sb->st_gid);

	printf("Preferred I/O block size: %ld bytes\n",
			(long) sb->st_blksize);
	printf("File size:                %lld bytes\n",
			(long long) sb->st_size);
	printf("Blocks allocated:         %lld\n",
			(long long) sb->st_blocks);

	printf("Last status change:       %s", ctime(&sb->st_ctime));
	printf("Last file access:         %s", ctime(&sb->st_atime));
	printf("Last file modification:   %s", ctime(&sb->st_mtime));
}


int get_adc_value(int ch, int is_raw, int count)
{
	unsigned int sample_data[SAMPLE_RANGE] = { 0, };

	FILE *fp;
	int ret = -1;
	char buf[256] = { 0, };
	char cmd[256] = { 0, };
	int err = 0;

	uint32_t idx;

	int min = 0, max = 0, freq = 0;
	int min_flag = 1;
	int i;


	if (count < 0)
		return -1;

	/* /sys/bus/iio/devices/iio\:device0/in_voltage0_raw */
	sprintf(cmd, "%s%d%s", "cat " ADC_SYS_DEV "/in_voltage", ch, is_raw ? "_raw" : "");

	for (i = 0; i < count; i++)
	{
		memset (buf, 0x00, sizeof buf);

		fp = popen (cmd, "r");
		if (!fp) {
			perror ("popen");
			return -1;
		}
		fgets(buf, 256, fp);
		pclose (fp);

		if (i <= __DUMMY_READ__)	// dummy read
			continue;

		idx = strtoul(buf, NULL, 10);
		if (idx >= SAMPLE_RANGE)
		{
			err++;
			continue;
		}

		(sample_data[idx])++;
	}


	for (i = 0; i < SAMPLE_RANGE; i++) {
		if (sample_data[i] > 0)
		{
			if (min_flag)
			{
				min = i;
				min_flag = 0;
			}
			max = i;
		}
		if (sample_data[i] >= sample_data[freq])
			freq = i;
	}

	nx_debug("ch[%d]  -    min: %d    max: %d    freq: %d\n", ch, min, max, freq);

	ret = freq;

	if (err > 0)
		ret = -1;

	return ret;
}


int asb_pwm_adc_on(int pwm_id)
{
	char cmd1[256] = { 0, };
	char cmd2[256] = { 0, };

	sprintf(cmd1, "echo %d,%d > /sys/devices/platform/pwm/pwm.%d", PWM_FREQ, PWM_DUTY, pwm_id);
	sprintf(cmd2, "echo %d > /sys/class/gpio/unexport 2>/dev/null", to_gpionum(pwm_id));

	nx_debug("%s\n", cmd1);
	nx_debug("%s\n", cmd2);

	system(cmd1);
	system(cmd2);

	return 0;
}

int asb_pwm_adc_off(int pwm_id)
{
	char cmd1[256] = { 0, };
	char cmd2[256] = { 0, };
	char cmd3[256] = { 0, };

	sprintf(cmd1, "echo %d > /sys/class/gpio/export 2>/dev/null", to_gpionum(pwm_id));
	sprintf(cmd2, "echo out > /sys/class/gpio/gpio%d/direction", to_gpionum(pwm_id));
	sprintf(cmd3, "echo 0 > /sys/class/gpio/gpio%d/value", to_gpionum(pwm_id));

	nx_debug("%s\n", cmd1);
	nx_debug("%s\n", cmd2);
	nx_debug("%s\n", cmd3);

	system(cmd1);
	system(cmd2);
	system(cmd3);

	return 0;
}


int asb_init_pwm(void)
{
	if (g_mode == MODE_ASB) {
		asb_pwm_adc_off(1);
		asb_pwm_adc_off(2);
	}

	return 0;
}

int asb_prepare_pwm(int adc_ch)
{
	if (g_mode == MODE_ASB) {
		if (adc_ch == 6) {
			asb_pwm_adc_on(1);
		}
		else if (adc_ch == 7) {
			asb_pwm_adc_on(2);
		}
	}
	return 0;
}

int asb_finish_pwm(int adc_ch)
{
	if (g_mode == MODE_ASB) {
		if (adc_ch == 6) {
			asb_pwm_adc_off(1);
		}
		else if (adc_ch == 7) {
			asb_pwm_adc_off(2);
		}
	}
	return 0;
}


int get_adc_values(int *values)
{
	int i;
	int ch;

	//ch = (g_mode == MODE_ASB) ? ASB_VOLT_CNT : ADC_MAX_CH;
	ch = ADC_MAX_CH;
	asb_init_pwm();

	for (i = 0; i < ch; i++) {
		if (g_mode == MODE_VTK
			&& g_chan != ADC_MAX_CH
			&& g_chan != i)
			continue;

		asb_prepare_pwm(i);

		values[i] = get_adc_value(i, GET_RAW, g_sample + __DUMMY_READ__);
		if (values[i] < 0) {
			printf("Cannot get adc%d value.\n", i);
			return -1;
		}

		asb_finish_pwm(i);
	}

	return 0;
}


/*
 * Test accuracy for ASB.

 * argument:
 *      value         read from adc
 *      input         input voltage (Min 0 ~ Max 1.8v)
 *      thr           threshold
 *
 * return:
 *      PASS          test pass
 *      FAIL          test fail
 *      otherwise     error
 */
int test_accuracy_asb(int value, double input, int thr)
{
	double exp;
	int min, max;

	if (input < 0 || input > MAX_VOLTAGE)
		return -EINVAL;

	if (thr < 0 || thr > 100)
		return -EINVAL;

	exp = input / MAX_VOLTAGE * 4096;

	min = (int)(exp * ((100 - thr)/100.0));
	max = (int)(exp * ((100 + thr)/100.0));

	nx_debug("value: %d (torelance range: %d ~ %d)\n", value, min, max);

	if (value >= min && value <= max)
		return PASS;

	return FAIL;
}


/*
 *  Test accuracy for VTK.
 *
 *  return:
 *      PASS          test pass ((max - min)/4096 < torelance)
 *      FAIL          test fail
 *      otherwise     error
 */
int test_accuracy_vtk(int *values)
{
	int i = 0;
	int min, max;
	double diff;

	if (!values) {
		nx_debug("Cannot check accuracy! Invalid arguments.\n");
		return -EINVAL;
	}


	if (g_chan != ADC_MAX_CH) {
		printf("val: %d\n", values[g_chan]);
		return PASS;
	}


	min = max = values[0];

	for (i = 1; i < ADC_MAX_CH; i++) {
		if (min > values[i]) min = values[i];
		if (max < values[i]) max = values[i];
	}

	diff = ((max - min) / (double)SAMPLE_RANGE) * 100;
	nx_debug("\tvalues -    min: %d,  max: %d,   diff: %f\n", min, max, diff);

	if (diff < g_torelance)
		return PASS;
	else
		return FAIL;
}

/*
 *  Test accuracy with get from ADC.
 *
 *  return:
 *      PASS          test pass
 *      FAIL          test fail
 *      otherwise     error
 */
int test_accuracy(int *values)
{
	int i = 0;
	int ret, errcnt = 0;
	int result;

	if (g_mode == MODE_ASB) {
		for (i = 0; i < ASB_VOLT_CNT; i++)
		{
			ret = test_accuracy_asb(values[i], ASB_INVOLT_TABLE[i], g_torelance);
			if (ret != PASS)
				errcnt++;
		}
		for (i = ASB_VOLT_CNT; i < ADC_MAX_CH; i++)
		{
			ret = test_accuracy_asb(values[i], ASB_INVOLT_TABLE[i], g_torelance);
			if (ret != PASS)
				errcnt++;
		}

		if (errcnt == 0)
			result = PASS;
		else
			result = FAIL;
	}
	else /* MODE_VTK */ {
		result = test_accuracy_vtk(values);
	}

	return result;
}


void print_usage(void)
{
	fprintf(stderr, "Usage: %s\n", PROG_NAME);
	fprintf(stderr, "                [-m TestMode]        (ASB or VTK. default:VTK)\n");
	fprintf(stderr, "                [-t torelance]       (default:%d)\n", DEF_TORELANCE);
	fprintf(stderr, "                [-n sample count]    (default:%d)\n", DEF_SAMPLE_COUNT);
	fprintf(stderr, "                [-v voltage]         (0~1.8V. default:%f)\n", DEF_VOLTAGE);
	fprintf(stderr, "                [-c channel]         (0~%d. 0:all(default), else:channel)\n\n",
					ADC_MAX_CH);
	fprintf(stderr, "                [-C test_count]      (default %d)\n", DEF_REPEAT);
	fprintf(stderr, "                [-T not support]\n");
	fprintf(stderr, "    NOTE: 'voltage' and 'channel' option applies only if the VTK.\n");
	fprintf(stderr, "          ASB tests only 1-6 channel.\n");
}

int parse_opt(int argc, char *argv[])
{
	int opt;

	TEST_MODE _mode = MODE_VTK;
	double _involt = 0.0;
	int _torelance = 0;
	int _chan = 0;
	int _sample = 0;
	int _repeat = 0;

	char tmp[256] = { 0, };


	while ((opt = getopt(argc, argv, "m:v:c:n:t:C:Tdh")) != -1) {
		switch (opt) {
			case 'C':
				_repeat		= atoi(optarg);
				break;
			case 'T':
				print_usage();
				break;
			case 'm':
				snprintf(tmp, 256, "%s", optarg);
				_mode = (strcmp(tmp, "ASB") == 0) ? MODE_ASB : MODE_VTK;
				break;
			case 'v':
				_involt		= strtod(optarg, NULL);
				break;
			case 'c':
				_chan		= atoi(optarg);
				break;
			case 'n':
				_sample		= atoi(optarg);
				break;
			case 't':
				_torelance	= atoi(optarg);
				break;
			case 'd':
				g_debug		= 1;
				break;
			case 'h':
			default: /* '?' */
				print_usage();
				exit(EINVAL);
		}
	}


	nx_debug("m: %d, v:%f, c:%d, n:%d, t:%d\n", _mode, _involt, _chan, _sample, _torelance);

	if (_repeat < 0) {
		goto _exit;
	}

	if (_involt < 0.0 || _involt > MAX_VOLTAGE) {
		goto _exit;
	}

	if (_torelance < 0 || _torelance > 100) {
		goto _exit;
	}

	if (_chan < 0 || _chan > ADC_MAX_CH) {
		goto _exit;
	}

	if (_sample < 0) {
		goto _exit;
	}

	g_repeat = (_repeat == 0) ? DEF_REPEAT : _repeat;
	g_mode = _mode;
	g_chan = (_chan == 0) ? ADC_MAX_CH : _chan-1;
	g_torelance = (_torelance == 0) ? DEF_TORELANCE : _torelance;
	g_involt = (_involt == 0) ? DEF_VOLTAGE  : _involt;
	g_sample = (_sample == 0) ? DEF_SAMPLE_COUNT : _sample;

	return 0;

_exit:
	print_usage();
	exit(EINVAL);
}

int test(void)
{
	int values[ADC_MAX_CH];
	int result = FAIL;

	if (get_adc_values(values) < 0)
		exit(-1);

	result = test_accuracy(values);

	printf("\tresult =========>  %s\n", (result == PASS) ? "pass" : "fail");

	return result;
}


/* */
int main(int argc, char *argv[])
{
	struct stat sb;

	int err;
	int r;			// repeat


	parse_opt(argc, argv);

	if (stat(ADC_SYS_DEV, &sb) == -1) {
		err = errno;
		perror("stat");
		exit(err);
	}

	//print_info(&sb);


	for (r = 1; r <= g_repeat; r++) {
		printf("[TRY #%d] ....\n", r);
		if (test() != PASS) {
			exit(EBADMSG);
		}
	}


	exit(EXIT_SUCCESS);
}
