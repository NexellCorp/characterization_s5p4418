
nand_test

	build:
		>  arm-cortex_a9-linux-gnueabi-gcc -o nand_test nand_test.c

	test:
		nand_test

		ex> nand_test

		description:
			read "/sys/devices/virtual/mtd/mtd0/type" and "/sys/devices/virtual/mtd/mtd0/erasesize"
			to check nand probe success.

		return:
			EXIT_SUCCESS(0) is sucess, Otherwise errno.

