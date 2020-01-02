#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>

#define TIMEOUT 1000

#define DEFAULT_CHANNEL 0x0b // 11

#define DATA_EP_CC2531 0x83
#define DATA_EP_CC2530 0x82
#define DATA_EP_CC2540 0x83
#define DATA_TIMEOUT 2500

#define GET_IDENT 0xC0
#define SET_POWER 0xC5
#define GET_POWER 0xC6
#define SET_START 0xD0
#define SET_END   0xD1
#define SET_CHAN  0xD2 // 0x0d (idx 0) + data)0x00 (idx 1)
#define DIR_OUT   0x40
#define DIR_IN    0xC0

#define POWER_RETRIES 10

#define CC2531 0x01
#define CC2540 0x02

libusb_context *maincontext = NULL;

struct usb_device
{
	libusb_device_handle *dev;
	uint8_t channel;
	u_char dev_type;
	bool configured;
};

usb_device usbdev;

int find_devices()
{

    libusb_device **list = NULL;

    libusb_device *device;
    libusb_device_descriptor desc = {0};

    int rc = 0;
    int ret = 0;
    ssize_t count = 0;


    count = libusb_get_device_list(maincontext, &list);
    assert(count > 0);

    for (size_t idx = 0; idx < count; ++idx) {
        device = list[idx];
        rc = libusb_get_device_descriptor(device, &desc);
        assert(rc == 0);

	if(desc.idVendor == 0x0451 && desc.idProduct == 0x16b3)
	{
		printf("found device %d\n",(int)idx);
		//libusb_device_handle *dev;
		rc = libusb_open(device,&usbdev.dev);
		assert(usbdev.dev != NULL);
		//break;
	}
    }
    libusb_free_device_list(list, count);
}

int init(libusb_device_handle *dev, int channel)
{
    int rc = 0; 
   /*Check if kenel driver attached*/

   printf("Check if kernel driver attached\n");
   if(libusb_kernel_driver_active(dev, 0))
   {
      printf("detach driver\n");
      rc = libusb_detach_kernel_driver(dev, 0); // detach driver
      printf("rc:%d\n",rc);
      assert(rc == 0);
   }

    printf("libusb_reset_device\n");
    rc = libusb_reset_device(dev);	
    printf("libusb_reset_device rc:%d\n",rc);
    //assert(rc < 0);

    //set the configurationlibusb_set_configuration
    printf("set the configuration\n");
    rc = libusb_set_configuration(dev, 1);
    //assert(rc < 0);
    printf("libusb_set_configuration rc:%d\n",rc);

   printf("libusb_claim_interface\n");
   rc = libusb_claim_interface(dev, 0);
   printf("libusb_claim_interface rc:%d\n",rc);
   //assert(rc < 0);

    printf("libusb_set_interface_alt_setting\n");
    rc = libusb_set_interface_alt_setting(dev,0x00,0x00);	
    printf("libusb_set_interface_alt_setting rc:%d\n",rc);
    //assert(rc < 0);
    int ret = 0;
    uint8_t ident[32];
    ret = libusb_control_transfer(dev, DIR_IN, GET_IDENT, 0x00, 0x00, ident, sizeof(ident), TIMEOUT);
    if (ret > 0)
    {
        printf("IDENT:");
        for (int i = 0; i < ret; i++)
            printf(" %02X", ident[i]);
        printf("\n");
    }

    return rc;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    rc = libusb_init(&maincontext);
    assert(rc == 0);

    find_devices();

    init(usbdev.dev,11);

    libusb_exit(maincontext);
    return 0;
}
