#include "board.h"
#include "pcal6524.h"
#include "periph/gpio.h"
#include "periph/i2c.h"
#include "stm32l071xx.h"
#include "stws281x.h"
#include "ztimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pcal6524_t pcal;
pcal6524_pins_t pins;

static const pcal6524_params_t pcal6524_params[] = {
    {.i2c = I2C_DEV(1), .addr = 0x20},
    {.i2c = I2C_DEV(1), .addr = 0x21},
    {.i2c = I2C_DEV(1), .addr = 0x22},
    {.i2c = I2C_DEV(1), .addr = 0x23},
};

int main(void) {
    gpio_init(V5_EN, GPIO_OUT);
    gpio_set(V5_EN);
    stws281x_init();
    int ix = 0;
    for (;;) {
        stws281x_set(ix, 0, 50, 0);
        stws281x_write();
        ztimer_sleep(ZTIMER_MSEC, 200);
        stws281x_clear(ix);
        ix = (ix + 1) % 24;
    }

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
