######################################################################
all: compile_subdirs \
	install

compile_subdirs:
	make -C src

clean:
	make -C src clean
	rm ./bin/*
	sudo make clean -C fs
	rm -rf fs/rootfs-svt
	rm -rf fs/mmc

install: install_file_system \
	install_contents \
	install_images

install_images:
	install -d result
	cp -a ./fs/ramdisk.gz result/root.img.gz
	cp -a ./fs/mmc/* result/
	cp -a ./images/NXBTINFO.SBH result/
	cp -a ./images/NXDATA.SBL result/
	cp -a ./images/NXDATA.TBH result/
	cp -a ./images/u-boot.bin result/NXDATA.TBL
	cp -a ./images/uImage result/uImage

install_file_system:
	tar zxvf fs/rootfs-svt.tar.gz -C fs/
	cp -a bin/* fs/rootfs-svt/root/
	cp -a modules/* fs/rootfs-svt/root/
	cp -a src/main/vpu/lib/libnxvidrc.so fs/rootfs-svt/usr/lib/
	cp -a src/main/vr/lib/* fs/rootfs-svt/usr/lib/
	cp -a script/mdev.conf fs/rootfs-svt/etc/mdev.conf
	cp -a script/S90MOUNT fs/rootfs-svt/etc/init.d/S90MOUNT
	cp -a script/S91ASV fs/rootfs-svt/etc/init.d/S91ASV
	chmod 777 fs/rootfs-svt/etc/mdev.conf
	chmod 777 fs/rootfs-svt/etc/init.d/S90MOUNT
	chmod 777 fs/rootfs-svt/etc/init.d/S91ASV
	sudo make -C fs

install_contents:
	install -d fs/mmc
	install -d fs/mmc/video
	install -d fs/mmc/vr
	install -d fs/mmc/audio
	cp -a vectors/vpu/* fs/mmc/video/
	cp -a vectors/vr/*  fs/mmc/vr/
