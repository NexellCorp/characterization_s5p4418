CROSSNAME	:= arm-cortex_a9-linux-gnueabi-
#CROSSNAME	:= 

#########################################################################
#	Toolchain.
#########################################################################
CROSS 	 	:= $(CROSSNAME)
CC 		 	:= $(CROSS)gcc
CPP		 	:= $(CROSS)g++
AR 		 	:= $(CROSS)ar
LD 		 	:= $(CROSS)ld
NM 		 	:= $(CROSS)nm
RANLIB 	 	:= $(CROSS)ranlib
OBJCOPY	 	:= $(CROSS)objcopy
STRIP	 	:= $(CROSS)strip


#########################################################################
#	Library & Header macro
#########################################################################
INCLUDE   	:= 

#########################################################################
# 	Build Options
#########################################################################
OPTS		:= -Wall -O2 -Wextra -Wcast-align -Wno-unused-parameter -Wshadow \
			   -Wwrite-strings -Wcast-qual -fno-strict-aliasing -fstrict-overflow \
			   -fsigned-char -fno-omit-frame-pointer -fno-optimize-sibling-calls
COPTS 		:= $(OPTS)
CPPOPTS 	:= $(OPTS) -Wnon-virtual-dtor

CFLAGS 	 	:= $(COPTS)
CPPFLAGS 	:= $(CPPOPTS)
AFLAGS 		:=

ARFLAGS		:= crv
LDFLAGS  	:=
LIBRARY		:=


#########################################################################
# 	Generic Rules
#########################################################################
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) -c -o $@ $<

