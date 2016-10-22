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
    printf("\n");
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
    } else if (len) {
        memset(outbuf + 6, 0, len);
    }

    hid_write(powersave, outbuf, sizeof(outbuf));
}

#define sendGenericMessage(type, response_length) sendMessage(type, NULL, 0x00, response_length)
#define sendNTRMessage(cmdbuf, response_length) sendMessage(NTR, cmdbuf, 0x08, response_length)
#define sendCTRMessage(cmdbuf, response_length) sendMessage(CTR, cmdbuf, 0x10, response_length)
#define sendSPIMessage(cmdbuf, len, response_length) sendMessage(SPI, cmdbuf, len, response_length)

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

    sendGenericMessage(SWITCH_MODE, 0x00);
    sendGenericMessage(NTR_MODE, 0x00);
    sendGenericMessage(TEST, 0x40);

    memset(outbuf, 0, sizeof(outbuf));

    uint8_t ntrcmd[0x8] = {0, 0, 0, 0, 0, 0, 0, 0};

    ntrcmd[0] = 0x9F;
    printNTRCommand(ntrcmd);
    sendNTRMessage(ntrcmd, 0x2000);

    ntrcmd[0] = 0x90;
    printNTRCommand(ntrcmd);
    sendNTRMessage(ntrcmd, 0x4);

    uint8_t cardid[0x4];
    hid_read(powersave, cardid, 0x4);
    printf("CardID: %02x%02x%02x%02x\n",
        cardid[0], cardid[1], cardid[2], cardid[3]);

    bool cheapChip = (cardid[0] >> 7) & 1;

    if (cheapChip) {
        // Not handling this additional logic yet.
        puts("I'm not dealing with this kind of card yet. Sorry!");
        return -1;
    }

    uint8_t *header = (uint8_t*)malloc(0x4000);
    if (!header) {
        puts("Memory allocation failure!");
    }

    ntrcmd[0] = 0x00;
    printNTRCommand(ntrcmd);
    sendNTRMessage(ntrcmd, 0x4000);

    hid_read(powersave, header, 0x4000);
    FILE *headerfile = fopen("header.bin", "wb");
    fwrite(header, 0x1000, 1, headerfile);

    return 0;
}
