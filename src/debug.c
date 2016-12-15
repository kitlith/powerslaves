#include <stdio.h>
#include <stdint.h>

void printCommand(const char *prefix, const uint8_t *cmdbuf, unsigned len) {
    if (!cmdbuf) return;
    printf("%s ", prefix);
    for (unsigned iii = 0; iii < len; ++iii) {
        printf("%02x", cmdbuf[iii]);
    }
    puts("");
}
