ifndef TOPDIR
TOPDIR := ../..
endif

include $(TOPDIR)/env.mk

######################################################################
# Build options
INCLUDE += -I./

LIBRARY += 

######################################################################
# Target
TARGET	:= wdt_test

######################################################################
# Build
COBJS	:=
COBJS	+= wdt_test.o

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
