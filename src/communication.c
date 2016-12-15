#include <hidapi/hidapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "communication.h"
#include "debug.h"

static hid_device* getPowersaves() {
    static hid_device *device;
    if (!device) {
        device = hid_open(0x1C1A, 0x03D5, NULL);
        if (device == NULL) {
            struct hid_device_info *enumeration = hid_enumerate(0, 0);
            if (!enumeration) {
                puts("No HID devices found! Try running as root/admin?");
                exit(-1);
            }
            free(enumeration);
            puts("No PowerSaves device found!");
            exit(-1);
        }
    }

    return device;
}

#define OUTBUF_SIZE 65

static uint8_t* getOutputBuffer() {
    static uint8_t buf[OUTBUF_SIZE];
    return buf;
}

void readData(uint8_t *buf, unsigned len) {
    if (!buf) return;
    unsigned iii = 0;
    while (iii < len) {
        iii += hid_read(getPowersaves(), buf + iii, len - iii);
        // printf("Bytes read: 0x%x\n", iii);
    }
}

void sendMessage(enum command_type type, const uint8_t *cmdbuf, uint8_t len, uint16_t response_len) {
    uint8_t *outbuf = getOutputBuffer();
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
        memset(outbuf + 2, 0, (sizeof(*outbuf) * OUTBUF_SIZE) - 2);
    }

    hid_write(getPowersaves(), outbuf, sizeof(outbuf));
}

void simpleNTRcmd(uint8_t command, uint8_t *buf, unsigned len) {
    uint8_t cmd[8] = {command, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // memset(buf, 0, len);

    printNTRCommand(cmd);
    sendNTRMessage(buf, len);

    readData(buf, len);
}
