#include <stdio.h>
#include <unistd.h>	/* optarg */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>  /* strerror */
#include <errno.h>   /* errno */

#include "cpu_lib.h"
#include "cpu_asv.h"

#define	DEFAULT_ARM_REGULATOR_ID	(1)
#define	DEFAULT_CORE_REGULATOR_ID	(2)
#define	DEFAULT_CORE_VOLTAGE_UV		UV(1100)

void print_usage(void)
{
	printf( "usage: options\n"
    		"-a change current arm  voltage level (default on)\n"
    		"-c change current core voltage level (default off)\n"
    		"-t change current arm and core voltage level\n"
    		"-v change arm  regulator ID (default %d)\n"
    		"-i change core regulator ID (default %d)\n"
    		"-d change voltage (ex. +vol, -vol(mV) or +n%%, -n%%)\n"
    		"-b read margin from boot device\n"
			"-C test count \n"
    		"-T test time (ms)\n"
    		, DEFAULT_ARM_REGULATOR_ID
    		, DEFAULT_CORE_REGULATOR_ID
    		);
}

/*
 * MAIN
 */
struct freq_volt {
	long khz;
	long uV;
};

static int get_freq_tables(struct freq_volt *tables, int size)
{
	long *tb;
	int i = 0, nr;
	int cnt = cpu_avail_freqnr();

	if (0 >= cnt)
		return -EINVAL;

	tb = malloc(sizeof(long)*cnt);
	if (NULL == tb)
		return -ENOMEM;

	memset(tb, 0, sizeof(sizeof(long)*cnt));

	cnt = cpu_avail_freqs(tb, size);
	if (0 > cnt)
		return -EINVAL;
	nr = cnt;

	for (i = 0; size > i && cnt > i; i++)
		tables[i].khz = tb[i];

	memset(tb, 0, sizeof(sizeof(long)*cnt));

	cnt = cpu_avail_volts(tb, size);
	if (0 > cnt)
		return nr;

	for (i = 0; size > i && cnt > i; i++)
		tables[i].uV = tb[i];

	return nr;
}

extern int read_env(const char *str, char *rbuf);

#define	AVAIL_FREQ_SIZE		30

int main(int argc, char* argv[])
{
    int opt;
	struct freq_volt fv_tbl[AVAIL_FREQ_SIZE];
	struct freq_volt fv_val[AVAIL_FREQ_SIZE];
	long cur_khz;
	int  size, i = 0;
	long arm_uV = -1, core_uV = DEFAULT_CORE_VOLTAGE_UV;
	char margin[512];

	int  md_voltage = 0;
	bool md_percent = false, md_down = false;

	bool o_arm_mod = true, o_core_mod = false;
	int  o_arm_vID  = DEFAULT_ARM_REGULATOR_ID;
	int  o_core_vID = DEFAULT_CORE_REGULATOR_ID;
	char *o_changeV = NULL;
	int  o_read_env = 0;

    while (-1 != (opt = getopt(argc, argv, "hactv:i:d:bC:T"))) {
		switch(opt) {
		case 'a':   o_arm_mod  = true;  o_core_mod = false;	break;
		case 'c':   o_arm_mod  = false; o_core_mod = true;	break;
		case 't':   o_arm_mod  = true;  o_core_mod = true;	break;
		case 'v':   o_arm_vID  = strtol(optarg, NULL, 10); break;
		case 'i':   o_core_vID = strtol(optarg, NULL, 10); break;
		case 'd':   o_changeV  = optarg;	break;
		case 'b':   o_read_env = 1;						break;
		case 'C':   printf("-C option is not support\n");		break;
       	case 'T':   printf("-T option is not support\n");		break;
        case 'h':	print_usage();  exit(0);	break;
        default:
        	break;
      	}
	}

	if(o_read_env && 0 == read_env("margin", margin)) {
		printf("Read margin...\n");
		o_changeV = margin;
	}

	/*
	 * Get dvfs tables
	 */
	size = get_freq_tables(fv_tbl, AVAIL_FREQ_SIZE);
	if (0 > size) {
		printf("Not support dynamic voltage frequency !!!\n");
		return -EINVAL;
	}

	/* Copy tables */
	memcpy(&fv_val, &fv_tbl, sizeof(fv_tbl));

	/*
	 * Get voltage change option
	 */
	if (o_changeV) {
		char *s = strchr(o_changeV, '-');

		if (s)
			md_down = true;
		else
			s = strchr(o_changeV, '+');

		if (!s)
			s = o_changeV;
		else
			s++;

		if (strchr(o_changeV, '%'))
			md_percent = 1;

		md_voltage = strtol(s, NULL, 10);
	}
	printf("VOL :%s%d%s\n", md_down?"-":"+", md_voltage, md_percent?"%":"mV");


	/*
	 * Find current voltage level with frequency
	 */
	cur_khz = get_cpu_speed();
	for (i = 0; FREQ_ARRAY_SIZE > i; i++) {
		if (cur_khz == (fv_val[i].khz)) {
			arm_uV = fv_val[i].uV;
			break;
		}
	}

	if (0 > arm_uV) {
		printf("FAIL %ldkhz is not exist on the ASV TABLEs !!!\n", cur_khz);
		return -1;
	}

	/*
	 * Get new voltage (+- vol option)
	 */
	if (true == o_arm_mod || true == o_core_mod) {
		long step_vol = VOLTAGE_STEP_UV;
		bool md = false;
		long uV, dv, new, al = 0;
		int id;

		for (i = 0; 2 > i; i++, md = false) {

			if (i == 0) {
				md = o_arm_mod;
				id = o_arm_vID, uV = arm_uV;
			}

			if (i == 1) {
				md = o_core_mod;
				id = o_core_vID, uV = core_uV;
			}

			if (false == md)
				continue;

			dv  = md_percent ? ((uV/100) * md_voltage) : (md_voltage * 1000);;
			new = md_down ? uV - dv : uV + dv;

			if ((new % step_vol)) {
				new = (new / step_vol) * step_vol;
				al = 1;
				if (md_down) new += step_vol;	/* Upper */
			}
			printf("%7ldkhz %s %7ld (%s%ld) align %ld (%s) -> %7ld\n",
				cur_khz, i == 0 ? "ARM ":"CORE",
				uV, md_down?"-":"+", dv, step_vol, al?"X":"O", new);

			/* new voltage */
			set_cpu_voltage(id, new);
		}
	}

	return 0;
}
