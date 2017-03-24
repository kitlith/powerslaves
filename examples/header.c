#include <powerslaves.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h> // getopt

enum behavior {
    DEVICE_NTR,
    DEVICE_TWL,
    DEVICE_CTR,
    DEVICE_AUTO = DEVICE_NTR
};

struct args {
    unsigned header_len;
    const char *header_filename;
    enum behavior dev;
};

static void parse_args(int argc, char *argv[], struct args *arg) {
    char c;
    while ((c = getopt(argc, argv, "l:o:ntc:")) != -1) {
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
        }
    }
}

static const uint8_t CTRmagic[] = {0x71, 0xC9, 0x3F, 0xE9, 0xBB, 0x0A, 0x3B, 0x18};
static const uint8_t dummy_cmd[] = {0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t header_cmd[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t chipid_cmd[] = {0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define readHeader(buf, len) powerslaves_sendreceive(NTR, header_cmd, len, buf)
#define readChipID(buf) powerslaves_sendreceive(NTR, chipid_cmd, 4, buf)

int main(int argc, char *argv[]) {
    if (powerslaves_select(NULL)) {
        puts("Failed to initialize powerslaves!");
        exit(EXIT_FAILURE);
    }

    struct args arg = {
        .header_len = 0x1000,
        .header_filename = "header.bin",
        .dev = DEVICE_AUTO
    };
    parse_args(argc, argv, &arg);

    if (arg.header_len) printf("Reading 0x%x byte header to file '%s'.\n", arg.header_len, arg.header_filename);

    uint8_t *header = (uint8_t*)malloc(arg.header_len);
    {
        uint8_t *garbage = (uint8_t*)malloc(0x2000);
        if (!header || !garbage) {
            puts("Memory allocation failure!");
            exit(EXIT_FAILURE);
        }

        powerslaves_mode(ROM_MODE);
        powerslaves_sendreceive(NTR, dummy_cmd, 0x2000, garbage);
        free(garbage);
    }

    uint8_t chipid[0x4] = {0x00, 0x00, 0x00, 0x00};

    switch (arg.dev) {
        case DEVICE_NTR:
            readHeader(header, arg.header_len);
            readChipID(chipid);
            break;
        case DEVICE_TWL:
            readChipID(chipid);
            readHeader(header, arg.header_len);
            break;
        case DEVICE_CTR:
            powerslaves_send(NTR, CTRmagic, 0);
            readChipID(chipid);
            readHeader(header, arg.header_len);
            break;
        default:
            printf("This should never happen.\n");
            exit(EXIT_FAILURE);
    }

    printf("ChipID: %02x%02x%02x%02x\n",
        chipid[0], chipid[1], chipid[2], chipid[3]);

    FILE *headerfile = fopen(arg.header_filename, "wb");
    fwrite(header, arg.header_len, 1, headerfile);

    free(header);
    powerslaves_exit();

    return 0;
}
