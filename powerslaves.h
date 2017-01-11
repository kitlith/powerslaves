#pragma once

#ifndef POWERSLAVES_API
#define POWERSLAVES_API

#include <stdint.h>
#include <wchar.h>

/*! \brief Powersaves command type.
 * Magic numbers that need to be sent to the Powersaves MCU.
 */
/* TODO: Dump firmware on the MCU and complete this enum. */
enum powerslaves_cmdtype {
    TEST = 0x02,
    SWITCH_MODE = 0x10,
    NTR_MODE = 0x11, /* TODO: I'm sure there's more along this line. maybe 0x12 is CTR_MODE? */
    NTR = 0x13,
    CTR = 0x14,
    SPI = 0x15
};

/*! \brief Optional function that initializes a particular powersaves.
 * This function is called automatically by any function that needs a powersaves,
 * so it's only necessary to use this if you have multiple powersaves.
 *
 *  \param serial Serial number to specify an exact powersaves device. If NULL, automatic selection is preformed.
 *
 *  \return 0 on success, -1 on failure.
 */
int powerslaves_select(const wchar_t *serial);

/*! \brief Sends a cartridge command.
 *
 *  \param type Type of the command to be sent.
 *  \param cmdbuf Pointer to cartridge command.
 *  \param response_len Length of the response expected to be received.
 *
 *  \return Number of bytes sent on success, -1 on communication error, -2 on invalid paramater.
 */
int powerslaves_send(enum powerslaves_cmdtype type, const uint8_t *cmdbuf, uint16_t response_len);

/*! \brief Receives a response to a cartridge command.
 *
 *  \param buf Pointer to buffer that will receive the response.
 *  \param len Number of bytes to read into the buffer.
 *
 *  \return Number of bytes received on success, -2 on invalid handle.
 */
int powerslaves_receive(uint8_t *buf, uint16_t len);

/*! \brief Sends a cartridge command and receives a response.
 *
 *  \param type Type of the command to be sent.
 *  \param cmdbuf Pointer to cartridge command.
 *  \param response_len Length of the response expected to be received.
 *  \param resp Pointer to buffer that will receive the response.
 *
 *  \return Number of bytes received on success, -2 on invalid handle, -1 on communication error.
 */
int powerslaves_sendreceive(enum powerslaves_cmdtype type, const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp);

/* TODO: This feels like a kludge, perhaps when the command_type enum above
         is completed this could be changed to powerslaves_mode(enum mode mode)
         or something. */
int powerslaves_reset();

/*! \brief Deinitializes Powerslaves. */
void powerslaves_exit();

#endif /* end of include guard: POWERSLAVES_API */
