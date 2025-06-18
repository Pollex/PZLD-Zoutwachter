#include "app_functional_tests.h"
#include "board.h"
#include "periph/gpio.h"
#include "shell.h"
#include "stws281x.h"
#include "ztimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_io(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if (argc == 1) {
        puts("test_io <name> [1/0 on/off]");
        return 0;
    }
    uint8_t onoff = 1;
    if (argc == 3) {
        if (strcmp(argv[2], "on") == 0) {
            onoff = 1;
        } else if (strcmp(argv[2], "off") == 0) {
            onoff = 0;
        } else {
            onoff = atoi(argv[2]);
        }
    }

    if (strcmp(argv[1], "5V") == 0) {
        gpio_init(V5_EN, GPIO_OUT);
        gpio_write(V5_EN, onoff);
        return 0;
    }
    if (strcmp(argv[1], "BUCK") == 0) {
        puts("MCU POWER IS FED FROM THE BUCK!");
        gpio_init(BUCK_EN, GPIO_OUT);
        gpio_write(BUCK_EN, onoff);
        return 0;
    }

    puts("PIN not found");

    return 0;
}

static int test_leds(int argc, char **argv) {
    (void)argc;
    (void)argv;
    gpio_init(V5_EN, GPIO_OUT);
    gpio_set(V5_EN);
    ztimer_sleep(ZTIMER_MSEC, 1);
    stws281x_init();

    puts("Clear all");
    stws281x_clearall();
    stws281x_write();
    for (int ix = 0; ix < 28; ix++) {
        stws281x_set(ix, 50, 50, 50);
    }
    puts("Setting all for 5 seconds");
    stws281x_write();
    ztimer_sleep(ZTIMER_MSEC, 5000);
    puts("Clearing");
    stws281x_clearall();
    stws281x_write();
    return 0;
}

static const shell_command_t shell_commands[] = {
    {"test_io", "Enable or disable specific IO pins", test_io},
    {"test_leds", "Test all LEDs in segments for 3 seconds", test_leds},
    {NULL, NULL, NULL}};

void app_fat_begin(void) {
    puts("====================================");
    puts("=   Entering Functional Tests      =");
    puts("====================================\n");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
}
