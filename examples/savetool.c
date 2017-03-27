#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h> // getopt

#include "../powerslaves.h"

enum behavior {
    DUMP_SAVE,
    WRITE_SAVE,
    DEVICE_AUTO = DUMP_SAVE
};

struct args {
    unsigned save_len;
    const char *save_filename;
    enum behavior dev;
};

static void parse_args(int argc, char *argv[], struct args *arg) {
    char c;
    //arg->save_len = 0x1000000;
    arg->save_filename = "dump.sav";
    while ((c = getopt(argc, argv, "dwl:f:")) != -1) {
        switch (c) {
            case '?':
                exit(-1);
            case 'l':
                if (!optarg) exit(EXIT_FAILURE);
                arg->save_len = strtol(optarg, NULL, 0);
                break;
            case 'd':
                arg->dev = DUMP_SAVE;
                break;
            case 'w':
                arg->dev = WRITE_SAVE;
                break;
            case 'f':
                if (!optarg) exit(EXIT_FAILURE);
                arg->save_filename = optarg;
                break;
        }
    }
}

static const uint8_t dummy_cmd[] = {0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t chipid_cmd[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const uint8_t jedecid_cmd[] = {0x9F};
static const uint8_t status_cmd[] = {0x05};

int main(int argc, char *argv[]) {
    // struct args arg = {
    //     .dev = DEVICE_AUTO,
    // }
    // parse_args(argc, argv, &arg);

    uint8_t jedecid[3] = {0,0,0};
    uint8_t status = 0;

    powerslaves_mode(SPI_MODE);
    powerslaves_sendlen(SPI, 0x01, jedecid_cmd, 0x03);
    powerslaves_receive(jedecid, 0x03);
    powerslaves_sendlen(SPI, 0x01, status_cmd, 0x01);
    powerslaves_receive(&status, 0x01);

    printf("status, id\n%02x, %02x%02x%02x\n",
        status, jedecid[0], jedecid[1], jedecid[2]);

    // switch (arg.dev) {
    //     case DUMP_SAVE:
    //         puts("UNIMPLEMENTED!");
    //         return 1;
    //     case WRITE_SAVE:
    //         puts("UNIMPLEMENTED!");
    //         return 1;
    //     default:
    //         puts("This shouldn't happen.");
    //         return -1;
    // }
}
