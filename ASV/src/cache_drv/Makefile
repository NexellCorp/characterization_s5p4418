######################################################################
# Kernel path
KERNDIR := ~/devel/android/kitkat-mr1/linux/kernel/kernel-3.4.39
ARCH	:= arm

######################################################################
# Target
obj-m := cache_test_mod.o

######################################################################
# Build
ifeq "$(KDIR)" ""
KDIR := $(KERNDIR)
endif

PWD  := $(shell pwd)

# add iio header
KBUILD_CFLAGS	+= -I$(srctree)

default:
	@echo "==========================================================================="
	@echo "KDIR : $(KDIR)";
	@echo "ARCH : $(ARCH)";
	@echo "==========================================================================="
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(PWD) modules
	cp cache_test_mod.ko ../../modules

clean:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(PWD) clean;
