TARGET := powerslaves

CFLAGS := -O2 -pipe -march=native -Wall -Wextra -Werror -Wno-strict-aliasing
LDFLAGS := -lhidapi-libusb

OBJECT_FILES := build/main.o build/communication.o build/debug.o

ifeq ($(OS),Windows_NT)
	CFLAGS += -DWIN32
	OBJECT_FILES += build/wincompat/getopt.c
endif


build/%.o: src/%.c
	mkdir -p build/
	$(CC) -o $@ -c $< $(CFLAGS)

all: $(TARGET)

clean:
	rm $(TARGET)
	rm -r build

$(TARGET): $(OBJECT_FILES)
	$(CC) -o $@ $^ $(LDFLAGS)
