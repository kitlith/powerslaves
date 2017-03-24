#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h> // getopt

#include <powerslaves.h>

enum behavior {
    AK2I_DUMP_FLASH,
    AK2I_WRITE_FLASH,
    DEVICE_AUTO = AK2I_DUMP_FLASH
};

struct args {
    unsigned flash_len;
    const char *flash_filename;
    enum behavior dev;
};

static const uint8_t ak2i_cmdGetHWRevision[] = {0xD1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t ak2i_cmdSetMapTableAddress[] = {0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t ak2i_cmdSetFlash1681_81[] = {0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC6, 0x06};
static const uint8_t ak2i_cmdActiveFatMap[] = {0xC2, 0x55, 0xAA, 0x55, 0xAA, 0x00, 0x00, 0x00};
static const uint8_t ak2i_cmdUnlockFlash[] = {0xC2, 0xAA, 0x55, 0xAA, 0x55, 0x00, 0x00, 0x00};
static const uint8_t ak2i_cmdUnlockASIC[] = {0xC2, 0xAA, 0x55, 0x55, 0xAA, 0x00, 0x00, 0x00};

static uint32_t ak2iSetup() { // Returns HW Revision
    uint32_t hw_revision;
    powerslaves_sendreceive(NTR, ak2i_cmdGetHWRevision, 0, (uint8_t*)&hw_revision); // Get HW Revision
    powerslaves_send(NTR, ak2i_cmdSetMapTableAddress, 0);

    if (hw_revision & 1) {
        powerslaves_send(NTR, ak2i_cmdSetFlash1681_81, 0);
    }

    {
        uint8_t garbage[4];
        powerslaves_sendreceive(NTR, ak2i_cmdActiveFatMap, 4, garbage);
    }

    powerslaves_send(NTR, ak2i_cmdUnlockFlash, 0);
    powerslaves_send(NTR, ak2i_cmdUnlockASIC, 0);

    return hw_revision;
}

static const uint8_t ak2i_cmdWaitFlashBusy[] = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static void ak2i_waitFlashBusy() {
    while (true) {
        uint8_t state[4];
        powerslaves_sendreceive(NTR, ak2i_cmdWaitFlashBusy, 4, state);
        if((state[3] & 1) == 0) break;
    }
}

static void parse_args(int argc, char *argv[], struct args *arg) {
    char c;
    while ((c = getopt(argc, argv, "l:df:")) != -1) {
        switch (c) {
            case '?':
                exit(-1);
            case 'l':
                if (!optarg) exit(EXIT_FAILURE);
                arg->flash_len = strtol(optarg, NULL, 0);
                break;
            case 'd':
                arg->dev = AK2I_DUMP_FLASH;
                arg->flash_filename = "ak2i_flash.bin";
                arg->flash_len = 0x1000000;
                break;
            case 'f':
                if (!optarg) exit(EXIT_FAILURE);
                arg->dev = AK2I_WRITE_FLASH;
                arg->flash_filename = optarg;
                arg->flash_len = 0x20000;
                break;
        }
    }
}

static const uint8_t dummy_cmd[] = {0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t chipid_cmd[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define readChipID(buf) powerslaves_sendreceive(NTR, chipid_cmd, 4, buf)

static const uint8_t ak2i_cmdReadFlash[] = {0xB7, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00};
static const uint8_t ak2i_cmdEraseFlash[2][8] = {
    {0xD4, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00},
    {0xD4, 0x00, 0x00, 0x00, 0x30, 0x80, 0x00, 0x35}
};

static const uint8_t ak2i_cmdWriteByteFlash[2][8] = {
    {0xD4, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00},
    {0xD4, 0x00, 0x00, 0x00, 0x30, 0xa0, 0x00, 0x63}
};

static const uint8_t ak2i_cmdLockFlash[] = {0xC2, 0xAA, 0xAA, 0x55, 0x55, 0x00, 0x00, 0x00};

int main(int argc, char *argv[]) {
    struct args arg = {
        .flash_len = 0x1000000,
        .flash_filename = "ak2i_flash.bin",
        .dev = DEVICE_AUTO
    };
    parse_args(argc, argv, &arg);

    if (arg.flash_filename) printf("Flashing file %s to ak2i.", arg.flash_filename);

    uint8_t *header = (uint8_t*)malloc(arg.flash_len);
    {
        uint8_t *garbage = (uint8_t*)malloc(0x2000);
        if (!header || !garbage) {
            puts("Memory allocation failure!");
        }

        powerslaves_mode(ROM_MODE);
        powerslaves_sendreceive(NTR, dummy_cmd, 0x2000, garbage);
        free(garbage);
    }

    uint8_t chipid[0x4] = {0x00, 0x00, 0x00, 0x00};

    switch (arg.dev) {
        case AK2I_DUMP_FLASH: {
                uint8_t ak2ibuf[8];
                readChipID(chipid);
                if (*(uint32_t*)chipid != 0x00000FC2) {
                    printf("Not a AK2i! ChipID: %02x%02x%02x%02x\n",
                        chipid[0], chipid[1], chipid[2], chipid[3]);
                    exit(EXIT_FAILURE);
                }
                uint32_t hw_revision = ak2iSetup();
                memset(ak2ibuf, 0, sizeof(ak2ibuf));

                unsigned length = (hw_revision & 1) ? 0x1000000 : 0x200000;
                length = (length < arg.flash_len) ? length : arg.flash_len;

                memcpy(ak2ibuf, ak2i_cmdReadFlash, 8);
                for (uint32_t addr=0; addr < length; addr += 0x200) {
                    ak2ibuf[1] = (addr >> 24) & 0xFF;
                    ak2ibuf[2] = (addr >> 16) & 0xFF;
                    ak2ibuf[3] = (addr >>  8) & 0xFF;
                    ak2ibuf[4] = (addr >>  0) & 0xFF;
                    powerslaves_sendreceive(NTR, ak2ibuf, 0x200, header + addr);
                }
                printf("AK2i Revision: 0x%04x\n", hw_revision);
            }
            break;
        case AK2I_WRITE_FLASH:  {
                FILE *flash_file = fopen(arg.flash_filename, "rb");
                if (!flash_file) {
                    printf("Error opening file.\n");
                    exit(EXIT_FAILURE);
                }
                fread(header, arg.flash_len, 1, flash_file);
                fclose(flash_file);

                uint8_t ak2ibuf[8];
                readChipID(chipid);
                if (*(uint32_t*)chipid != 0x00000FC2) {
                    printf("Not a AK2i! ChipID: %02x%02x%02x%02x\n",
                        chipid[0], chipid[1], chipid[2], chipid[3]);
                    exit(EXIT_FAILURE);
                }
                uint32_t hw_revision = ak2iSetup();

                printf("Erasing Flash.\n");
                memcpy(ak2ibuf, ak2i_cmdEraseFlash[hw_revision & 1], 8);
                for (uint32_t addr=0; addr < arg.flash_len; addr+=64*1024) {
                    ak2ibuf[1] = (addr >> 16) & 0x1F;
                    ak2ibuf[2] = (addr >>  8) & 0xFF;
                    ak2ibuf[3] = (addr >>  0) & 0xFF;
                    powerslaves_send(NTR, ak2ibuf, 0);
                    ak2i_waitFlashBusy();
                }

                printf("Flashing. This may take a while.\n");
                memcpy(ak2ibuf, ak2i_cmdWriteByteFlash[hw_revision & 1], 8);
                for (uint32_t addr=0; addr < arg.flash_len; ++addr) {
                    ak2ibuf[1] = (addr >> 16) & 0x1F;
                    ak2ibuf[2] = (addr >>  8) & 0xFF;
                    ak2ibuf[3] = (addr >>  0) & 0xFF;
                    ak2ibuf[4] = header[addr];
                    powerslaves_send(NTR, ak2ibuf, 0);
                    ak2i_waitFlashBusy();
                }

                printf("Done!\n");
                powerslaves_send(NTR, ak2i_cmdLockFlash, 0);
                powerslaves_sendreceive(NTR, ak2i_cmdActiveFatMap, 4, ak2ibuf);
            }
            break;
        default:
            printf("This should never happen.\n");
            exit(EXIT_FAILURE);
    }

    free(header);
    powerslaves_exit();

    return EXIT_SUCCESS;
}
