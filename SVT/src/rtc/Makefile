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
RTC_ALARM_TEST	:= rtc_alarm_test

######################################################################
# Build
OBJ_CPU	:= rtc_alarm_test.o

COBJS   := $(OBJ_CPU)
CPPOBJS	:=

OBJS	:= $(COBJS) $(CPPOBJS)

all: depend $(OBJS) \
	$(RTC_ALARM_TEST)

$(RTC_ALARM_TEST):
	$(CC) $(CFLAGS) $(OBJ_CPU) -o $(RTC_ALARM_TEST) $(LIBRARY)
	cp -f $(RTC_ALARM_TEST) $(TOPDIR)/bin/$(RTC_ALARM_TEST)
	@echo "done"

clean :
	rm -f $(RTC_ALARM_TEST) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif
SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
