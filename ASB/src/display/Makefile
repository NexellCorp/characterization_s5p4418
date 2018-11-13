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
FB_PAN_TEST	:= fb_pan_test

######################################################################
# Build
OBJ_FBP	:= fb_pan_test.o

COBJS   := $(OBJ_FBP)
CPPOBJS	:=

OBJS	:= $(COBJS) $(CPPOBJS)

all: depend $(OBJS) \
	$(FB_PAN_TEST)

$(FB_PAN_TEST):
	$(CC) $(CFLAGS) $(OBJ_FBP) -o $(FB_PAN_TEST) $(LIBRARY)
	cp -f $(FB_PAN_TEST) $(TOPDIR)/bin/$(FB_PAN_TEST)
	@echo "done"

clean :
	rm -f $(FB_PAN_TEST) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif
SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
