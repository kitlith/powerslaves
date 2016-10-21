TARGET := powertools

CFLAGS := -O2 -pipe -march=native

LDFLAGS := -lhidapi-libusb

build/%.o: src/%.c
	mkdir -p build/
	$(CC) -o $@ -c $< $(CFLAGS)

all: $(TARGET)

clean:
	rm $(TARGET)
	rm -r build

$(TARGET): build/main.o
	$(CC) -o $@ $< $(LDFLAGS)
