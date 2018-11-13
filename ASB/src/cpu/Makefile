ifndef TOPDIR
TOPDIR := ../..
endif

include $(TOPDIR)/env.mk

######################################################################
# Build options
INCLUDE += -I./
LIBRARY += -lz -lpthread

######################################################################
# Target
CPUINFO	:= cpuinfo_test
CPUDVFS	:= cpu_dvfs_test
CPUMDVOL:= cpu_md_vol
CPUMDFRQ:= cpu_md_speed
RTC_SUSPEND	:= rtc_suspend_test

######################################################################
# Build
OBJ_CPU	:= cpuinfo_test.o
OBJ_DFS	:= cpu_dvfs_test.o cpu_lib.o
OBJ_VOL	:= cpu_md_vol.o	cpu_lib.o
OBJ_FRQ	:= cpu_md_speed.o cpu_lib.o
OBJ_RTC	:= rtc_suspend_test.o

COBJS   := $(OBJ_CPU) $(OBJ_DFS) $(OBJ_RTC) $(OBJ_VOL) $(OBJ_FRQ)
CPPOBJS	:=

OBJS	:= $(COBJS) $(CPPOBJS)

all: depend $(OBJS) \
	$(CPUINFO) 	\
	$(CPUDVFS) 	\
	$(CPUMDVOL) \
	$(CPUMDFRQ) \
	$(RTC_SUSPEND)

$(CPUINFO):
	$(CC) $(CFLAGS) $(OBJ_CPU) -o $(CPUINFO) $(LIBRARY)
	cp -f $(CPUINFO) $(TOPDIR)/bin/
	@echo "done"

$(CPUDVFS):
	$(CC) $(CFLAGS) $(OBJ_DFS) -o $(CPUDVFS) $(LIBRARY)
	cp -f $(CPUDVFS) $(TOPDIR)/bin/
	@echo "done"

$(CPUMDVOL):
	$(CC) $(CFLAGS) $(OBJ_VOL) -o $(CPUMDVOL) $(LIBRARY)
	cp -f $(CPUMDVOL) $(TOPDIR)/bin/
	@echo "done"

$(CPUMDFRQ):
	$(CC) $(CFLAGS) $(OBJ_FRQ) -o $(CPUMDFRQ) $(LIBRARY)
	cp -f $(CPUMDFRQ) $(TOPDIR)/bin/
	@echo "done"

$(RTC_SUSPEND):
	$(CC) $(CFLAGS) $(OBJ_RTC) -o $(RTC_SUSPEND) $(LIBRARY)
	cp -f $(RTC_SUSPEND) $(TOPDIR)/bin/
	@echo "done"

clean :
	rm -f $(CPUINFO) $(CPUDVFS) $(RTC_SUSPEND) $(CPUMDVOL) $(CPUMDFRQ) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif
SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
