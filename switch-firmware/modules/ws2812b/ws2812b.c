/*
 * Copyright 2019 Marian Buschsieweke
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ws2812b
 *
 * @{
 *
 * @file
 * @brief       Driver for the WS2812 or the SK6812 RGB LEDs sold as NeoPixel
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 *
 * @}
 */

#include <stdint.h>
#include "ws2812b.h"
#include "ws2812b_constants.h"
#include "ws2812b_params.h"

/* Default buffer used in ws2812b_params.h. Will be optimized out if unused */
uint8_t ws2812b_buf[WS2812B_PARAM_NUMOF * WS2812B_BYTES_PER_DEVICE];

void ws2812b_set_buffer(void *_dest, uint16_t n, color_rgb_t c)
{
    uint8_t *dest = _dest;
    dest[WS2812B_BYTES_PER_DEVICE * n + WS2812B_OFFSET_R] = c.r;
    dest[WS2812B_BYTES_PER_DEVICE * n + WS2812B_OFFSET_G] = c.g;
    dest[WS2812B_BYTES_PER_DEVICE * n + WS2812B_OFFSET_B] = c.b;
}

void ws2812b_write_buffer(ws2812b_t *dev, const void *buf, size_t size)
{
    (void) dev;
    const uint8_t *src = buf;

    for (unsigned i = 0; i < size; i += WS2812B_BYTES_PER_DEVICE) {
        int r = src[i + WS2812B_OFFSET_R];
        int g = src[i + WS2812B_OFFSET_G];
        int b = src[i + WS2812B_OFFSET_B];
        printf("\033[48;2;%d;%d;%dm ", r, g, b);
    }
}

void ws2812b_prepare_transmission(ws2812b_t *dev)
{
    (void) dev;

    /* clear the line and reset cursor position */
    printf("\033[2K\r");
}

void ws2812b_end_transmission(ws2812b_t *dev)
{
    (void) dev;

    /* set color back to normal */
    printf("\033[0m");
}
