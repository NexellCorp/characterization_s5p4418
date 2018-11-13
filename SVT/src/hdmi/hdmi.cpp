#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/media.h>
#include <linux/v4l2-subdev.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>

#include <ion.h>
#include <linux/ion.h>
#include <linux/nxp_ion.h>
#include <linux/videodev2_nxp_media.h>

#include <nxp-v4l2.h>

#define MAX_BUFFER_COUNT 4
#define DEFAULT_WIDTH    1920
#define DEFAULT_HEIGHT   1080

#define DEFAULT_TEST_COUNT  300000

#define MAX_BUFFER_COUNT 4

#define CHECK_COMMAND(command) do { \
        int ret = command; \
        if (ret < 0) { \
            fprintf(stderr, "line %d error!!!\n", __LINE__); \
            return ret; \
        } \
    } while (0)

void print_usage(void)
{
    printf( "usage: no options\n"
			" -C test count       (default: %d)\n"
			" -T not supported.\n"
			, DEFAULT_TEST_COUNT
            );
}

extern "C" {
  int alloc_buffers(int ion_fd, int count, struct nxp_vid_buffer *bufs, int width, int height, int format);
}

static int test_count = DEFAULT_TEST_COUNT;

static void draw_color_bar(uint32_t *buf, int width, int height, uint32_t color1, uint32_t color2, uint32_t color3)
{
    int i, j;
    int width_1_3 = width / 3;
    uint32_t *p = buf;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (j < width_1_3) {
                *p = color1;
            } else if (j < width_1_3 * 2) {
                *p = color2;
            } else {
                *p = color3;
            }
            p++;
        }
    }
}

#define COLOR_RED 0xffff0000
#define COLOR_GREEN 0xff00ff00
#define COLOR_BLUE  0xff0000ff

int main(int argc, char *argv[])
{
    int ion_fd = ion_open();
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    int opt;

    while (-1 != (opt = getopt(argc, argv, "hC:T"))) {
        switch (opt) {
        case 'h':	print_usage();  exit(0);	break;
        case 'C':   test_count = atoi(optarg);	break;
        case 'T':   							break;
        default:
                    break;
        }
    }

    if (ion_fd < 0) {
        fprintf(stderr, "can't open ion!!!\n");
        return -EINVAL;
    }

    struct V4l2UsageScheme s;
    memset(&s, 0, sizeof(s));

    s.useMlc1Rgb = true;
    s.useHdmi = true;

    CHECK_COMMAND(v4l2_init(&s));
    CHECK_COMMAND(v4l2_link(nxp_v4l2_mlc1, nxp_v4l2_hdmi));

    CHECK_COMMAND(v4l2_set_preset(nxp_v4l2_hdmi, V4L2_DV_1080P60));

    CHECK_COMMAND(v4l2_set_format(nxp_v4l2_mlc1_rgb, width, height, V4L2_PIX_FMT_RGB32));
    CHECK_COMMAND(v4l2_set_crop(nxp_v4l2_mlc1_rgb, 0, 0, width, height));

    CHECK_COMMAND(v4l2_reqbuf(nxp_v4l2_mlc1_rgb, MAX_BUFFER_COUNT));

    struct nxp_vid_buffer rgb_bufs[MAX_BUFFER_COUNT];
    CHECK_COMMAND(alloc_buffers(ion_fd, MAX_BUFFER_COUNT, rgb_bufs, width, height, V4L2_PIX_FMT_RGB32));

    draw_color_bar((uint32_t *)rgb_bufs[0].virt[0], width, height, COLOR_RED, COLOR_GREEN, COLOR_BLUE);
    draw_color_bar((uint32_t *)rgb_bufs[1].virt[0], width, height, COLOR_RED, COLOR_GREEN, COLOR_BLUE);
    draw_color_bar((uint32_t *)rgb_bufs[2].virt[0], width, height, COLOR_RED, COLOR_GREEN, COLOR_BLUE);
    draw_color_bar((uint32_t *)rgb_bufs[3].virt[0], width, height, COLOR_RED, COLOR_GREEN, COLOR_BLUE);

    int out_index = 0;
    int out_dq_index = 0;
    int out_q_count = 0;
    bool started_out = false;

    struct nxp_vid_buffer *rgb_buf;
    while (test_count >= 0) {
        rgb_buf = &rgb_bufs[out_index];
        CHECK_COMMAND(v4l2_qbuf(nxp_v4l2_mlc1_rgb, 1, out_index, rgb_buf, -1, NULL));

        out_q_count++;
        out_index++;
        out_index %= MAX_BUFFER_COUNT;

        if (!started_out) {
            CHECK_COMMAND(v4l2_streamon(nxp_v4l2_mlc1_rgb));
            started_out = true;
        }

        //if (out_q_count >= MAX_BUFFER_COUNT) {
        if (out_q_count >= 2) {
            CHECK_COMMAND(v4l2_dqbuf(nxp_v4l2_mlc1_rgb, 1, &out_dq_index, NULL));
            out_q_count--;
        }

        test_count--;
    }

    CHECK_COMMAND(v4l2_streamoff(nxp_v4l2_mlc1_rgb));

    v4l2_exit();
    close(ion_fd);

    return 0;
}
