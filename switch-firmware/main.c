#include "board.h"
#include "pcal6524.h"
#include "pcal6524_params.h"
#include "periph/i2c.h"
#include "ztimer.h"
#include <stdio.h>

pcal6524_t pcal;
pcal6524_pins_t pins;

int main(void) {
    puts("Zoutwachter Switch getting ready...");
    i2c_init(I2C_DEV(1));

    if (pcal6524_init(&pcal, pcal6524_params) < 0) {
        puts("Failed to initialize PCAL6524...");
        for (;;)
            ;
    }

    for (;;) {
        ztimer_sleep(ZTIMER_MSEC, 1000);
        pins.raw <<= 1;
        if ((pins.raw & 0x0F) == 0) {
            pins.raw = 1;
        }
        pcal6524_write(&pcal, pins);
        printf("Updated pins: %02X%02X%02X   %d   %d%d%d%d\n", pins.bytes[0],
               pins.bytes[1], pins.bytes[2], pins.raw, pins.pin0, pins.pin1,
               pins.pin2, pins.pin3);
    }
    return 0;
}
