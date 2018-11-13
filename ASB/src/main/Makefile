ifndef TOPDIR
TOPDIR := ../..
endif

include $(TOPDIR)/env.mk

######################################################################
# Build options
INCLUDE += -I./

LIBRARY += -lstdc++

######################################################################
# Target
TARGET	:= asb_main

######################################################################
# Build
COBJS	:=
CPPOBJS	:=

CPPOBJS	+= ecid.o
CPPOBJS	+= spi.o
CPPOBJS	+= asb_protocol.o
CPPOBJS	+= nx_gpio.o
CPPOBJS	+= main.o

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
