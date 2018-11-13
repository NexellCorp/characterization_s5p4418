#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>          /* error */

#include <usb.h>
#include "usb_test.h"

#define FSG_VENDOR_ID   0x0525  /* NetChip */
#define FSG_PRODUCT_ID  0xa4a5  /* Linux-USB File-backed Storage Gadget */
#define HUB_VENDOR_ID   0x0424  /* Microsystems */
#define HUB_PRODUCT_ID  0x2640  /* Hub Device */

#define TEST_COUNT				(1)

int   debug   = 0;
int test_mode = 0;

#define DBG(x) if (debug) { printf x; }

void print_usage(void)
{
    printf( "usage: options\n"
			" -d test_mode, 0 = echi host to otg device and hsic mode(default)\n"
			"               1 = echi host to otg device\n"
			"               2 = hsic host to otg device\n"
			"               3 = otg host to usb hub device\n"
			"				4 = hsic mode\n"
			" -C test_count (default %d)\n"
			" -T not support\n"
			" -v vervose    (default disable)\n"
			, TEST_COUNT
            );
}

int verify_device(struct usb_device *dev)
{
	DBG(("dev %p: configurations %d\n",
	     dev, dev->descriptor.bNumConfigurations));
	
	if (dev->descriptor.bNumConfigurations != 1)
		return -EINVAL;

	DBG(("\t=> bLength %d\n", dev->descriptor.bLength));
	DBG(("\t=> bType   %d\n", dev->descriptor.bDescriptorType));
	DBG(("\t=> bDeviceClass  %x\n", dev->descriptor.bDeviceClass));
	DBG(("\t=> bcdUSB  %x\n", dev->descriptor.bcdUSB));
	DBG(("\t=> idVendor %x\n", dev->descriptor.idVendor));
	DBG(("\t=> idProduct %x\n", dev->descriptor.idProduct));

	switch(test_mode)
	{
		case 0:
			if ((dev->descriptor.idVendor == FSG_VENDOR_ID) // otg device mass storage
				&& (dev->descriptor.idProduct == FSG_PRODUCT_ID)) {
				DBG((" EHCI HOST TO OTG DEVICE OK!\n"));
		 		return 1;
			}
			else if ((dev->descriptor.idVendor == HUB_VENDOR_ID) // hub device at hsic port
				&& (dev->descriptor.idProduct == HUB_PRODUCT_ID)) {
				DBG((" HSIC HOST TO OTG DEVICE OK!\n"));
				return 2;
			}
			else
				return -1;
			break;
		case 1:
			if ((dev->descriptor.idVendor == FSG_VENDOR_ID) // otg device mass storage
				&& (dev->descriptor.idProduct == FSG_PRODUCT_ID)) {
				DBG((" EHCI HOST TO OTG DEVICE OK!\n"));
		 		return 1;
			}
			else
				return -1;
			break;
		case 2:
			if ((dev->descriptor.idVendor == FSG_VENDOR_ID) // otg device mass storage
				&& (dev->descriptor.idProduct == FSG_PRODUCT_ID)) {
				DBG((" HSIC HOST TO OTG DEVICE OK!\n"));
				return 2;
			}
			else
				return -1;
			break;
		case 3:
			if ((strcmp(dev->bus->dirname, "001") == 0) // otg host found new device
				&& (strcmp(dev->filename, "001") != 0)) {
				DBG((" OTG HOST TO HUB DEVICE OK!\n"));
				return 3;
			}
			else
				return -1;
			break;
		case 4:
			if ((dev->descriptor.idVendor == HUB_VENDOR_ID) // hub device at hsic port
				&& (dev->descriptor.idProduct == HUB_PRODUCT_ID)) {
				DBG((" HSIC MODE OK!\n"));
				return 2;
			}
			else
				return -1;
			break;
	default:
			break;
	}
}

int flg_show = 0;

int main(int argc, char **argv)
{
	struct usb_bus *bus, *busp;
	struct usb_device *result = NULL;
	struct usb_device *found  = NULL;
	struct usb_device *found_ehci  = NULL;
	struct usb_device *found_hsic  = NULL;
	struct usb_device *found_otg  = NULL;
	int opt, dev_chk = 0, ret = 0;
	int   test_count = TEST_COUNT, index = 0;

    while(-1 != (opt = getopt(argc, argv, "hd:C:Tv"))) {
	switch(opt) {
        case 'h':	print_usage();  exit(0);	break;
		case 'd':	test_mode = atoi(optarg);	break;	
        case 'C': 	test_count = atoi(optarg);	break;
		case 'T':   							break;
		case 'v':	debug = 1;					break;
        default:
        	break;
		}
	}

	for (index = 0; test_count > index; index++) {
		printf("USB Device Check\n");
		printf("\n");

		flg_show = 0;

		usb_init();
		usb_find_busses();
		usb_find_devices();

		bus = usb_get_busses();

		DBG(("usb_get_busses: %p\n", bus));

		for (busp = bus; busp != NULL; busp = busp->next) {	  
			struct usb_device *dev;

			DBG(("bus %p: dirname %s\n", busp, busp->dirname));

			for (dev = busp->devices; dev != NULL; dev = dev->next) {
				DBG(("dev %p filename %s\n", dev, dev->filename));

				dev_chk = verify_device(dev);
				DBG(("dev_chk value =========== %d\n", dev_chk));
				if (dev_chk < 0)
					continue;

				if (flg_show) {
					printf("bus %s: device %s\n", 
						   busp->dirname, dev->filename);
					continue;
				}

				if (dev_chk == 1) {
					found_ehci = dev;
				}
				else if (dev_chk == 2) {
					found_hsic = dev;
				}
				else if (dev_chk == 3) {
					found_otg = dev;
				}
			}
		}

		if ((found_ehci != NULL) || (found_hsic != NULL) || (found_otg != NULL)) {
			if (found_ehci != NULL)
				printf("=> echi found device: bus %s, dev %s\n", found_ehci->bus->dirname, found_ehci->filename);
			if (found_hsic != NULL)
				printf("=> hsic found device: bus %s, dev %s\n", found_hsic->bus->dirname, found_hsic->filename);
			if (found_otg != NULL)
				printf("=> otg found device: bus %s, dev %s\n", found_otg->bus->dirname, found_otg->filename);
		}
		else {
			fprintf(stderr, "failed to find device\n");
		   	ret = -ENODEV;
			goto err;
		}

		if (test_mode == 0) {
			if (found_ehci == NULL) {
				printf("echi error!\n");
				ret = -ENODEV;
				goto err;
			}
			else if (found_hsic == NULL) {
				printf("hsic error!\n");
				ret = -ENODEV;
				goto err;
			}
		}
	} // end for loop (test count)

err:
	return ret;
}
