######################################################################
SUB_DIRS	:=
SUB_DIRS	+= cpu
SUB_DIRS	+= gpio
SUB_DIRS	+= i2c
SUB_DIRS	+= i2s_test
SUB_DIRS	+= spdif_test
SUB_DIRS	+= mmc
SUB_DIRS	+= uart_test
SUB_DIRS	+= spirom
SUB_DIRS	+= rtc
SUB_DIRS	+= usb_test
SUB_DIRS	+= pwm_ppm
SUB_DIRS	+= nand_test
SUB_DIRS	+= gmac_test
SUB_DIRS	+= adc_test
SUB_DIRS	+= wdt_test
SUB_DIRS	+= sfr_test
SUB_DIRS	+= display
SUB_DIRS	+= vpu_test
SUB_DIRS	+= vr_test
SUB_DIRS	+= mipi_test
SUB_DIRS	+= ro_test
SUB_DIRS	+= main

all: compile_subdirs

compile_subdirs:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir || exit $?;	\
	done

clean:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir clean;		\
	done
