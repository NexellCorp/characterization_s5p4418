

CROSSTOOL_PATH = /opt/crosstoosl

	1. copy arm-cortex_a9-eabi-4.7-eglibc-2.18-splita* to <CROSSTOOL_PATH>

	2. move to <CROSSTOOL_PATH> and uncompress arm-cortex_a9-eabi-4.7-eglibc-2.18-splita*

		cat arm-cortex_a9-eabi-4.7-eglibc-2.18-splita* | tar -zxvpf - 

	3. set environment

		export PATH=$PATH:/opt/crosstools/arm-cortex_a9-eabi-4.7-eglibc/bin
