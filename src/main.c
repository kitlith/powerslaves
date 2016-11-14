#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <hidapi/hidapi.h>

#include "communication.h"
#include "debug.h"

// Takes a buffer to dump garbage into.
static void cartInit(uint8_t *buf) {
    sendGenericMessage(SWITCH_MODE);
    sendGenericMessage(NTR_MODE);
    sendGenericMessage(TEST);
    readData(buf, 0x40);
}

enum device {
    DEVICE_NTR,
    DEVICE_TWL,
    DEVICE_3DS
};

int main(int argc, char *argv[]) {
    if (hid_init()) {
        puts("hid_init() failed!");
        return -1;
    }

    unsigned header_len = 0x1000;
    const char *header_filename = "header.bin";
    enum device dev = DEVICE_NTR;

    char c;
    while ((c = getopt(argc, argv, "l:o:3i")) != -1) {
        switch (c) {
            case '?':
                exit(-1);
            case 'l':
                if (!optarg) exit(EXIT_FAILURE);
                header_len = strtol(optarg, NULL, 0);
                break;
            case 'o':
                if (!optarg) exit(EXIT_FAILURE);
                header_filename = optarg;
                break;
            case '3':
                dev = DEVICE_3DS;
                break;
            case 'i':
                dev = DEVICE_TWL;
                break;
        }
    }

    printf("Reading 0x%x byte header to file '%s'.\n", header_len, header_filename);

    uint8_t *header = (uint8_t*)malloc((header_len > 0x2000) ? header_len : 0x2000);
    if (!header) {
        puts("Memory allocation failure!");
    }

    uint8_t chipid[0x4];

    cartInit(header);
    dummyData(header, 0x2000);

    switch (dev) {
        default:
        case DEVICE_NTR:
            readHeader(header, header_len);
            readChipID(chipid);
            break;
        case DEVICE_TWL:
            readChipID(chipid);
            readHeader(header, header_len);
            break;
        case DEVICE_3DS: {
                uint8_t command_3ds[] = {0x71, 0xC9, 0x3F, 0xE9, 0xBB, 0x0A, 0x3B, 0x18};
                printNTRCommand(command_3ds);
                sendNTRMessage(command_3ds, 0x00);
                readChipID(chipid);
                readHeader(header, header_len);
            }
            break;
    }

    printf("ChipID: %02x%02x%02x%02x\n",
        chipid[0], chipid[1], chipid[2], chipid[3]);

    FILE *headerfile = fopen(header_filename, "wb");
    fwrite(header, header_len, 1, headerfile);

    free(header);

    return 0;
}
