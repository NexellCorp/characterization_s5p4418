#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <time.h>		// ctime

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nand_test.h"

#define nx_debug(fmt, ...)			\
		if (g_debug == 1)			\
			printf(fmt, ##__VA_ARGS__)


int g_repeat = DEF_REPEAT;

int g_debug = 0;					/* print debug message. hidden */

void print_info(struct stat *sb)
{

	if (!sb) return;

	printf("File type:                ");

	switch (sb->st_mode & S_IFMT) {
		case S_IFBLK:  printf("block device\n");            break;
		case S_IFCHR:  printf("character device\n");        break;
		case S_IFDIR:  printf("directory\n");               break;
		case S_IFIFO:  printf("FIFO/pipe\n");               break;
		case S_IFLNK:  printf("symlink\n");                 break;
		case S_IFREG:  printf("regular file\n");            break;
		case S_IFSOCK: printf("socket\n");                  break;
		default:       printf("unknown?\n");                break;
	}

	printf("I-node number:            %ld\n", (long) sb->st_ino);

	printf("Mode:                     %lo (octal)\n",
			(unsigned long) sb->st_mode);

	printf("Link count:               %ld\n", (long) sb->st_nlink);
	printf("Ownership:                UID=%ld   GID=%ld\n",
			(long) sb->st_uid, (long) sb->st_gid);

	printf("Preferred I/O block size: %ld bytes\n",
			(long) sb->st_blksize);
	printf("File size:                %lld bytes\n",
			(long long) sb->st_size);
	printf("Blocks allocated:         %lld\n",
			(long long) sb->st_blocks);

	printf("Last status change:       %s", ctime(&sb->st_ctime));
	printf("Last file access:         %s", ctime(&sb->st_atime));
	printf("Last file modification:   %s", ctime(&sb->st_mtime));
}


int get_nand_info(struct mtd_local *mtd)
{
	int ret = -1;
	FILE *fp_type, *fp_erasesize;
	char buf[256] = { 0, };

	memset(mtd, 0, sizeof (*mtd));


	fp_type = fopen(SYS_DEV "type", "r");
	if (!fp_type) {
		perror("type open");
		goto err_type;
	}

	fp_erasesize = fopen(SYS_DEV "erasesize", "r");
	if (!fp_erasesize) {
		perror("erasesize open");
		goto err_erasesize;
	}

	fgets((char *)(mtd->type), 32, fp_type);
	fgets(buf, 256, fp_erasesize);
	mtd->erasesize = (uint32_t)strtol(buf, NULL, 10);

	if (strcmp((const char *)(mtd->type), "") == 0 || mtd->erasesize == 0)
		goto err_data;

	ret = 0;

	printf("TYPE: %s", mtd->type);
	//printf("BLOCKSIZE: 0x%x\n", mtd->erasesize);

err_data:
	fclose(fp_erasesize);
err_erasesize:
	fclose(fp_type);
err_type:

	return ret;
}


void print_usage(void)
{
	fprintf(stderr, "Usage: %s\n", PROG_NAME);
	fprintf(stderr, "                [-C test_count]      (default %d)\n", DEF_REPEAT);
	fprintf(stderr, "                [-T not support]\n");
}

int parse_opt(int argc, char *argv[])
{
	int opt;
	int _repeat = 1;

	while ((opt = getopt(argc, argv, "C:Tdh")) != -1) {
		switch (opt) {
			case 'C':
				_repeat		= atoi(optarg);
				break;
			case 'T':
				print_usage();
				break;
			case 'd':
				g_debug		= 1;
				break;
			case 'h':
			default: /* '?' */
				print_usage();
				exit(EINVAL);
		}
	}


	if (_repeat < 0) {
		goto _exit;
	}

	g_repeat = _repeat;

	return 0;

_exit:
	print_usage();
	exit(EINVAL);
}


int test(void)
{
	struct mtd_local mtd;

	if (get_nand_info(&mtd) < 0) {
		printf("Can't get nand mtd info.\n");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct stat sb;

	int err;
	int r;			// repeat


	parse_opt(argc, argv);

	if (stat(SYS_DEV, &sb) == -1) {
		err = errno;
		perror("stat");
		exit(err);
	}

	//print_info(&sb);


	for (r = 1; r <= g_repeat; r++) {
		printf("[TRY #%d] ....\n", r);
		if (test() < 0) {
			exit(EBADMSG);
		}
	}
	

	exit(EXIT_SUCCESS);
}
