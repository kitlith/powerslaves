#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <hidapi/hidapi.h>

#include "communication.h"
#include "debug.h"

enum behavior {
    DEVICE_NTR,
    DEVICE_TWL,
    DEVICE_CTR,
    AK2I_DUMP_FLASH,
    AK2I_WRITE_FLASH,
    DEVICE_AUTO = DEVICE_NTR
};

struct args {
    unsigned header_len;
    const char *header_filename;
    const char *flash_filename;
    enum behavior dev;
};

// Takes a buffer to dump garbage into.
static void cartInit(uint8_t *buf) {
    sendGenericMessage(SWITCH_MODE);
    sendGenericMessage(NTR_MODE);
    sendGenericMessage(TEST);
    readData(buf, 0x40);
}

static void parse_args(int argc, char *argv[], struct args *arg) {
    char c;
    while ((c = getopt(argc, argv, "l:o:ntcdf:")) != -1) {
        switch (c) {
            case '?':
                exit(-1);
            case 'l':
                if (!optarg) exit(EXIT_FAILURE);
                arg->header_len = strtol(optarg, NULL, 0);
                break;
            case 'o':
                if (!optarg) exit(EXIT_FAILURE);
                arg->header_filename = optarg;
                break;
            case 'n':
                arg->dev = DEVICE_NTR;
                break;
            case 't':
                arg->dev = DEVICE_TWL;
                break;
            case 'c':
                arg->dev = DEVICE_CTR;
                break;
            case 'd':
                arg->dev = AK2I_DUMP_FLASH;
                break;
            case 'f':
                if (!optarg) exit(EXIT_FAILURE);
                arg->dev = AK2I_WRITE_FLASH;
                arg->flash_filename = optarg;
                arg->header_filename = "ak2i_flash.bin";
                arg->header_len = 0x1000000;
                break;
        }
    }
}

static const uint8_t CTRmagic[] = {0x71, 0xC9, 0x3F, 0xE9, 0xBB, 0x0A, 0x3B, 0x18};

int main(int argc, char *argv[]) {
    if (hid_init()) {
        puts("hid_init() failed!");
        exit(EXIT_FAILURE);
    }

    struct args arg = {
        .header_len = 0x1000,
        .header_filename = "header.bin",
        .dev = DEVICE_AUTO
    };
    parse_args(argc, argv, &arg);

    printf("Reading 0x%x byte header to file '%s'.\n", arg.header_len, arg.header_filename);

    uint8_t *header = (uint8_t*)malloc(arg.header_len);
    {
        uint8_t *garbage = (uint8_t*)malloc(0x2000);
        if (!header || !garbage) {
            puts("Memory allocation failure!");
        }

        cartInit(garbage);
        dummyData(garbage, 0x2000);
    }

    uint8_t chipid[0x4] = {0x00, 0x00, 0x00, 0x00};

    switch (arg.dev) {
        case DEVICE_NTR:
            readHeader(header, arg.header_len);
            readChipID(chipid);
            readChipID(chipid);
            break;
        case DEVICE_TWL:
            readChipID(chipid);
            readChipID(chipid);
            readHeader(header, arg.header_len);
            break;
        case DEVICE_CTR:
            printNTRCommand(CTRmagic);
            sendNTRMessage(CTRmagic, 0x00);
            readChipID(chipid);
            readChipID(chipid);
            readHeader(header, arg.header_len);
            break;
        case AK2I_DUMP_FLASH: {
                uint8_t hw_revision[4];
                readChipID(chipid);
                simpleNTRcmd(0xD1, (uint8_t*)(&hw_revision), 4);
                printf("AK2i Revision: %02x%02x%02x%02x\n",
                    hw_revision[0], hw_revision[1], hw_revision[2], hw_revision[3]);

                // uint8_t ak2ibuf[8] = {0xB7, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00};
                // for (uint32_t addr=0; addr < arg.header_len; addr += 0x200) {
                //     ak2ibuf[1] = (addr >> 24) & 0xFF;
                //     ak2ibuf[2] = (addr >> 16) & 0xFF;
                //     ak2ibuf[3] = (addr >>  8) & 0xFF;
                //     ak2ibuf[4] = (addr >>  0) & 0xFF;
                //     printNTRCommand(ak2ibuf);
                //     sendNTRMessage(ak2ibuf, 0x200);
                //     readData(header + addr, 0x200);
                // }
            }
            break;
        case AK2I_WRITE_FLASH:
            printf("Not Implemented.\n");
            exit(EXIT_FAILURE);
        default:
            printf("This should never happen.\n");
            exit(EXIT_FAILURE);
    }

    printf("ChipID: %02x%02x%02x%02x\n",
        chipid[0], chipid[1], chipid[2], chipid[3]);

    FILE *headerfile = fopen(arg.header_filename, "wb");
    fwrite(header, arg.header_len, 1, headerfile);

    free(header);

    return 0;
}
