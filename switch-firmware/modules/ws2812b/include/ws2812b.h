/*
 * Copyright 2019 Marian Buschsieweke
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_ws2812b WS2812/SK6812 RGB LED (NeoPixel)
 * @ingroup     drivers_actuators
 * @brief       Driver for the WS2812 or the SK6812 RGB LEDs sold as NeoPixel
 *
 * # Summary
 *
 * The WS2812 or SK6812 RGB LEDs, or more commonly known as NeoPixels, can be
 * chained so that a single data pin of the MCU can control an arbitrary number
 * of RGB LEDs. This driver supports both the WS2812/SK6812 and the WS2812b
 * LEDs.
 *
 * # Support
 *
 * The protocol to communicate with the WS2812b is custom, so no hardware
 * implementations can be used. Hence, the protocol needs to be bit banged in
 * software. As the timing requirements are too strict to do this using
 * the platform independent APIs for accessing @ref drivers_periph_gpio and
 * @ref sys_xtimer, platform specific implementations of @ref ws2812b_write are
 * needed.
 *
 * ## ATmega
 *
 * A bit banging implementation for ATmegas clocked at 8MHz and at 16MHz is
 * provided. Boards clocked at any other core frequency are not supported.
 * (But keep in mind that most (all?) ATmega MCUs do have an internal 8MHz
 * oscillator, that could be enabled by changing the fuse settings.)
 *
 * @warning On 8MHz ATmegas, only pins at GPIO ports B, C, and D are supported.
 *          (On 16MHz ATmegas, any pin is fine.)
 *
 * ## ESP32
 *
 * The ESP32 implementation is frequency independent, as frequencies above 80MHz
 * are high enough to support bit banging without assembly.
 *
 * ## Native/VT100
 *
 * The native (VT100) implementation writes the LED state to the console.
 *
 * ### Usage
 *
 * Add the following to your `Makefile`:
 *
 * * Auto-selecting the backend:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Makefile
 * USEMODULE += ws2812b
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This will automatically pull in one of the backends supported by your board.
 * In case multiple backends apply and the automatic selection does not pick
 * your preferred backend, you can manually pick your preferred backend as
 * described below.
 *
 * * the ATmega backend:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Makefile
 * USEMODULE += ws2812b_atmega
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * * the ESP32 backend:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Makefile
 * USEMODULE += ws2812b_esp32
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * * the native/VT100 backend:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Makefile
 * USEMODULE += ws2812b_vt100
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @{
 *
 * @file
 * @brief       WS2812/SK6812 RGB LED Driver
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 */

#ifndef WS2812B_H
#define WS2812B_H

#include <stdint.h>

#include "color.h"
#include "periph/gpio.h"
#include "ws2812b_constants.h"
#include "xtimer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The number of bytes to allocate in the data buffer per LED
 */
#define WS2812B_BYTES_PER_DEVICE (3U)

/**
 * @brief   Struct to hold initialization parameters for a WS2812b RGB LED
 */
typedef struct {
    /**
     * @brief   A statically allocated data buffer storing the state of the LEDs
     *
     * @pre     Must be sized `numof * WS2812B_BYTES_PER_DEVICE` bytes
     */
    uint8_t *buf;
    uint16_t numof; /**< Number of chained RGB LEDs */
    gpio_t pin;     /**< GPIO connected to the data pin of the first LED */
} ws2812b_params_t;

/**
 * @brief   Device descriptor of a WS2812b RGB LED chain
 */
typedef struct {
    ws2812b_params_t params; /**< Parameters of the LED chain */
} ws2812b_t;

#if defined(WS2812B_HAVE_INIT) || defined(DOXYGEN)
/**
 * @brief   Initialize an WS2812b RGB LED chain
 *
 * @param   dev     Device descriptor to initialize
 * @param   params  Parameters to initialize the device with
 *
 * @retval  0       Success
 * @retval  -EINVAL Invalid argument
 * @retval  -EIO    Failed to initialize the data GPIO pin
 */
int ws2812b_init(ws2812b_t *dev, const ws2812b_params_t *params);
#else
static inline int ws2812b_init(ws2812b_t *dev, const ws2812b_params_t *params) {
    dev->params = *params;
    return 0;
}
#endif

/**
 * @brief   Writes the color data of the user supplied buffer
 *
 * @param   dev     Device descriptor of the LED chain to write to
 * @param   buf     Buffer to write
 * @param   size    Size of the buffer in bytes
 *
 * @pre     Before the transmission starts @ref ws2812b_prepare_transmission is
 *          called
 * @post    At the end of the transmission @ref ws2812b_end_transmission is
 *          called
 *
 * This function can be used to drive a huge number of LEDs with small data
 * buffers. However, after the return of this function the next chunk should
 * be send within a few microseconds to avoid accidentally sending the end of
 * transmission signal.
 *
 * Usage:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * uint8_t chunk[CHUNK_SIZE];
 * ws2812b_prepare_transmission(ws2812b_dev);
 * while (more_chunks_available()) {
 *    prepare_chunk(chunk);
 *    ws2812b_write_buffer(ws2812b_dev, chunk, sizeof(chunk));
 * }
 * ws2812b_end_transmission(dev);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
void ws2812b_write_buffer(ws2812b_t *dev, const void *buf, size_t size);

/**
 * @brief   Sets up everything needed to write data to the WS2812B LED chain
 *
 * @param   dev     Device descriptor of the LED chain to write to
 */
void ws2812b_prepare_transmission(ws2812b_t *dev);
/**
 * @brief   Ends the transmission to the WS2812/SK6812 LED chain
 *
 * @param   dev     Device descriptor of the LED chain to write to
 *
 * Does any cleanup the backend needs after sending data. In the simplest case
 * it simply waits for 80µs to send the end of transmission signal.
 */
void ws2812b_end_transmission(ws2812b_t *dev);

/**
 * @brief   Sets the color of an LED in the given data buffer
 *
 * @param   dest    Buffer to set the color in
 * @param   index   The index of the LED to set the color of
 * @param   color   The new color to apply
 *
 * @warning This change will not become active until @ref ws2812b_write is
 *          called
 */
void ws2812b_set_buffer(void *dest, uint16_t index, color_rgb_t color);

/**
 * @brief   Sets the color of an LED in the chain in the internal buffer
 *
 * @param   dev     Device descriptor of the LED chain to modify
 * @param   index   The index of the LED to set the color of
 * @param   color   The new color to apply
 *
 * @warning This change will not become active until @ref ws2812b_write is
 *          called
 */
static inline void ws2812b_set(ws2812b_t *dev, uint16_t index,
                               color_rgb_t color) {
    ws2812b_set_buffer(dev->params.buf, index, color);
}

/**
 * @brief   Writes the internal buffer to the LED chain
 *
 * @param   dev     Device descriptor of the LED chain to write to
 *
 * @note    This function implicitly calls @ref ws2812b_end_transmission
 * @see     ws2812b_set
 */
static inline void ws2812b_write(ws2812b_t *dev) {
    ws2812b_prepare_transmission(dev);
    ws2812b_write_buffer(dev, dev->params.buf,
                         (size_t)dev->params.numof * WS2812B_BYTES_PER_DEVICE);
    ws2812b_end_transmission(dev);
}

#ifdef __cplusplus
}
#endif

#endif /* WS2812B_H */
/** @} */
