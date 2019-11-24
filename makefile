#makefile
#needs libusb-1.0

all: libusttest 

libusttest:
	g++ libusb.c -o libusttest -lusb-1.0 -lpcap

clean:
	rm -f libusttest 
