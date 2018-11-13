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

#define	IN_FILE_NAME			"/mnt/mmc0/audio/0xaa55_48000_16bit.wav"
#define	OUT_FILE_NAME			"/tmp/spdif_test.wav"

#define TEST_COUNT				(1)
#define PLAY_TIME				(1)

int   debug   = 0;

#define DBG(x) if (debug) { printf x; }

void print_usage(void)
{
    printf( "usage: no options\n"
            " -p playback test (default: disable)\n"
            " -l loopback test (default: enable)\n"
			" -C test count    (default: %d)\n"
			" -T playtime      (default: %d secs)\n"
			" -v vervose       (default: disable)\n"
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
	int playback = 0;
	int test_count = TEST_COUNT, index = 0;
	int play_time = PLAY_TIME;
	struct tm *newtime;
	char am_pm[] = "AM";
	time_t long_time;

    while (-1 != (opt = getopt(argc, argv, "hplC:T:v"))) {
	switch (opt) {
        case 'h':	print_usage();  exit(0);	break;
        case 'p':	playback = 1;				break;
        case 'l':	playback = 0;				break;
        case 'C':   test_count = atoi(optarg);	break;
        case 'T':   play_time = atoi(optarg);	break;
		case 'v':	debug = 1;					break;
        default:
        	break;
		}
	}

	for (index = 0; test_count > index; index++) {
		if (playback) {
			DBG(("---------SPDIF_TX Playback TEST---------\n"));

			sprintf(cmdString, "speaker-test -tw -l %d -D default:CARD=SPDIFTranscieve > /dev/null", play_time);
			ret = system(cmdString);
			if (ret != 0) {
				printf("spdif_tx device is not found.\n");
				return -ENODEV;
			}
		}
		else {
			DBG(("-----SPDIF TX <-> RX loopback TEST------\n"));
		
			sprintf( cmdString,
			"aplay -D default:CARD=SPDIFTranscieve %s -d %d &"
			"arecord -D default:CARD=SPDIFReceiver -f 'Signed 16 bit Little Endian' -r 48000 -c 2 -d %d %s"
			, IN_FILE_NAME, play_time + 2, play_time, OUT_FILE_NAME );
			system( cmdString );
			if (ret != 0) {
				printf("spdif_tx device is not found.\n");
				return -ENODEV;
			}
	
			fd = fopen(OUT_FILE_NAME,"rb");
			if (fd == NULL)	{
				printf("file open error!\n");
				return -EIO;
			}

			fseek(fd, 54, SEEK_SET);
	
			for (i=0; i<2; i++)	{
				verify[i] = fgetc(fd);
				if (compare[i] != verify[i]) {
					printf("file diff!\n");
					printf("ori %x rec %x\n", compare[i], verify[i]);
					ret = -1;
                    fclose(fd);
					time( &long_time );                /* Get time as long integer. */
			        newtime = localtime( &long_time ); /* Convert to local time. */

			        if( newtime->tm_hour > 12 )        /* Set up extension. */
		                strcpy( am_pm, "PM" );
			        if( newtime->tm_hour > 12 )        /* Convert from 24-hour */
		                newtime->tm_hour -= 12;    /*   to 12-hour clock.  */
			        if( newtime->tm_hour == 0 )        /*Set hour to 12 if midnight. */
		                newtime->tm_hour = 12;
	    			sprintf( cmdString, "cp /tmp/spdif_test.wav /mnt/mmc0/audio/%dm%dd_%dh%dm%ds_%s.wav", newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec, am_pm );
					system( cmdString );
					goto exit;
				}
			}	
			fclose(fd);
		}
		if (ret!=0)
			printf("fail!\n");
		else
			printf("pass!\n");
		sleep(2);
	} // end loop test_count

exit:
	return ret;
}


