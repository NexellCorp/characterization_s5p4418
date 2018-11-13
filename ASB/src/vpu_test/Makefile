ifndef TOPDIR
TOPDIR := ../..
endif

########################################################################
# Get Linux Build Enviornment:
include $(TOPDIR)/env.mk

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I./lib
LIBRARY	+= -L./lib
LIBRARY += -lnxvpu -lnxvmem -lnxvidrc -lion

######################################################################
# Target
COBJS	:= asv_video_test.o
CPPOBJS	:=  

CFLAGS	+= -g

LIBNAME	:= libasv_vpu
#TARGET	:= asv_vpu
TARGET	:= asb_vpu

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
#	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBRARY)
	cp $(TARGET) $(TOPDIR)/bin

clean:
	rm -f $(TARGET) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
