#include <stdio.h>
#include <string.h> 	/* strerror */
#include <errno.h> 		/* errno */
#include <fcntl.h> 			/* O_RDWR */
#include <errno.h> 			/* error */
#include <sys/ioctl.h> 		/* ioctl */
#include <sys/mman.h> 		/* for mmap */
#include <linux/fb.h>	/* frame buffer */

struct iomap_table {
	unsigned int phys;
	unsigned int virt;
	unsigned int size;
};

#define	IO_PHYS(_t)			((_t)->phys)
#define	IO_VIRT(_t)			((_t)->virt)
#define	IO_SIZE(_t)			((_t)->size)
#define	IO_PTOV(_t, _p)		((_t)->virt + (_p - (_t)->phys))

#define	MMAP_ALIGN		4096	// 0x1000

#define	MMAP_DEVICE		"/dev/mem"
#define FB_DEVICE		"/dev/fb0"

unsigned long iomem_map (unsigned long phys, unsigned long len);
void iomem_free(unsigned long virt, unsigned long len);

int  fbmem_map(unsigned long *base, unsigned long *len);
void fbmem_free(int fd, unsigned long base, unsigned long len);
