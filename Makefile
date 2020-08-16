CWARNINGS := -Wall -Wextra -fno-strict-aliasing -Wno-error=unused-parameter
CFLAGS := -pipe -O2 -fPIC -I. $(CWARNINGS)
CXXFLAGS := $(CFLAGS) -IChaiScript-6.0.0/include
LDFLAGS := -lhidapi-libusb

HEADER_FILES := powerslaves.h

all: libpowerslaves.a

powerslaves.o: powerslaves.c
	$(CC) -o $@ -c $^ $(CFLAGS) -std=c11

libpowerslaves.a: powerslaves.o
	ar rcs libpowerslaves.a $^

examples/header: examples/header.o libpowerslaves.a
	$(CC) -o $@ $^ $(LDFLAGS)

examples/savetool: examples/savetool.o libpowerslaves.a
	$(CC) -o $@ $^ $(LDFLAGS)

examples/arbitrary: examples/arbitrary.o libpowerslaves.a
	$(CC) -o $@ $^ $(LDFLAGS)

example: examples/header examples/arbitrary examples/savetool

clean:
	rm -f *.o *.a powerslaves examples/header examples/arbitrary examples/*.o
