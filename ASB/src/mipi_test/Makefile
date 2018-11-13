ifndef TOPDIR
TOPDIR := ../..
endif

include $(TOPDIR)/env.mk

FRAMEWORKDIR	:= framework
PROTOTYPEDIR	:= prototype
######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I$(FRAMEWORKDIR)/include
INCLUDE += -I$(FRAMEWORKDIR)/config
INCLUDE += -I$(PROTOTYPEDIR)/nx_base
INCLUDE += -I$(PROTOTYPEDIR)/module

LIBRARY	+= -L./prototype -lprototype
CFLAGS	+= -D__LINUX__ -DNX_RELEASE
CFLAGS  += -static
######################################################################
# Target
COBJS  	:= main.o map.o
TARGET 	:= mipi_test
SUB_DIR	:= $(PROTOTYPEDIR)

######################################################################
# Build
OBJS	:= $(COBJS)

all: $(TARGET)

build_subdir:
	@for dir in $(SUB_DIR); do \
	make -C $$dir || exit $?;	\
	done

$(TARGET): depend $(OBJS) build_subdir
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBRARY)
	cp -f $(TARGET) $(TOPDIR)/bin/

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name '*.map' -o -name '*.a' -o -name '*.so.*' \) \
		-print | xargs rm -f

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif
SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CFLAGS)   $(INCS) $(SRCS) > .depend