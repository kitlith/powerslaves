#pragma once

#include <stdint.h>

enum command_type {
    TEST = 0x02,
    SWITCH_MODE = 0x10,
    NTR_MODE = 0x11,
    NTR = 0x13,
    CTR = 0x14,
    SPI = 0x15
};

// Takes a buffer and reads len bytes into it.
void readData(uint8_t *buf, unsigned len);

// Takes a command type, command buffer with length, and the length of the expected response.
void sendMessage(enum command_type type, const uint8_t *cmdbuf, uint8_t len, uint16_t response_len);

// Convience function that takes a single byte for the command buffer and reads data into the buffer.
void simpleNTRcmd(uint8_t command, uint8_t *buf, unsigned len);

// Convienence functions that specialize the more general functions above.

#define sendGenericMessage(type) sendMessage(type, NULL, 0x00, 0x00)
#define sendNTRMessage(cmdbuf, response_length) sendMessage(NTR, cmdbuf, 0x08, response_length)
#define sendCTRMessage(cmdbuf, response_length) sendMessage(CTR, cmdbuf, 0x10, response_length)
#define sendSPIMessage(cmdbuf, len, response_length) sendMessage(SPI, cmdbuf, len, response_length)

#define readHeader(buf, len) simpleNTRcmd(0x00, buf, len)
#define readChipID(buf) simpleNTRcmd(0x90, buf, 0x4)
#define dummyData(buf, len) simpleNTRcmd(0x9F, buf, len)
