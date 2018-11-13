adc_test

	build:
		>  arm-cortex_a9-linux-gnueabi-gcc -o adc_test adc_test.c

	test:
		Usage: adc_test
                [-m TestMode]        (ASB or VTK. default:VTK)
                [-t torelance]       (default:10)
                [-n sample count]    (default:50)
                [-v voltage]         (0~1.8V. default:1.800000)
                [-c channel]         (0~8. 0:all, else:channel. default:0)

		ex> (ASB)
				./adc_test -m ASB -t 5 -n 10

			(VTK)
				./adc_test -m VTK -t 10 -n 50 -v 1.2 -c 3

		description:
			read "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
			   ~ "/sys/bus/iio/devices/iio:device0/in_voltage7_raw"
			 to check get voltage value success.

			'ASB mode' check channel 1 to 6 with preset voltage.
				[0] = 0.2,
				[1] = 0.3,
				[2] = 0.4,
				[3] = 0.5,
				[4] = 0.6,
				[5] = 0.9


		return:
			EXIT_SUCCESS(0) is sucess, Otherwise errno

