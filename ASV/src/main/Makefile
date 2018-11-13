ifndef TOPDIR
TOPDIR := ../..
endif

########################################################################
# Get Linux Build Enviornment:
include $(TOPDIR)/env.mk

######################################################################
# Build options
INCLUDE += -I./ -I../ASVCommandLib
INCLUDE += -I./zlib/lib/include
LIBRARY	+= -lstdc++ -lpthread

LIBRARY	+= -L./vr/lib -lEGL -lGLESv1_CM -lGLESv2 -lVR
STATIC_LIB :=	./zlib/lib/lib/libz.a \
				./vr/libasv_graphic.a

LIBRARY	+= -L./vpu/lib/ -lnxvidrc
STATIC_LIB +=	./vpu/libasv_vpu.a
STATIC_LIB +=	./vpu/lib/libnxvpu.a
STATIC_LIB +=	./vpu/lib/libnxvmem.a
STATIC_LIB +=	./vpu/lib/libion.a


######################################################################
# Target
COBJS	:= ../ASVCommandLib/asv_command.o \
		   dvfsutil.o

COBJS	+= ./cache/cache_test_app.o

CPPOBJS	:= asv_test.o 
CPPOBJS	+= cpu_test.o 

CFLAGS	+= -g

TARGET := asv_test
DEP_BUILD := Dependency_Build

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(DEP_BUILD) \
	 $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(STATIC_LIB) -o $@ $(LIBRARY)
	cp -a $(TARGET) $(TOPDIR)/bin

$(DEP_BUILD):
	make -C ./vr
	make -C ./vpu

clean:
	make -C ./vr clean
	make -C ./vpu clean
	rm -f $(TARGET) *.o .depend
	rm -f ../ASVCommandLib/*.o
	rm -f ../cache/*.o

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CPPFLAGS) $(INCS) $(SRCS) > .depend
