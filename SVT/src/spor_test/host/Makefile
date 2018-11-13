.SUFFIXES : .c .o

CROSS_COMPILE :=
INC = -I../common
#INC =
LIBS =
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -O2 -ggdb $(INC)
LDFLAGS =



OBJS = spor_host.o
SRCS = 

TARGET = spor_host


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

