.SUFFIXES : .c .o

CROSS_COMPILE := arm-cortex_a9-linux-gnueabi-
#CROSS_COMPILE :=
INC = -I../common
#INC =
LIBS =
CC = $(CROSS_COMPILE)gcc
#CFLAGS = -DDEBUG -O2 $(INC)
CFLAGS = -O2 $(INC)
LDFLAGS = -static



OBJS = spor_target.o ../common/nx_lib.o
SRCS = 

TARGET = spor_target


.PHONY: clean

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

dep :
	gccmakedep $(INC) $(SRCS)

clean :
	rm -rf $(OBJS) $(TARGET) core

new : 
	$(MAKE) clean 
	$(MAKE) 


