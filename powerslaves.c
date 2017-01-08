#include <hidapi/hidapi.h>
#include <stdlib.h>
#include <string.h>

#include "powerslaves.h"
#define OUTBUF_SIZE (CMDBUF_SIZE + 6)

static hid_device *powersaves = NULL;

/*! \brief Command buffer sent to the powersaves.
 * This struct exactly represents the buffer send to the powersaves, and is
 * used by copying in the appropriate bits and byteswapping when necessary.
 */
struct powerslaves_cmdbuf {
    uint8_t zero; //!< Required to be 0 when sending. Always overwritten.
    uint8_t type; //!< Type of the command as described by the command_type enum.
    uint8_t cmdlen[2]; //!< The command length. Based on the type field. Little Endian
    uint8_t resplen[2]; //!< The response length. Little Endian.
    uint8_t cmdbuf[CMDBUF_SIZE]; //!< The command send to the cartridge. This has a maximum size of CMDBUF_SIZE.
} __attribute__((packed));

static struct powerslaves_cmdbuf outbuf;

// Pass NULL to get a default.
int powerslaves_select(const wchar_t *serial) {
    static const unsigned vendorid = 0x1C1A;
    static const unsigned productid = 0x03D5;

    outbuf.zero = 0x00;

    if (powersaves) { hid_close(powersaves); powersaves = NULL; }

    powersaves = hid_open(vendorid, productid, serial);
    if (!powersaves) { return -1; }
    return 0;
}

int powerslaves_send(enum command_type type, const uint8_t *cmdbuf, uint16_t response_len) {
    if (!powersaves) {
        if (powerslaves_select(NULL)) return -1;
    }
    uint16_t cmdlen = 0;

    switch (type) {
        case SWITCH_MODE:
        case NTR_MODE:
        case TEST:
            cmdlen = 0x0;
            break;
        default: // Currently not well defined, should error in the future.
        case NTR:
            cmdlen = 0x8;
            break;
        case CTR:
            cmdlen = 0x10;
            break;
    }

    outbuf.type = type;
    outbuf.cmdlen[0] = cmdlen & 0xFF;
    outbuf.cmdlen[1] = (cmdlen >> 8) & 0xFF;
    outbuf.resplen[0] = response_len & 0xFF;
    outbuf.resplen[1] = (response_len >> 8) & 0xFF;

    if (cmdbuf) { memcpy(outbuf.cmdbuf, cmdbuf, cmdlen); }
    else { memset(outbuf.cmdbuf, 0, CMDBUF_SIZE); }

    return hid_write(powersaves, (uint8_t*)&outbuf, OUTBUF_SIZE);
}

int powerslaves_receive(uint8_t *buf, uint16_t len) {
    if (!powersaves) {
        if (powerslaves_select(NULL)) return -1;
    }
    uint16_t iii = 0;
    while (iii < len) {
        int ret = hid_read(powersaves, buf + iii, len - iii);
        if (ret > 0) {
            iii += ret;
        } else {
            break;
        }
    }

    return iii;
}

int powerslaves_sendreceive(enum command_type type, const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp) {
    int err;
    if ((err = powerslaves_send(type, cmdbuf, response_len)) < 0) return err;
    return powerslaves_receive(resp, response_len);
}

int powerslaves_reset() {
    int err;
    if ((err = powerslaves_send(SWITCH_MODE, NULL, 0x00)) < 0) return err;
    if ((err = powerslaves_send(NTR_MODE, NULL, 0x00)) < 0) return err;

    uint8_t testbuf[0x40];
    if ((err = powerslaves_sendreceive(TEST, NULL, 0x40, testbuf)) < 0) return err;

    return 0;
}

void powerslaves_exit() {
    if (powersaves) hid_close(powersaves);
    powersaves = NULL;
}
