#pragma once

#include <stdint.h>

void printCommand(const char *prefix, const uint8_t *cmdbuf, unsigned len);

#define printNTRCommand(buf) printCommand("NTR", buf, 0x08)
#define printCTRCommand(buf) printCommand("CTR", buf, 0x10)
