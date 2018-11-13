#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define PWM_DEFAULT_PORT 		0
#define PWM_DEFAULT_CLOCK 		100
#define PWM_DEFAULT_DUTY		50
#define TEST_COUNT              (1)

void print_usage(void)
{
	printf("usage: options\n"
		   " -p pwm_port        , default %d \n"
		   " -r pwm_clock rate  , default %d \n"
		   " -d pwm_duty        , default %d \n"
		   " -C test_count      , default %d \n"
		   " -T not support\n"
			,PWM_DEFAULT_PORT 
			,PWM_DEFAULT_CLOCK
			,PWM_DEFAULT_DUTY 
			,TEST_COUNT
	);	 
}

int set_pwm(int ch, int clk, int duty)
{
	char s[100];
	char p[100];
	int fd;
	sprintf(s,"/sys/devices/platform/pwm/pwm.%d",ch);
	fd = open(s,O_WRONLY);
	if (fd < 0)	{
		printf("pwm open err \n");
		return -1;
	}
	
	sprintf(p,"%d,%d",clk,duty);
	write(fd,p,sizeof(p));

	close(fd);
	return 0;
}

int stop_pwm(int ch)
{
	char s[100];
	char p[100];
	int fd;
	sprintf(s,"/sys/devices/platform/pwm/pwm.%d",ch);
	fd = open(s,O_WRONLY);
	if (fd < 0)	{
		printf("pwm open err \n");
		return -1;
	}
	
	sprintf(p,"0.0");
	write(fd,p,sizeof(p));

	close(fd);
	return 0;

}
int get_duty( void)
{
	char s[100];
	char p[100];
	int fd;
	int ret;
	sprintf(s,"/sys/devices/platform/ppm/duty");
	fd = open(s,O_RDONLY);
	if (fd < 0)	{
		printf("ppm open err \n");
		return -1;
	}
	
	read(fd,p,10);

	close(fd);
	ret = atoi(p);

	return ret;

}

int main(int argc, char **argv)
{
	int read_duty,set_duty = PWM_DEFAULT_DUTY;
	int opt;
	int ch  = PWM_DEFAULT_PORT;
	int clk = PWM_DEFAULT_CLOCK;
	int calc=0;
	int   test_count = TEST_COUNT, index = 0;

	while (-1 != (opt = getopt(argc, argv, "hp:r:d:C:T"))) {
		switch(opt) {
			case 'h' :		print_usage();	exit(0);	break;
			case 'p' :      ch = atoi(optarg);			break;
			case 'r' :      clk = atoi(optarg);			break;
			case 'd' :      set_duty = atoi(optarg);	break;
	        case 'C' : 		test_count = atoi(optarg);	break;
 	        case 'T' :		break;
			}   
   	}   

	if (clk> PWM_DEFAULT_CLOCK)
		clk = 100;

	for (index = 0; test_count > index; index++) {	
	 	if (set_pwm(ch,clk,set_duty) < 0) {
			printf("pwm setting error\n");
			return -1;
		}
		
		sleep(1);

		read_duty =	get_duty(); 

		if (stop_pwm(ch)<0)	{
			printf("pwm setting error\n");
			return -1;
		}

		printf("set duty : %d : get duty %d\n",set_duty, read_duty);
	
		if(read_duty == set_duty)
			calc =  0;
		else if(read_duty > set_duty)
			calc = read_duty - set_duty;	
		else if(set_duty > read_duty)
			calc = set_duty - read_duty;	
	
		if(calc > 5) {
			printf("diff input/output!\n");
			return -1;
		}
	} // end for loop (test count)		
	return 0;
}
