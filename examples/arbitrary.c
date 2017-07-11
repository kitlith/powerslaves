#include <powerslaves.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static const uint8_t dummy_cmd[] = {0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int main(int argc, char *argv[]) {
    if (powerslaves_select(NULL)) {
        puts("Failed to initialize powerslaves!");
        exit(EXIT_FAILURE);
    }

    size_t length = 0;

    if (argc <= 1 || strlen(argv[1]) < 16) return 1;
    printf("%d\n", argc);
    if (argc > 2) length = strtol(argv[2], NULL, 0);

    uint8_t cmd[8] = {0,0,0,0,0,0,0,0};
    for (size_t count = 0; count < sizeof(cmd)/sizeof(cmd[0]); ++count) {
        if (sscanf(argv[1] + count*2, "%2hhx", &cmd[count]) == EOF)
            return 1;
    }

    for (size_t iii = 0; iii < sizeof(cmd)/sizeof(cmd[0]); ++iii) {
        printf("%02x", cmd[iii]);
    }
    puts("");

    powerslaves_mode(ROM_MODE);

    uint8_t *returnbuf = (uint8_t *)malloc(length);
    memset(returnbuf, 0, length);
    powerslaves_sendreceive(NTR, cmd, length, returnbuf);

    for (size_t iii = 0; iii < length; ++iii) {
        printf("%02x", returnbuf[iii]);
    }
    puts("");

    return 0;
}
