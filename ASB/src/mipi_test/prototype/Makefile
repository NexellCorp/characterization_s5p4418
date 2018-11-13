ifndef TOPDIR
TOPDIR := ../../..
endif

include $(TOPDIR)/env.mk

MODULES	  := module
NX_BASE	  := nx_base

######################################################################
# Build options

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS	 	+= -fpic
CFLAGS	 	+= -D__LINUX__ -DNX_RELEASE
INCLUDE 	+= -I$(NX_BASE)
INCLUDE 	+= -I$(MODULES)

######################################################################
# Target
COBJS  	+= $(MODULES)/nx_clkgen.o
COBJS  	+= $(MODULES)/nx_mipi.o
COBJS  	+= $(MODULES)/nx_vip.o
COBJS  	+= $(MODULES)/nx_tieoff.o
COBJS  	+= $(MODULES)/nx_rstcon.o

LIBNAME := libprototype

SO_MAJ 	:= .1
SO_MIN	:= .1

SO_NAME	:= $(LIBNAME).so$(SO_MAJ)
TARGET	:= $(LIBNAME).so$(SO_MAJ)$(SO_MIN)
######################################################################
# Build
OBJS	:= $(COBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $(OBJS) $(LIBRARY)
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJS)
	chmod 644 $(TARGET)
	chmod 644 $(LIBNAME).a

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.a' -o -name '*.so.*' -o \
		-name '*.bin' -o -name '*.map' -o -name 'core' -o -name '*~' \) -print \
		| xargs rm -f


#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend
