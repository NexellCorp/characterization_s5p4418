#include "map.h"

//======================================================================================================
unsigned long iomem_map(unsigned long phys, unsigned long len)
{
	unsigned long virt = 0;
	int fd;

	fd = open(MMAP_DEVICE, O_RDWR|O_SYNC);
	if (0 > fd) {
		printf("Fail, open %s, %s\n", MMAP_DEVICE, strerror(errno));
		return 0;
	}

	if (len & (MMAP_ALIGN-1))
		len = (len & ~(MMAP_ALIGN-1)) + MMAP_ALIGN;

	virt = (unsigned long)mmap((void*)0, (size_t)len,
				PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)phys);
	if (-1 == (long)virt) {
		printf("Fail: map phys=0x%08lx, len=%ld, %s \n", phys, len, strerror(errno));
		goto _err;
	}

_err:
	close(fd);
	return virt;
}

void iomem_free(unsigned long virt, unsigned long len)
{
	if (virt && len)
		munmap((void*)virt, len);
}

//======================================================================================================
int fbmem_map(unsigned long *base, unsigned long *len)
{
	struct fb_var_screeninfo var;
	char *dev = FB_DEVICE;
	unsigned long size;

	if (NULL == base)
		return -EINVAL;

	int fd = open(dev, O_RDWR);
	if (0 > fd)	{
		printf("Fail: %s open: %s\n", dev, strerror(errno));
		return -errno;
	}

	if (0 > ioctl(fd, FBIOGET_VSCREENINFO, &var)) {
		printf("Fail: ioctl(0x%x): %s\n", FBIOGET_VSCREENINFO, strerror(errno));
		return -errno;
	}

	size = (var.xres * var.yres_virtual * var.bits_per_pixel/8);
	*base = (unsigned long*)mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if ((unsigned long)*base == (unsigned long)(-1)) {
		printf("Fail: mmap: %s\n", strerror(errno));
		return -errno;
	}

	if (len)
		*len  = size;

	return fd;
}

void fbmem_free(int fd, unsigned long base, unsigned long len)
{
	if (base && len)
		munmap((void*)base, len);

	if (0 > fd)
		return;

	close(fd);
}

