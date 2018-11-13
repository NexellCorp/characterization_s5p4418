
gmac_test

	build:
		>  arm-cortex_a9-linux-gnueabi-gcc -o gmac_test gmac_test.c

	test:
		gmac_test

		ex> gmac_test

		description:
			send udp raw packet to eth0 interface and loopback receive
			to check nand probe success.

		return:
			EXIT_SUCCESS(0) is sucess, Otherwise EXIT_FAILURE.

