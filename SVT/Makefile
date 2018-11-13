######################################################################
all: compile_subdirs 

compile_subdirs:
	mkdir -p bin
	make -C src

clean:
	make -C src clean
	rm ./bin/*

install: install_file_system \
	install_contents \
	install_images \
	install_android_images

install_images:
	install -d result
	cp -a ./fs/ramdisk.gz result/root.img.gz
	cp -a ./fs/mmc/* result/
	cp -a ./images/NXDATA.SBH result/
	cp -a ./images/NXDATA.SBH result/NXBTINFO.SBH
	cp -a ./images/NXDATA.SBL result/
	cp -a ./images/NXDATA.TBH result/
	cp -a ./images/2ndboot_spi.bin result/
	cp -a ./images/u-boot.bin result/NXDATA.TBL
	cp -a ./images/uImage result/

install_android_images:
	install -d result_android
	cp -a ./images/NXDATA.SBH result_android/
	cp -a ./images/NXDATA.SBH result_android/NXBTINFO.SBH
	cp -a ./images/NXDATA.SBL result_android/
	cp -a ./images/NXDATA.TBH result_android/
	cp -a ./images/2ndboot_spi.bin result_android/
	cp -a ./images/u-boot.bin result_android/NXDATA.TBL
	cp -a ./images/android/uImage result_android/
	cp -a ./images/android/root.img.gz result_android/
	cp -a ./images/android/ramdisk-recovery.img result_android/
	cp -a ./images/android/battery.bmp result_android/
	cp -a ./images/android/logo.bmp result_android/
	cp -a ./images/android/update.bmp result_android/

install_file_system:
	tar zxvf fs/rootfs-svt.tar.gz -C fs/
	cp -a bin/* fs/rootfs-svt/root/
	cp -a modules/* fs/rootfs-svt/root/
	cp -a src/vpu_test/lib/libnxvidrc.so fs/rootfs-svt/usr/lib/
	cp -a src/vr_test/lib/* fs/rootfs-svt/usr/lib/
	cp -a src/vip/lib/* fs/rootfs-svt/usr/lib/
	cp -a script/mdev.conf fs/rootfs-svt/etc/mdev.conf
	cp -a script/S90MOUNT fs/rootfs-svt/etc/init.d/S90MOUNT
	cp -a script/S91SVT fs/rootfs-svt/etc/init.d/S91SVT
	chmod 777 fs/rootfs-svt/etc/mdev.conf
	chmod 777 fs/rootfs-svt/etc/init.d/S90MOUNT
	chmod 777 fs/rootfs-svt/etc/init.d/S91SVT
	sudo make -C fs

install_contents:
	install -d fs/mmc
	install -d fs/mmc/video
	install -d fs/mmc/vr
	install -d fs/mmc/audio
	cp -a vectors/vpu/* fs/mmc/video/
	cp -a vectors/vr/*  fs/mmc/vr/
	cp -a vectors/i2s/* fs/mmc/audio/

distclean:
	make -C src clean
	rm ./bin/*
	sudo make clean -C fs
	rm -rf fs/rootfs-svt
	rm -rf fs/mmc
