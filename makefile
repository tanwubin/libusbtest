#makefile
#needs libusb-1.0

all: libusbtest 

libusbtest:
	g++ libusb.c -o libusbtest -lusb-1.0

clean:
	rm -f libusbtest libusb
