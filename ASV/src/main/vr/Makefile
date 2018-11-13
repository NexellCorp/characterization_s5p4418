########################################################################
# Get Linux	Build Enviornment:
include	../../../env.mk

######################################################################
# Build	options
INCLUDE	+= -I./
INCLUDE	+= -I./vr_headers/include
INCLUDE	+= -I./vr_headers/include/EGL/platform_fbdev/
INCLUDE	+= -I./vr_headers/3rdparty/include/khronos

LIBRARY	+= -L./lib/ -lEGL -lGLESv1_CM  -ldl -lm
#LIBRARY	+= -L$(LIB_INSTALL) -L../../../mali400/driver/lib/ -lVR -lEGL -lGLESv1_CM  -ldl -lm
LIBRARY	+= -lstdc++	-lpthread

######################################################################
# Target
COBJS	:=
CPPOBJS	:= OGLEarthSphere.o ES_texture.o V6_sphere_980.o V6_sphere_9680.o Fake_OS.o GLFont.o __font_modify_tga.o 
#CPPOBJS += main.o
LIBNAME	:= libasv_graphic
TARGET	:= avs_graphic

#CPPFLAGS += -g

######################################################################
# Build
OBJS	:= $(COBJS)	$(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJS)
#	$(CC) $(CPPFLAGS) $(OBJS) -o $@	$(LIBRARY)
#	cp -f $(TARGET)	../../release/
#	sudo cp -f $(TARGET)	../../../../../../devel/nfs/kernel_rootfs-nxp4330/test/

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name $(LIBNAME).a -o -name	'*.map'	\) -print \
		| xargs	rm -f

nfs:
	sudo cp -f $(TARGET)	../../../../../../devel/nfs/kernel_rootfs-nxp4330/test/

#########################################################################
# Dependency
ifeq (dependency_list_$(TARGET),$(wildcard dependency_list_$(TARGET)))
include	dependency_list_$(TARGET)
endif

SRCS :=	$(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS :=	$(INCLUDE)
depend dep:
	$(CC)  -M  $(CPPFLAGS) $(CFLAGS) $(INCS) $(INCLUDE)	$(SRCS)	> dependency_list_$(TARGET)



