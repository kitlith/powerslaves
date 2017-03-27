#include <hidapi/hidapi.h>
#include <stdlib.h>
#include <string.h>

#include "powerslaves.h"

#define OUTBUF_SIZE 65

static hid_device *powersaves = NULL;
static uint8_t outbuf[OUTBUF_SIZE];

#ifdef POWERSLAVES_DEBUG
#include <stdio.h>

static void printcmd(enum powerslaves_cmdtype type, const uint8_t *buf) {
    unsigned length = powerslaves_cmdlen(type);
    const char *prefix = NULL;
    const uint8_t *pos = buf;

    switch (type) {
        case SWITCH_MODE:
            prefix = "MODE";
            break;
        case ROM_MODE:
            prefix = "MODE ROM";
            break;
        case SPI_MODE:
            prefix = "MODE SPI";
            break;
        case TEST:
            prefix = "TEST";
            break;
        case NTR:
            prefix = "NTR ";
            break;
        case CTR:
            prefix = "CTR ";
            break;
        case SPI:
            prefix = "SPI";
            break;
    }

    printf("%s", prefix);
    if (length && length != (unsigned)-2) {
        while (pos < (buf + length)) {
            printf("%02x", *pos++);
        }
    }
    puts("");
}
#else
static void printcmd(enum powerslaves_cmdtype type, const uint8_t *buf) {}
#endif

/* Pass NULL to get a default. */
int powerslaves_select(const wchar_t *serial) {
    static const unsigned vendorid = 0x1C1A;
    static const unsigned productid = 0x03D5;

    outbuf[0] = 0x00;

    if (powersaves) {
        #ifdef POWERSLAVES_DEBUG
        puts("Reopening the powersaves device! Is there a bug somewhere?");
        #endif
        hid_close(powersaves);
        powersaves = NULL;
    }

    powersaves = hid_open(vendorid, productid, serial);
    if (!powersaves) { return -1; }
    return 0;
}

uint16_t powerslaves_cmdlen(enum powerslaves_cmdtype type) {
    switch (type) {
        case SWITCH_MODE:
        case ROM_MODE:
        case SPI_MODE:
        case TEST:
            return 0x0;
        case NTR:
            return 0x8;
            break;
        case CTR:
            return 0x10;
            break;
        case SPI:
            /* oh god why, how do I manage this? I'll just say invalid param for now. */
        default:
            return -2; /* Invalid Parameter */
    }
}

int powerslaves_send(enum powerslaves_cmdtype type, const uint8_t *cmdbuf, uint16_t response_len) {
    uint16_t cmdlen = powerslaves_cmdlen(type);
    if (cmdlen == (uint16_t)-2) return -2;

    return powerslaves_sendlen(type, cmdlen, cmdbuf, response_len);
}

int powerslaves_sendlen(enum powerslaves_cmdtype type, uint16_t cmdlen, const uint8_t *cmdbuf, uint16_t response_len) {
    if (!powersaves) {
        if (powerslaves_select(NULL)) return -1;
    }

    outbuf[1] = type;
    outbuf[2] = cmdlen & 0xFF;
    outbuf[3] = (cmdlen >> 8) & 0xFF;
    outbuf[4] = response_len & 0xFF;
    outbuf[5] = (response_len >> 8) & 0xFF;

    if (cmdbuf) { memcpy(outbuf + 6, cmdbuf, cmdlen); }
    else { memset(outbuf + 6, 0, OUTBUF_SIZE - 6); }

    printcmd(type, cmdbuf);
    return hid_write(powersaves, outbuf, OUTBUF_SIZE);
}

int powerslaves_receive(uint8_t *buf, uint16_t len) {
    uint16_t iii = 0;
    if (!powersaves) {
        if (powerslaves_select(NULL)) return -1;
    }

    while (iii < len) {
        int ret = hid_read(powersaves, buf + iii, len - iii);
        if (ret > 0) {
            iii += ret;
        } else {
            #ifdef POWERSLAVES_DEBUG
            printf("hid_read returned %d!\n0x%x bytes read.", ret, iii);
            #endif
            break;
        }
    }

    return iii;
}

int powerslaves_sendreceive(enum powerslaves_cmdtype type, const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp) {
    int err;
    if ((err = powerslaves_send(type, cmdbuf, response_len)) < 0) return err;
    return powerslaves_receive(resp, response_len);
}

int powerslaves_mode(enum powerslaves_cmdtype mode) {
    int err;
    uint8_t testbuf[0x40];

    switch (mode) {
        case ROM_MODE:
        case SPI_MODE:
            break;
        default:
            return -2;
    }

    if ((err = powerslaves_send(SWITCH_MODE, NULL, 0x00)) < 0) return err;
    if ((err = powerslaves_send(mode, NULL, 0x00)) < 0) return err;
    if ((err = powerslaves_sendreceive(TEST, NULL, 0x40, testbuf)) < 0) return err;

    return 0;
}

void powerslaves_exit() {
    if (powersaves) hid_close(powersaves);
    powersaves = NULL;
}
