ifndef TOPDIR
TOPDIR := ../..
endif

include $(TOPDIR)/env.mk

######################################################################
# Build options
INCLUDE += -I./

ifeq ($(CC),gcc)
LIBRARY	+= -lusb
else
INCLUDE += -I./libusb-compat/include

LIBRARY += -L./libusb-compat/lib -lusb
LIBRARY += -L./libusb/lib -lusb-1.0
endif

######################################################################
# Target
TARGET	:= usb_test

######################################################################
# Build
COBJS	:=
COBJS	+= usb_test.o gpio.o

CPPOBJS	:=

OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

clean :
	rm -f $(TARGET) $(UPDATE) *.o .depend

$(TARGET): depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBRARY)
	cp $(TARGET) $(TOPDIR)/bin
	@echo "done"

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif
SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
