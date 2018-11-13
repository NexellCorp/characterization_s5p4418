########################################################################
# Get Linux Build Enviornment:
include ../../../env.mk

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I./lib
LIBRARY	+= -L./lib
LIBRARY += -lnxvpu -lnxvmem -lnxvidrc

######################################################################
# Target
COBJS	:= asv_video_test.o
CPPOBJS	:=  

CFLAGS	+= -g

LIBNAME	:= libasv_vpu
TARGET	:= asv_vpu

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJS)
#	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBRARY)

clean:
	rm -f $(TARGET) $(LIBNAME).a *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
