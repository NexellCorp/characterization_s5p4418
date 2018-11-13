
cpuinfo_test

	build:
		>  arm-cortex_a9-linux-gnueabi-gcc -o cpuinfo_test cpuinfo_test.c

	test:
		cpuinfo_test -c <process count>

		ex> cpuinfo_test -c 4

		description:
			execute "grep processor /proc/cpuinfo | wc -l" and compare with input parameter <process count>
			to check smp processor count

		return:
			0 is sucess, Otherwise errno

rtc_suspend_test

	build:
		>  arm-cortex_a9-linux-gnueabi-gcc -o rtc_suspend_test rtc_suspend_test.c

	test:
		rtc_suspend_test -d <rtc device node path> -a <alarm time>

		ex> rtc_suspend_test -d /dev/rtc0 -a 3

		description:
			set rtc alarm (unit sec) and goto excute "echo mem > /sys/power/state" to goto sleep mode
			cpu will wakeup after alarm time.

		return:
			0 is sucess, Otherwise errno


cpu_dvfs_test

	build:
		>  arm-cortex_a9-linux-gnueabi-gcc -o cpu_dvfs_test cpu_dvfs_test.c cpu_lib.c -lz -lpthread

	test:
		refer to help (#> cpu_dvfs_test -h)

		description:
			test for cpu (DVFS)dynamic voltage and frequency scaling based on ASV table.

		return:
			0 is sucess, Otherwise errno


cpu_md_vol

	build:
		>  arm-cortex_a9-linux-gnueabi-gcc -o cpu_md_vol cpu_md_vol.c cpu_lib.c -lz -lpthread

	test:
		refer to help (#> cpu_md_vol -h)

		description:
			change cpu (arm or core) currnet voltage based on  ASV table.

		return:
			0 is sucess, Otherwise errno
