
rtc_alarm_test

	build:
		>  arm-cortex_a9-linux-gnueabi-gcc -o rtc_suspend_test rtc_suspend_test.c

	test:
		rtc_alarm_test -d <rtc device node path> -a <alarm time>

		ex> rtc_alarm_test -d /dev/rtc0 -a 3

		description:
			set rtc alarm (unit sec) and wait for rang alarm 

		return:
			compare system time with alarm time,
			0 is sucess, Otherwise errno

