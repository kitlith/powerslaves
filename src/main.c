#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <hidapi/hidapi.h>

static uint8_t outbuf[65];
hid_device *powersave;

enum command_type {
    TEST = 0x02,
    SWITCH_MODE = 0x10,
    NTR_MODE = 0x11,
    NTR = 0x13,
    CTR = 0x14,
    SPI = 0x15
};

static void printCommand(const char *prefix, uint8_t *cmdbuf, unsigned len) {
    if (!cmdbuf) return;
    printf("%s ", prefix);
    for (unsigned iii = 0; iii < len; ++iii) {
        printf("%02x", cmdbuf[iii]);
    }
    puts("");
}

static void readData(uint8_t *buf, unsigned len) {
    if (!buf) return;
    unsigned iii = 0;
    while (iii < len) {
        iii += hid_read(powersave, buf + iii, len - iii);
        // printf("Bytes read: 0x%x\n", iii);
    }
}

#define printNTRCommand(buf) printCommand("NTR", buf, 0x08)
#define printCTRCommand(buf) printCommand("CTR", buf, 0x10)

static void sendMessage(enum command_type type, uint8_t *cmdbuf, uint8_t len, uint16_t response_len) {
    outbuf[1] = type;
    outbuf[2] = len;
    outbuf[3] = 0x00;
    outbuf[4] = (uint8_t)(response_len & 0xFF);
    outbuf[5] = (uint8_t)((response_len >> 8) & 0xFF);

    if (cmdbuf) {
        memcpy(outbuf + 6, cmdbuf, len);
    } else if (len || response_len) {
        memset(outbuf + 6, 0, len);
    } else {
        memset(outbuf + 2, 0, sizeof(outbuf) - 2);
    }

    hid_write(powersave, outbuf, sizeof(outbuf));
}

#define sendGenericMessage(type) sendMessage(type, NULL, 0x00, 0x00)
#define sendNTRMessage(cmdbuf, response_length) sendMessage(NTR, cmdbuf, 0x08, response_length)
#define sendCTRMessage(cmdbuf, response_length) sendMessage(CTR, cmdbuf, 0x10, response_length)
#define sendSPIMessage(cmdbuf, len, response_length) sendMessage(SPI, cmdbuf, len, response_length)

static void simpleNTRcmd(uint8_t command, uint8_t *buf, unsigned len) {
    uint8_t cmd[8] = {command, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // memset(buf, 0, len);

    printNTRCommand(cmd);
    sendNTRMessage(buf, len);

    readData(buf, len);
}

#define readHeader(buf, len) simpleNTRcmd(0x00, buf, len)
#define readChipID(buf) simpleNTRcmd(0x90, buf, 0x4)
#define dummyData(buf, len) simpleNTRcmd(0x9F, buf, len)

int main(int argc, char *argv[]) {
    if (hid_init()) {
        puts("hid_init() failed!");
        return -1;
    }

    powersave = hid_open(0x1C1A, 0x03D5, NULL);
    if (!powersave) {
        struct hid_device_info *enumeration = hid_enumerate(0, 0);
        if (!enumeration) {
            puts("No HID devices found! Try escelating privileges?");
            return -1;
        }
        free(enumeration);
        puts("No PowerSaves device found!");
        return -1;
    }

    uint8_t *header = (uint8_t*)malloc(0x4000);
    if (!header) {
        puts("Memory allocation failure!");
    }

    sendGenericMessage(SWITCH_MODE);
    sendGenericMessage(NTR_MODE);
    sendGenericMessage(TEST);
    readData(header, 0x40);

    memset(outbuf, 0, sizeof(outbuf));

    dummyData(header, 0x2000);

    uint8_t chipid[0x4];
    readChipID(chipid);
    printf("ChipID: %02x%02x%02x%02x\n",
        chipid[0], chipid[1], chipid[2], chipid[3]);

    readHeader(header, 0x4000);

    FILE *headerfile = fopen("header0x4000.bin", "wb");
    fwrite(header, 0x4000, 1, headerfile);

    free(header);

    return 0;
}
