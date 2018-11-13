/* This is NEXELL usb downloader PC tool */
#ifndef __USB_TEST_H__
#define __USB_TEST_H__

#define NXP2120_VID		0x2375
#define NXP2120_PID		0x2120

#define 	VID	NXP2120_VID
#define 	PID	NXP2120_PID

typedef struct {
	char *nsih_txt;
	char *input_file;
	char *output_file;
	int correction_num;
	unsigned int down_addr;
	unsigned int jump_addr;
} tDNImageFormat;

#endif
