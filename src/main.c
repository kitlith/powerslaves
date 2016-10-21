#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <hidapi/hidapi.h>

static uint8_t outbuf[65];
hid_device *powersave;

enum command_type {
    TEST = 0x02,
    SWITCH_MODE = 0x10,
    NTR_MODE = 0x11,
    NTR = 0x13,
    CTR = 0x14
};

static void printNTRCommand(uint8_t *cmdbuf) {
    // This is quick and ugly and everything. Hope that nothing invalid gets passed in here.
    printf("NTRCMD %02x %02x %02x %02x %02x %02x %02x %02x\n",
        cmdbuf[0], cmdbuf[1], cmdbuf[2], cmdbuf[3], cmdbuf[4], cmdbuf[5], cmdbuf[6], cmdbuf[7]);
}

static void printCTRCommand(uint8_t *cmdbuf) {
    // This is quick and ugly and everything. Hope that nothing invalid gets passed in here.
    printf("NTRCMD %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        cmdbuf[0], cmdbuf[1], cmdbuf[2], cmdbuf[3], cmdbuf[4], cmdbuf[5], cmdbuf[6], cmdbuf[7],
        cmdbuf[8], cmdbuf[9], cmdbuf[10], cmdbuf[11], cmdbuf[12], cmdbuf[13], cmdbuf[14], cmdbuf[15]);
}

static void sendMessage(enum command_type type, uint8_t *cmdbuf, uint8_t len, uint16_t response_len) {
    outbuf[1] = type;
    outbuf[2] = len;
    outbuf[3] = 0x00;
    outbuf[4] = (uint8_t)(response_len & 0xFF);
    outbuf[5] = (uint8_t)((response_len >> 8) & 0xFF);

    if (cmdbuf) {
        memcpy(outbuf + 6, cmdbuf, len);
    } else {
        memset(outbuf + 6, 0, len);
    }

    hid_write(powersave, outbuf, sizeof(outbuf));
}

#define sendGenericMessage(type, response_length) sendMessage(type, NULL, 0, response_length)

int main(int argc, char *argv[]) {
    if (hid_init()) {
        printf("hid_init() failed!\n");
        return -1;
    }

    powersave = hid_open(0x1C1A, 0x03D5, NULL);
    if (!powersave) {
        printf("No PowerSaves device found!\n");
        return -1;
    }

    {
        wchar_t wstr[255];

        hid_get_manufacturer_string(powersave, wstr, 255);
        printf("Manufacturer: %ls\n", wstr);

        hid_get_product_string(powersave, wstr, 255);
        printf("Product: %ls\n", wstr);
    }

    uint8_t ntrcmd[0x8] = {0, 0, 0, 0, 0, 0, 0, 0};

    sendGenericMessage(SWITCH_MODE, 0x00);
    sendGenericMessage(NTR_MODE, 0x00);
    sendGenericMessage(TEST, 0x40);

    unsigned res = hid_read(powersave, outbuf, 0x40);
    for (unsigned iii = 0; iii < res; ++iii) {
        printf("buf[0x%02x]: %02x\n", iii, outbuf[iii]);
    }

    ntrcmd[0] = 0x9F;
    printNTRCommand(ntrcmd);
    sendMessage(NTR, ntrcmd, 0x8, 0x2000);

    return 0;
}
