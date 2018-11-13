#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for open/close .. */
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h> // ETIMEDOUT

#define CACHE_DEV_NODE "/dev/cache_test"

#if 1
int main(int argc, char **argv)
{
	int fd, ret;
	char path[32] = CACHE_DEV_NODE;
	int value;

	if (argc > 2)
		strcpy(path, argv[1]);

	fd = open(path, O_RDWR);
	if (0 > fd) {
		printf("Fail: %s open (%s)\n", path, strerror(errno));
		return -1;
	}

	ret = write(fd, &value, sizeof(value));
	if (0 > ret) {
		printf("fail, %s read (%s)\n", path, strerror(errno));
		return -1;
	}

	close(fd);
	return 0;
}
#else

int test_cpu_cache( void )
{
	int fd, ret;
	int value;
	fd = open(CACHE_DEV_NODE, O_RDWR);
	if (0 > fd) {
		printf("Fail: %s open (%s)\n", CACHE_DEV_NODE, strerror(errno));
		return -1;
	}

	ret = write(fd, &value, sizeof(value));
	if (0 > ret) {
		printf("fail, %s read (%s)\n", CACHE_DEV_NODE, strerror(errno));
		return -1;
	}
	close(fd);
	return 0;
}

#endif
