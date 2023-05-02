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
#define SET_END 0xD1
#define SET_CHAN 0xD2 // 0x0d (idx 0) + data)0x00 (idx 1)
#define DIR_OUT 0x40
#define DIR_IN 0xC0

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

    for (size_t idx = 0; idx < count; ++idx)
    {
        device = list[idx];
        rc = libusb_get_device_descriptor(device, &desc);
        assert(rc == 0);

        if (desc.idVendor == 0x1d50 && desc.idProduct == 0x6089)
        {
            printf("found device %d\n", (int)idx);
            // libusb_device_handle *dev;
            rc = libusb_open(device, &usbdev.dev);
            assert(usbdev.dev != NULL);
            // break;
        }
    }
    libusb_free_device_list(list, count);
    return 0;
}

int hackrf_board_id_read(libusb_device_handle* dev, unsigned char* value)
{
	int result;
	result = libusb_control_transfer(
		dev,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		14,
		0,
		0,
		value,
		1,
		0);

	return result;
}

int hackrf_version_string_read(libusb_device_handle* dev, char* version, unsigned char length)
{
	int result;
	result = libusb_control_transfer(
		dev,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		15,
		0,
		0,
		(unsigned char*) version,
		length,
		0);

	return result;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    unsigned char boardId = 0;
    char version[255 + 1] = { 0 };
    unsigned char length = 255;

    // init
    rc = libusb_init(&maincontext);
    assert(rc == 0);
    // find devices
    find_devices();
    // read
    hackrf_board_id_read(usbdev.dev, &boardId);
    printf("board id is %d\n", boardId);
    hackrf_version_string_read(usbdev.dev, &version[0], length);
    printf("version is %s\n", version);

    // write

    // exit
    libusb_exit(maincontext);
    return 0;
}
