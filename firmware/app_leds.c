#include "app_leds.h"
#include "periph/gpio.h"
#include "stws281x.h"
#include "ztimer.h"
#include <stdint.h>
#define ENABLE_DEBUG 1
#include "debug.h"

uint8_t pin_colors[4][3] = {
    {10, 0, 0},
    {0, 10, 0},
    {0, 0, 10},
    {10, 10, 10},
};

void on_switching_update(uint8_t a_to_b_side[4]) {
    // clear LEDS
    stws281x_clearall();

    // Update the LED representation
    for (int a_side = 0; a_side < 4; a_side++) {
        uint8_t b_side = a_to_b_side[a_side];
        uint8_t *c = pin_colors[a_side];
        stws281x_set(b_side, c[0], c[1], c[2]);
    }

    // Write the new LED state
    stws281x_write();
}

int app_leds_init(void) {
    DEBUG("Initializing LEDs... ");
    gpio_init(V5_EN, GPIO_OUT);
    gpio_set(V5_EN);
    ztimer_sleep(ZTIMER_MSEC, 1);
    stws281x_init();
    DEBUG("OK\n");
    return 0;
}
