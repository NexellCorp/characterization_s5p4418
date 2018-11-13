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

static int 	fb_open(const char *name);
static void fb_close(int fd);
static int  fb_mmap(int fd, unsigned int *base, unsigned int *len);
static void fb_munmap(unsigned int base, unsigned int len);
static int  fb_get_resol(int fd, int *w, int *h, int *pixbit, int *bufnum);
static void fill_color(unsigned int base, int xres, int yres, unsigned int pixbyte, unsigned int color);

static void print_usage(void)
{
	printf( "usage: options\n"
		 	"-d FB path (default /dev/fb0)	\n"
			"-f Fill color to FB\n"
			);
}

#define	PAGE_SIZE				(4096)
#define	PAGE_SIZE_ALIGN(l, a)	(((l+(a-1))/a)*a)

int main(int argc, char *argv[])
{
	int opt;
	int fb;
	unsigned int fb_base = 0, fb_size = 0;
	int fb_xres = 0, fb_yres = 0, fb_pixbit = 0, fb_bufs = 0;

	int ret = 0, i = 0;

	const char *o_fb_path = FB_DEVICE;
	int o_color = 0xFF0000;

	while (-1 != (opt = getopt(argc, argv, "hd:f:"))) {
		switch (opt) {
		case 'd':	o_fb_path = optarg;	break;
		case 'f':   o_color = strtol(optarg, NULL, 16);	break;
        case 'h':	print_usage();  exit(0);	break;
        default:
        	break;
      	}
	}
	printf("= Fill Color = 0x%x =\n", o_color);
	fb = fb_open(o_fb_path);
	if (0 > fb) {
		ret = errno;
		goto err_end;
	}

	if (0 > fb_mmap(fb, &fb_base, &fb_size)) {
		ret = errno;
		goto err_end;
	}

	if (0 > fb_get_resol(fb, &fb_xres, &fb_yres, &fb_pixbit, &fb_bufs)) {
		ret = errno;
		goto err_end;
	}

	printf("%s: virt = 0x%08x, %8d (%4d * %4d, %d bpp, %d buffers) ....\n",
		o_fb_path, fb_base, fb_size, fb_xres, fb_yres, fb_pixbit, fb_bufs);

	if (fb_size%PAGE_SIZE)
		printf("%s: Warning fb size not aligned %d\n", o_fb_path, PAGE_SIZE);

	for (i = 0; fb_bufs > i; i++) {
		unsigned int base = PAGE_SIZE_ALIGN(fb_base + i*(fb_xres * fb_yres * fb_pixbit/8), PAGE_SIZE);
		fill_color(base, fb_xres, fb_yres, fb_pixbit/8, o_color);
	}

err_end:
	if (fb_base)
		fb_munmap(fb_base, fb_size);

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

static int fb_mmap(int fd, unsigned int *base, unsigned int *len)
{
	void *addr = NULL;
	struct fb_var_screeninfo var;
	unsigned long  size;

	if (0 > ioctl(fd, FBIOGET_VSCREENINFO, &var)) {
		printf("Fail: ioctl(0x%x): %s\n", FBIOGET_VSCREENINFO, strerror(errno));
		return -errno;
	}

	size  = (var.xres * var.yres_virtual * var.bits_per_pixel/8);
	addr = (void*)mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if ((unsigned int)addr == (unsigned int)(-1)) {
		printf("Fail: mmap: %s\n", strerror(errno));
		return -errno;
	}

	if (addr)
		*base = (unsigned int)addr;
	if (len)
		*len  = size;

	return 0;
}

static void fb_munmap(unsigned int base, unsigned int len)
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
 * Fill color
 */
#define	RGB555TO565(col) 	(((col>>10)&0x1F) << 11) | (((col>> 5)&0x1F) << 6) | ((col<< 0)&0x1F)

static void pixel_555to565(unsigned int base, int xpos, int ypos,
			int width, int height, unsigned int color)
{
	*(unsigned short*)(base + (ypos * width + xpos) * 2) = (unsigned short)RGB555TO565(color);
}

static void pixel_565to888(unsigned int base, int xpos, int ypos,
			int width, int height, unsigned int color)
{
	*(unsigned char*)(base + (ypos * width + xpos) * 3 + 0) =
			(((color >> 0 ) << 3) & 0xf8) | (((color >> 0 ) >> 2) & 0x7);	// B
	*(unsigned char*)(base + (ypos * width + xpos) * 3 + 1) =
			(((color >> 5 ) << 2) & 0xfc) | (((color >> 5 ) >> 4) & 0x3);	// G
	*(unsigned char*)(base + (ypos * width + xpos) * 3 + 2) =
			(((color >> 11) << 3) & 0xf8) | (((color >> 11) >> 2) & 0x7);	// R
}

static void pixel_565to8888(unsigned int base, int xpos, int ypos,
			int width, int height, unsigned int color)
{
	*(unsigned char*)(base + (ypos * width + xpos) * 4 + 0) =
			(((color >> 0 ) << 3) & 0xf8) | (((color >> 0 ) >> 2) & 0x7);	// B
	*(unsigned char*)(base + (ypos * width + xpos) * 4 + 1) =
			(((color >> 5 ) << 2) & 0xfc) | (((color >> 5 ) >> 4) & 0x3);	// G
	*(unsigned char*)(base + (ypos * width + xpos) * 4 + 2) =
			(((color >> 11) << 3) & 0xf8) | (((color >> 11) >> 2) & 0x7);	// R
	*(unsigned char*)(base + (ypos * width + xpos) * 4 + 3) = 0;	// Alpha
}

#define	RGB888TO565(col) 	((((col>>16)&0xFF)&0xF8)<<8) | ((((col>>8)&0xFF)&0xFC)<<3) | ((((col>>0 )&0xFF)&0xF8)>>3)

static void pixel_888to565(unsigned int base, int xpos, int ypos,
			int width, int height, unsigned int color)
{
	*(unsigned short*)(base + (ypos * width + xpos) * 2) = (unsigned short)RGB888TO565(color);
}

static void pixel_888to888(unsigned int base, int xpos, int ypos,
			int width, int height, unsigned int color)
{
	base = base + (ypos * width + xpos) * 3;
	*(unsigned char*)(base++) = ((color>> 0)&0xFF);	// B
	*(unsigned char*)(base++) = ((color>> 8)&0xFF);	// G
	*(unsigned char*)(base)   = ((color>>16)&0xFF);	// R
}

static void pixel_888to8888(unsigned int base, int xpos, int ypos,
			int width, int height, unsigned int color)
{
	*(unsigned int*)(base + (ypos * width + xpos) * 4) = (0xFF000000) | (color & 0xFFFFFF);
}

static void (*PUTPIXELTABLE[])(unsigned int, int, int, int, int, unsigned int) = {
	pixel_555to565,
	pixel_565to888,
	pixel_565to8888,
	pixel_888to565,
	pixel_888to888,
	pixel_888to8888,
};

/*
 * Fill color to FB
 */
static void fill_color(unsigned int base, int xres, int yres, unsigned int pixbyte, unsigned int color)
{
	void (*pixel_)(unsigned int, int, int, int, int, unsigned int) = NULL;
	int x = 0, y = 0;

	// 888to565 or 888to888
	pixel_ = PUTPIXELTABLE[3 + pixbyte - 2];

	printf("Fill FB=0x%08x, x=%4d, y=%4d, bpp=%d, col=0x%08x\n",
		base, xres, yres, pixbyte*8, color);

	/* clear */
	for (y = 0; yres > y; y++)
	for (x = 0; xres > x; x++)
		pixel_(base, x, y, xres, yres, color);
}
