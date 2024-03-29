/*
 * Copyright (C) 2019 Marian Buschsieweke
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ws2812b
 *
 * @{
 * @file
 * @brief       Default configuration for WS2812/SK6812 RGB LEDs
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 */

#ifndef WS2812B_PARAMS_H
#define WS2812B_PARAMS_H

#include "board.h"
#include "ws2812b.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default configuration parameters for WS2812b RGB LEDs
 * @{
 */
#ifndef WS2812B_PARAM_PIN
#define WS2812B_PARAM_PIN                (GPIO_PIN(0, 0)) /**< GPIO pin connected to the data pin of the first LED */
#endif
#ifndef WS2812B_PARAM_NUMOF
#define WS2812B_PARAM_NUMOF              (8U)            /**< Number of LEDs chained */
#endif
#ifndef WS2812B_PARAM_BUF
/**
 * @brief   Data buffer holding the LED states
 */
extern uint8_t ws2812b_buf[WS2812B_PARAM_NUMOF * WS2812B_BYTES_PER_DEVICE];
#define WS2812B_PARAM_BUF                (ws2812b_buf)  /**< Data buffer holding LED states */
#endif

#ifndef WS2812B_PARAMS
/**
 * @brief   WS2812b initialization parameters
 */
#define WS2812B_PARAMS                   { \
                                            .pin = WS2812B_PARAM_PIN,  \
                                            .numof = WS2812B_PARAM_NUMOF, \
                                            .buf = WS2812B_PARAM_BUF, \
                                        }
#endif
/**@}*/

/**
 * @brief   Initialization parameters for WS2812b devices
 */
static const ws2812b_params_t ws2812b_params[] =
{
    WS2812B_PARAMS
};

/** @brief Timer used for WS2812b (by the timer_gpio_ll implementation)
 *
 * A single timer is configured for any number of WS2812b strands, so this does
 * not need to be part of params.
 *
 * It is required that the timer has at least 2 channels. (Future versions may
 * require a 3rd channel).
 *
 * It is required that the timer's MAX_VALUE is 2^n-1, which is a trivial but
 * not explicitly stated case.
 *
 * This timer is configured at WS2812b initialization time, and kept stopped
 * outside of transmissions.
 *
 * The default value of 2 is chosen because the only platform on which the
 * module is usable is nRF5x, where TIMER_DEV(1) is in use by the radio module.
 * It is strongly advised to explicitly set this timer to a known free timer,
 * as the default may change without notice.
 * */
#if !defined(WS2812B_TIMER_DEV) || defined(DOXYGEN)
#define WS2812B_TIMER_DEV TIMER_DEV(2)
#endif

/** @brief Maximum value of the timer used for WS2812b (by the timer_gpio_ll implementation)
 *
 * This macro needs to be defined to the `TIMER_x_MAX_VALUE` corresponding to
 * the `TIMER_DEV(x)` in @ref WS2812B_TIMER_DEV.
 * */
#ifndef WS2812B_TIMER_MAX_VALUE
#define WS2812B_TIMER_MAX_VALUE TIMER_2_MAX_VALUE
#endif

/** @brief Frequency for the timer used for WS2812b (by the timer_gpio_ll implementation)
 *
 * This should be set to a frequency that is a close multiple of 3MHz,
 * depending on the precise low and high times. A value of 16MHz works well.
 * */
#ifndef WS2812B_TIMER_FREQ
#define WS2812B_TIMER_FREQ 16000000
#endif

#ifdef __cplusplus
}
#endif

#endif /* WS2812B_PARAMS_H */
/** @} */
