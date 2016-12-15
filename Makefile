TARGET := powerslaves

CFLAGS := -O2 -pipe -march=native -Wall -Wextra -Werror -Wno-strict-aliasing

LDFLAGS := -lhidapi-libusb

build/%.o: src/%.c
	mkdir -p build/
	$(CC) -o $@ -c $< $(CFLAGS)

all: $(TARGET)

clean:
	rm $(TARGET)
	rm -r build

$(TARGET): build/main.o build/communication.o build/debug.o
	$(CC) -o $@ $^ $(LDFLAGS)
