#include "app_switching.h"
#include "pcal6524.h"
#include "stws281x.h"
#define ENABLE_DEBUG 1
#include "debug.h"

#define ELECTRODE_COUNT 24

pcal6524_t pcals[4];

static const pcal6524_params_t pcal6524_params[] = {
    {.i2c = I2C_DEV(1), .addr = 0x20},
    {.i2c = I2C_DEV(1), .addr = 0x21},
    {.i2c = I2C_DEV(1), .addr = 0x22},
    {.i2c = I2C_DEV(1), .addr = 0x23},
};

/*
 * Pin 1..6 on 0x20
 *      F1 14,15,16,17
 *      F2 20,21,22,23
 *      F3 24,25,26,27
 *      F4 00,01,02,03
 *      F5 04,05,06,07
 *      F6 10,11,12,13
 * Pin 7..12 on 0x21
 * Pin 13..18 on 0x22
 * Pin 18..23 on 0x23
 */
#define _pcal(x) &pcals[((x >> 5) & 0b111)]
#define _pin(x) ((x) & 0b11111)
uint8_t pcal_lookup[24] = {
    // <PCAL index>       <Pin M0 index>
    ((0 & 0b111) << 5) | (12 & 0b11111), //
    ((0 & 0b111) << 5) | (16 & 0b11111), //
    ((0 & 0b111) << 5) | (20 & 0b11111), //
    ((0 & 0b111) << 5) | (0 & 0b11111),  //
    ((0 & 0b111) << 5) | (4 & 0b11111),  //
    ((0 & 0b111) << 5) | (8 & 0b11111),  //

    ((1 & 0b111) << 5) | (12 & 0b11111), //
    ((1 & 0b111) << 5) | (16 & 0b11111), //
    ((1 & 0b111) << 5) | (20 & 0b11111), //
    ((1 & 0b111) << 5) | (0 & 0b11111),  //
    ((1 & 0b111) << 5) | (4 & 0b11111),  //
    ((1 & 0b111) << 5) | (8 & 0b11111),  //

    ((2 & 0b111) << 5) | (12 & 0b11111), //
    ((2 & 0b111) << 5) | (16 & 0b11111), //
    ((2 & 0b111) << 5) | (20 & 0b11111), //
    ((2 & 0b111) << 5) | (0 & 0b11111),  //
    ((2 & 0b111) << 5) | (4 & 0b11111),  //
    ((2 & 0b111) << 5) | (8 & 0b11111),  //

    ((3 & 0b111) << 5) | (12 & 0b11111), //
    ((3 & 0b111) << 5) | (16 & 0b11111), //
    ((3 & 0b111) << 5) | (20 & 0b11111), //
    ((3 & 0b111) << 5) | (0 & 0b11111),  //
    ((3 & 0b111) << 5) | (4 & 0b11111),  //
    ((3 & 0b111) << 5) | (8 & 0b11111),  //
};

uint8_t active_a_to_b_mapping[4] = {0xff, 0xff, 0xff, 0xff};

/**
 * @brief Connects the 4 A side pins to the requested B side pins.
 *
 * @param requested_a_to_b Array of 4 B side pins where the index represents the
 * A side pin it's connected to.
 */
int app_switching_connect(uint8_t requested_a_to_b_args[4]) {
    // Remove current connections
    for (int i = 0; i < 4; i++) {
        pcals[i].pins.raw = 0;
        pcal6524_write(&pcals[i]);
    }

    // Set new connection
    for (int a_side_pin = 0; a_side_pin < 4; a_side_pin++) {
        // Get the requested IO expander and pin the given b side pin index
        uint8_t b_side_pin = requested_a_to_b_args[a_side_pin];
        pcal6524_t *pcal = _pcal(pcal_lookup[b_side_pin]);
        // The given pin connects the B side to A side pin 0, add 1 for A side
        // pin 1, 2 for pin 2 etc.
        uint8_t pcal_pin = _pin(pcal_lookup[b_side_pin]) + a_side_pin;

        // Update the IO expander state, this will not write it yet
        pcal6524_set(pcal, pcal_pin);
        active_a_to_b_mapping[a_side_pin] = b_side_pin;
    }

    // Actually write the new io expander state
    for (int i = 0; i < 4; i++) {
        pcal6524_write(&pcals[i]);
    }

    return 0;
}

int app_switching_init(void) {
    DEBUG("Initializing PCAL IO Expanders... ");
    i2c_init(I2C_DEV(1));
    for (int i = 0; i < 4; i++) {
        int result;
        result = pcal6524_init(&pcals[i], &pcal6524_params[i]);
        if (result < 0) {
            DEBUG("Error on %d: ERR %d\n", i, result);
            // TODO: What to do when we error!?
            for (;;)
                ;
        }
    }
    DEBUG("OK\n");
    return 0;
}
