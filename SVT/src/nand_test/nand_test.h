/*
 * ref: include/linux/mtd/mtd.h
 */
struct mtd_local {
	uint8_t type[32];
	uint32_t flags;
	uint64_t size;	 // Total size of the MTD

	uint32_t erasesize;
	uint32_t writesize;
};

#define SYS_DEV		"/sys/devices/virtual/mtd/mtd0/"

#define DEF_REPEAT			(1)
#define PROG_NAME			"nand_test"
