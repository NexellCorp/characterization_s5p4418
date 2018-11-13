ifndef TOPDIR
TOPDIR := ../..
endif

########################################################################
# Get Linux Build Enviornment:
include $(TOPDIR)/env.mk

#SRCS += $(wildcard *.c)
SRCS += $(wildcard *.cpp)
#OBJS += $(patsubst %.c,%.o,$(SRCS))
OBJS += $(patsubst %.cpp,%.o,$(SRCS))

TARGET = svt_hdmi

CFLAGS += -O2 -g -Wall
CFLAGS += -I./include

LDFLAGS := -L./lib -lion -lv4l2-nexell -lm -lstdc++

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	cp $(TARGET) $(TOPDIR)/bin

%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o : %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf *.o
	rm -rf $(TARGET)
