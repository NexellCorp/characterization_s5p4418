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
CPUINFO	:= cpu_cache_test

######################################################################
# Build
OBJ_CPU	:= cache_test_app.o 

COBJS   := $(OBJ_CPU)
CPPOBJS	:=

OBJS	:= $(COBJS) $(CPPOBJS)

all: depend $(OBJS) \
	$(CPUINFO)

$(CPUINFO):
	$(CC) $(CFLAGS) $(OBJ_CPU) -o $(CPUINFO) $(LIBRARY)
	cp $(CPUINFO) $(TOPDIR)/bin
	@echo "done"

clean :
	rm -f $(CPUINFO) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif
SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
