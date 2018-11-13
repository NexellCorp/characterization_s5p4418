#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "gpio.h"

#define	IN_FILE_NAME			"/mnt/mmc0/audio/0xaa55_48000_16bit.wav"
#define	OUT_FILE_NAME			"/tmp/i2s_test.wav"

#define I2S_PORT				(0)
#define TEST_COUNT				(1)
#define PLAY_TIME				(1)

#define I2S_LOOPBACK_PIN1	70
#define I2S_LOOPBACK_PIN2	71

int   debug   = 0;

#define DBG(x) if (debug) { printf x; }

void print_usage(void)
{
    printf( "usage: no options\n"
            " -p i2s port         (default: %d)\n"
            " -l loopback channel (default: playback)\n"
			" -C test count       (default: %d)\n"
			" -T playtime         (default: %d secs)\n"
			" -v vervose          (default: disable)\n"
			, I2S_PORT
			, TEST_COUNT
			, PLAY_TIME
            );
}

int main(int argc, char **argv)
{
	unsigned char compare[2] = {0x55,0xaa};
	unsigned char verify[2];
	FILE *fd;
	int opt, i, ret = 0;
	char		cmdString[1024];
	int ch = I2S_PORT, loop_ch = -1;
	int test_count = TEST_COUNT, index = 0;
	int play_time = PLAY_TIME;

    while(-1 != (opt = getopt(argc, argv, "hp:l:C:T:v"))) {
	switch (opt) {
        case 'h':	print_usage();  exit(0);	break;
        case 'p':	ch = atoi(optarg);			break;
        case 'l':	loop_ch = atoi(optarg);		break;
        case 'C':   test_count = atoi(optarg);	break;
        case 'T':   play_time = atoi(optarg);	break;
	 	case 'v':	debug = 1;					break;
        default:
        	break;
		}
	}
	
	if (gpio_export(I2S_LOOPBACK_PIN1)!=0) {
		printf("gpio open fail\n");
		ret = -1;
		return ret;
	}
	if (gpio_export(I2S_LOOPBACK_PIN2)!=0) {
		printf("gpio open fail\n");
		ret = -1;
		return ret;
	}
	gpio_dir_out(I2S_LOOPBACK_PIN1);
	gpio_dir_out(I2S_LOOPBACK_PIN2);

	for (index = 0; test_count > index; index++) {
	 	if (loop_ch < 0) {
			DBG(("---------I2S%d Playback TEST---------\n", ch));

			if (ch == 1) {
				gpio_set_value(I2S_LOOPBACK_PIN1,1);
				gpio_set_value(I2S_LOOPBACK_PIN2,0);
			}
			else if (ch == 2) {
				gpio_set_value(I2S_LOOPBACK_PIN1,1);
				gpio_set_value(I2S_LOOPBACK_PIN2,1);
			}

			sprintf(cmdString, "speaker-test -tw -l %d -D default:CARD=SNDNULL%d > /dev/null", play_time, ch);
			ret = system(cmdString);
			if (ret < 0) {
				printf("i2s%d device is not found.\n", ch);
				return -ENODEV;
			}
		}
		else {
			DBG(("-----I2S%d to I2S%d loopback TEST-----\n", ch, loop_ch));

			if (loop_ch == 1) {
				gpio_set_value(I2S_LOOPBACK_PIN1,1);
				gpio_set_value(I2S_LOOPBACK_PIN2,0);
			}
			else if (loop_ch == 2) {
				gpio_set_value(I2S_LOOPBACK_PIN1,1);
				gpio_set_value(I2S_LOOPBACK_PIN2,1);
			}
			
			sprintf( cmdString,
			"aplay -D default:CARD=SNDNULL%d %s -d %d & sleep 1;"
			"arecord -D default:CARD=SNDNULL%d -f 'Signed 16 bit Little Endian' -r 48000 -c 2 -d %d %s"
			, ch, IN_FILE_NAME, play_time + 1, loop_ch, play_time, OUT_FILE_NAME);
			ret = system( cmdString );
			if (ret < 0) {
				printf("i2s%d device is not found.\n", ch);
				return -ENODEV;
			}
	
			fd = fopen(OUT_FILE_NAME,"rb");
			if (fd == NULL)	{
				printf("file open error!\n");
				return -EIO;
			}

			fseek(fd, 52, SEEK_SET);
	
			for (i=0; i<2; i++)	{
				verify[i] = fgetc(fd);
				if ((compare[i] >> 1) != verify[i] ) {
					printf("ori %x rec %x\n", compare[i] >> 1, verify[i]);
					printf("retry\n");
					if (compare[i] != verify[i]) {
						printf("file diff!\n");
						printf("ori %x rec %x\n", compare[i], verify[i]);
						ret = -1;
						goto exit;
					}
				}
			}	
			fclose(fd);
		}
		if (ret!=0)
			printf("fail!\n");
		else
			printf("pass!\n");
		sleep(1);
	}	// end loop test_count

exit:
    gpio_unexport(I2S_LOOPBACK_PIN1);
    gpio_unexport(I2S_LOOPBACK_PIN2);

	return ret;
}
