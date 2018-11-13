#include <stdio.h>
#include <stdlib.h>		/* malloc */
#include <string.h> 	/* strerror */
#include <unistd.h> 	/* read */
#include <errno.h> 		/* errno */
#include <fcntl.h> 		/* for O_RDWR */
#include <sys/time.h> 	/* gettimeofday */
#include <sys/mman.h>	/* for mmap options */
#include <sys/ioctl.h>	/* for ioctl */
#include <linux/fb.h>	/* frame buffer */

#define FB_DEVICE	"/dev/fb0"
#define	DEFAULT_TEST_COUNT		(10)

static int 	fb_open	(const char *name);
static void fb_close(int fd);
static int  fb_mmap	(int fd, unsigned long *base, unsigned long *len);
static void fb_munmap	(unsigned long base, unsigned long len);
static int  fb_get_resol(int fd, int *w, int *h, int *pixbit, int *bufnum);
static int	fb_flip_pan(int fd, int num);

static void print_usage(void)
{
	printf( "usage: options\n"
		 	"-d FB path (default /dev/fb0)	\n"
			"-C test count (default %d)\n"
    		"-T test time (ms)\n"
			, DEFAULT_TEST_COUNT);
}

int main(int argc, char *argv[])
{
	int opt;
	int fb;
	int width = 0, height = 0, pixelbit = 0;
	int bcount = 0;
	unsigned long fbbase = 0, length = 0;
	struct timeval tv1, tv2;
	long us = 0, ms = 0;
	int ret = 0, i = 0, n = 0;

	const char *o_fbPATH = FB_DEVICE;
	int o_count = DEFAULT_TEST_COUNT;

	while (-1 != (opt = getopt(argc, argv, "hd:C:T:"))) {
		switch (opt) {
		case 'd':	o_fbPATH = optarg;	break;
		case 'C':   o_count = strtol(optarg, NULL, 10);	break;
       	case 'T':   printf("-T option is not support\n");	break;
        case 'h':	print_usage();  exit(0);	break;
        default:
        	break;
      	}
	}

	fb = fb_open(o_fbPATH);
	if (0 > fb) {
		ret = errno;
		goto err_end;
	}

	if (0 > fb_mmap(fb, &fbbase, &length)) {
		ret = errno;
		goto err_end;
	}

	if (0 > fb_get_resol(fb, &width, &height, &pixelbit, &bcount)) {
		ret = errno;
		goto err_end;
	}

	printf("%s: virt = 0x%08lx, %8ld (%4d * %4d, %d bpp, %d buffers)\n",
		o_fbPATH, fbbase, length, width, height, pixelbit, bcount);

	for (i=0; o_count > i; i++, n++) {

		gettimeofday(&tv1, NULL);

		n %= bcount;
		ret = fb_flip_pan(fb, n);
		if (0 > ret)
			break;

		gettimeofday(&tv2, NULL);
		us = tv2.tv_usec - tv1.tv_usec;
		if(0 > us) {
			us += 1000000;
			tv2.tv_sec--;
		}
		ms = us/1000;

		if (ms > 20) {
			printf("(%3d)[FB: %2d] = pan time %3ld ms over 20ms\n", i, n, ms);
			ret = -EINVAL;
			break;
		}
		printf("(%3d)[FB: %2d] = %3ld msec\n", i, n, ms);
	}

err_end:
	if (fbbase)
		fb_munmap(fbbase, length);

	if (fb > 0)
		fb_close(fb);

	return ret;
}

static int fb_open(const char *name)
{
	int fd = open(name, O_RDWR);
	if (0 > fd)	{
		printf("Fail: %s open: %s\n", name, strerror(errno));
		return -errno;
	}
	return fd;
}

static void fb_close(int fd)
{
	if (fd >= 0)
		close(fd);
}

static int fb_mmap(int fd, unsigned long *base, unsigned long *len)
{
	void *fb_base = NULL;
	struct fb_var_screeninfo var;
	unsigned long  fb_len;

	if (0 > ioctl(fd, FBIOGET_VSCREENINFO, &var)) {
		printf("Fail: ioctl(0x%x): %s\n", FBIOGET_VSCREENINFO, strerror(errno));
		return -errno;
	}

	fb_len  = (var.xres * var.yres_virtual * var.bits_per_pixel/8);
	fb_base = (void*)mmap(0, fb_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if ((unsigned int)fb_base == (unsigned int)(-1)) {
		printf("Fail: mmap: %s\n", strerror(errno));
		return -errno;
	}

	if (base)
		*base = (unsigned long)fb_base;
	if (len)
		*len  = fb_len;

	return 0;
}

static void fb_munmap(unsigned long base, unsigned long len)
{
	if (base && len)
		munmap((void*)base, len);
}

static int fb_get_resol(int fd, int *width, int *height, int *pixbit, int *bufnum)
{
	struct fb_var_screeninfo var;

	if (0 > ioctl(fd, FBIOGET_VSCREENINFO, &var)){
		printf("Fail: ioctl(0x%x): %s\n", FBIOGET_VSCREENINFO, strerror(errno));
		return -errno;
	}

	if (width)  *width  = var.xres;
	if (height) *height = var.yres;
	if (pixbit) *pixbit = var.bits_per_pixel;
	if (bufnum) *bufnum = var.yres_virtual/var.yres;

	return 0;
}

#if 0
static int fb_set_resol(int fd, int width, int height, int pixbit)
{
	struct fb_var_screeninfo var;

	/*  1. Get var */
	if (0 > ioctl(fd, FBIOGET_VSCREENINFO, &var)){
		printf("Fail: ioctl(0x%x): %s\n", FBIOGET_VSCREENINFO, strerror(errno));
		return -errno;
	}

	var.xres = width;
	var.yres = height;
	var.bits_per_pixel = pixbit;

	/*  2. Set var */
	if (0 > ioctl(fd, FBIOPUT_VSCREENINFO, &var)){
		printf("Fail: ioctl(0x%x): %s\n", FBIOPUT_VSCREENINFO, strerror(errno));
		return -errno;
	}

	return 0;
}
#endif

/*
 * return pan time (milisec)
 */
static int fb_flip_pan(int fd, int num)
{
	struct fb_var_screeninfo var;

	if (0 > ioctl(fd, FBIOGET_VSCREENINFO, &var)){
		printf("Fail: ioctl(0x%x): %s\n", FBIOGET_VSCREENINFO, strerror(errno));
		return -errno;
	}

	if (num > (int)(var.yres_virtual/var.yres))
		return -EINVAL;

	/* change flip buffer */
	var.yoffset = (var.yres * num);

	/* Pan display */
	if (0 > ioctl(fd, FBIOPAN_DISPLAY, &var))
		printf("Fail: ioctl(0x%x): %s\n", FBIOPAN_DISPLAY, strerror(errno));

	return 0;
}
