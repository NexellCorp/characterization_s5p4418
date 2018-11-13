/* *************************************************************
 *   Global Configuration
 * *************************************************************/

#define VERSION_STRING	"20150609"




/* *************************************************************
 *   ADC Configuration
 * *************************************************************/

#define ADC_SYS_DEV		"/sys/bus/iio/devices/iio:device0"

#define SAMPLE_RANGE		4096

#define MAX_VOLTAGE			(1.8)
#define DEF_VOLTAGE			MAX_VOLTAGE
#define DEF_SAMPLE_COUNT	(50)
#define DEF_TORELANCE		(10)

#define PASS				(1)
#define FAIL				(0)

#define GET_RAW				(1)

#define DEF_REPEAT			(1)
#define PROG_NAME			"adc_test"

#define ASB_VOLT_CNT		6
#define ADC_MAX_CH			8

#define PWM_FREQ			300000
#define PWM_DUTY			10

double ASB_INVOLT_TABLE[ADC_MAX_CH] = {
	[0] = 0.2,
	[1] = 0.3,
	[2] = 0.4,
	[3] = 0.5,
	[4] = 0.6,
	[5] = 0.9,
	[6] = 0.33,
	[7] = 0.33,
};

// ASB

#define to_gpionum(n)		pwm_to_gpio_num[n]
int pwm_to_gpio_num[4] = {
	[0] = -1,
	[1] = 77,		// GPIO_C13
	[2] = 78,		// GPIO_C14
	[3] = -1,
};
