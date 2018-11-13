#
# 	make a root $(ROOTFS) image.
#
#	#> make
#	- build default rootfs is ROOTFSNAME at defined build.env
#
#	#> make ROOTFS=xxx
#	- build user define rootfs (xxx)
#
########################################################################
# Get Linux Build Enviornment:
#include build.env

######################################################################
# build target
INSTDIR := $(TFTPDIR)

ROOTFS 	:= rootfs-svt
FSTYPE	:= ext2

# ramdisk params
#RAMSIZE := 24576
#RAMSIZE := 32768 
RAMSIZE := 16384
MNTDISK := mnt
LOOPDEV := loop3

# target params
RAMDISK	:= ramdisk
YAFFS2  := yaffs2
CRAMFS  := cramfs
UBIFS   := ubifs

# build target
TARGET 	:= ramdisk
#TARGET += yaffs2
#TARGET += cramfs
#TARGET += ubifs

all: $(TARGET)

#########################################################################
# build ramdisk image

ramdisk:
	@echo "ROOTFS: $(ROOTFS) ..."
	dd if=/dev/zero of=$(RAMDISK) bs=1k count=$(RAMSIZE)
	losetup    /dev/$(LOOPDEV) $(RAMDISK)
	mke2fs     /dev/$(LOOPDEV)
	sleep 1
	losetup -d /dev/$(LOOPDEV)
	mkdir -p $(MNTDISK)
	mount -t $(FSTYPE) -o loop $(RAMDISK) $(MNTDISK)
	cp -dR $(ROOTFS)/* $(MNTDISK)/
	sleep 3
	umount $(MNTDISK)
	gzip -f $(RAMDISK)
	rm -rf $(MNTDISK)

#########################################################################
# build yaffs2 image

yaffs2:
	@echo "ROOTFS: $(ROOTFS) ..."
	mkyaffs2image $(ROOTFS) $(YAFFS2).img
	chmod 644 $(YAFFS2).img

#########################################################################
# build cramfs image

cramfs:
	@echo "ROOTFS: $(ROOTFS) ..."
	mkcramfs $(ROOTFS) $(CRAMFS).img
	chmod 644 $(CRAMFS).img

#########################################################################
# build UBIFS image

ubifs:
	@echo "ROOTFS: $(ROOTFS) ..."
	mkfs.ubifs -v -r $(ROOTFS) $(UBIFS).img -m
	chmod 644 $(CRAMFS).img

#########################################################################
# Clean and Install filesystem images

PWD		:= $(shell /bin/pwd)
MNTSTAT := $(shell df |  grep $(PWD)/mnt)

clean :
	rm -f *.img  $(TARGET) $(RAMDISK).gz
	@if [ -n "$(MNTSTAT)" ]; \
		then umount mnt; \
		fi

IMGOBJ += $(RAMDISK).gz
IMGOBJ += $(YAFFS2).img
IMGOBJ += $(CRAMFS).img

IMAGES := $(shell ls $(IMGOBJ) 2> /dev/null)

install:
	@echo "Install files: $(IMAGES) to $(INSTDIR)"
	@if [ -n "$(IMAGES)" ]; \
		then \
			echo "Copy $(IMAGES) to $(INSTDIR)"; \
			cp -f $(IMAGES) $(INSTDIR); \
		else \
			echo "Not exist filesystem image..."; \
		fi

