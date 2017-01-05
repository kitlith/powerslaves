CFLAGS := -pipe -O2 -fPIC -Wall -Wextra -Werror -fno-strict-aliasing -I.
LDFLAGS := -lhidapi-libusb

OFILES := powerslaves.o
HEADER_FILES := powerslaves.h

libpowerslaves.a: $(OFILES)
	ar rcs libpowerslaves.a $(OFILES)

all: libpowerslaves.a

examples/header: libpowerslaves.a examples/header.o
	$(CC) -o examples/header examples/header.o libpowerslaves.a $(LDFLAGS)

examples/ak2itool: libpowerslaves.a examples/ak2itool.o
	$(CC) -o examples/ak2itool examples/ak2itool.o libpowerslaves.a $(LDFLAGS)

example: examples/header examples/ak2itool

clean:
	rm -f *.o *.a examples/header examples/ak2itool examples/*.o
