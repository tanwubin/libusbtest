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

#define RZ_KILLERBEE_USB_VENDOR         0x03EB
#define RZ_KILLERBEE_USB_PRODUCT        0x210A

#define RZ_KILLERBEE_SET_MODE           0x07
#define RZ_KILLERBEE_SET_CHANNEL        0x08
#define RZ_KILLERBEE_OPEN_STREAM        0x09
#define RZ_KILLERBEE_CLOSE_STREAM       0x0A

#define RZ_KILLERBEE_CMD_MODE_AC        0x00
#define RZ_KILLERBEE_CMD_MODE_NONE      0x04

#define RZ_KILLERBEE_CMD_EP             0x02
#define RZ_KILLERBEE_REP_EP             0x84
#define RZ_KILLERBEE_PKT_EP             0x81

#define RZ_KILLERBEE_TIMEOUT            1000

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

        printf("Vendor:Device = %04x:%04x\n", desc.idVendor, desc.idProduct);

	if(desc.idVendor == RZ_KILLERBEE_USB_VENDOR && desc.idProduct == RZ_KILLERBEE_USB_PRODUCT)
	{
		printf("found device %d\n",(int)idx);
		//libusb_device_handle *dev;
		rc = libusb_open(device,&usbdev.dev);
        printf("libusb_open:%d\n",rc);
		assert(usbdev.dev != NULL);
		//set type 
		usbdev.dev_type = 1;
		usbdev.channel = 11;
		//break;
	}
    }
    libusb_free_device_list(list, count);
}

int init(libusb_device_handle *dev, int channel)
{
    int rc = 0; 
   /*Check if kenel driver attached*/
/**
   printf("Check if kernel driver attached\n");
   if(libusb_kernel_driver_active(dev, 0))
   {
      printf("detach driver\n");
      rc = libusb_detach_kernel_driver(dev, 0); // detach driver
      printf("rc:%d\n",rc);
      assert(rc == 0);
   }
   printf("rc:%d\n",rc);
/**/
sleep(1);

    printf("libusb_reset_device\n");
    rc = libusb_reset_device(dev);	
    printf("libusb_reset_device rc:%d\n",rc);
    //assert(rc < 0);

sleep(1);

    //set the configurationlibusb_set_configuration
    printf("set the configuration\n");
    rc = libusb_set_configuration(dev, 1);
    //assert(rc < 0);
    printf("libusb_set_configuration rc:%d\n",rc);

sleep(1);

   printf("libusb_claim_interface\n");
   rc = libusb_claim_interface(dev, 0);
   printf("libusb_claim_interface rc:%d\n",rc);
   //assert(rc < 0);

sleep(1);
/**/
    printf("libusb_set_interface_alt_setting\n");
    rc = libusb_set_interface_alt_setting(dev,0x00,0x00);	
    printf("libusb_set_interface_alt_setting rc:%d\n",rc);
    //assert(rc < 0);

sleep(1);
/**/
    printf("set mode\n");
    int xfer = 0;
    unsigned char data[2];
    data[0]=RZ_KILLERBEE_SET_MODE;
    data[1]=RZ_KILLERBEE_CMD_MODE_AC;

    rc = libusb_bulk_transfer(dev, RZ_KILLERBEE_CMD_EP, data, sizeof(data), &xfer, RZ_KILLERBEE_TIMEOUT);
    printf("set mode:%d xfer:%d\n",rc,xfer);
sleep(1);

    printf("set channel\n");
    data[0]=RZ_KILLERBEE_SET_CHANNEL;
    data[1]=11;

    rc = libusb_bulk_transfer(dev, RZ_KILLERBEE_CMD_EP, data, sizeof(data), &xfer, RZ_KILLERBEE_TIMEOUT);
    printf("set channel:%d xfer:%d\n",rc,xfer);
sleep(1);

    //open stream
    printf("open stream\n");
    data[0]=RZ_KILLERBEE_OPEN_STREAM;
    rc = libusb_bulk_transfer(dev, RZ_KILLERBEE_CMD_EP, data, sizeof(data)-1, &xfer, RZ_KILLERBEE_TIMEOUT);
    printf("open stream:%d xfer:%d\n",rc,xfer);
sleep(1);

    //read data
    unsigned char pktdata[1024];memset(pktdata,0x00,1024);

    while(1)
    {
        rc = libusb_bulk_transfer(dev, RZ_KILLERBEE_PKT_EP, pktdata, sizeof(pktdata), &xfer, RZ_KILLERBEE_TIMEOUT);
        printf("channel:%d ret:%d xfer:%d\n",channel,rc,xfer);
        if(xfer > 0)
        {
            for (int i = 0; i < xfer; i++)
                printf(" %02X", pktdata[i]);
            printf("\n");
            memset(pktdata,0x00,1024);
        }
    //    sleep(1);
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
