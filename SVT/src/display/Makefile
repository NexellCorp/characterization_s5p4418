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
FB_DRAW	:= fb_draw

######################################################################
# Build
OBJ_FBP	:= fb_draw.o

COBJS   := $(OBJ_FBP)
CPPOBJS	:=

OBJS	:= $(COBJS) $(CPPOBJS)

all: depend $(OBJS) \
	$(FB_DRAW)

$(FB_DRAW):
	$(CC) $(CFLAGS) $(OBJ_FBP) -o $(FB_DRAW) $(LIBRARY)
	cp -f $(FB_DRAW) $(TOPDIR)/bin/$(FB_DRAW)
	@echo "done"

clean :
	rm -f $(FB_DRAW) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif
SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
