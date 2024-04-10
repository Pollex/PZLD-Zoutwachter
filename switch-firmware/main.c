#include "board.h"
#include "pcal6524.h"
#include "periph/gpio.h"
#include "periph/i2c.h"
#include "shell.h"
#include "stws281x.h"
#include "ztimer.h"
#include <stdio.h>
#include <stdlib.h>

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

uint8_t fm_color[4][3] = {
    {10, 0, 0},
    {0, 10, 0},
    {0, 0, 10},
    {10, 10, 10},
};

uint8_t m_to_f[4] = {0xff, 0xff, 0xff, 0xff};

void connect(uint8_t m, uint8_t f) {
    pcal6524_t *pcal = _pcal(pcal_lookup[f]);
    uint8_t pcal_pin = _pin(pcal_lookup[f]) + m;

    // Remove old connection
    uint8_t old_f = m_to_f[m];
    if (old_f != 0xff) {
        pcal6524_clear(pcal, _pin(pcal_lookup[old_f]) + m);
        stws281x_clear(old_f);
    }
    printf("CONNECT M%d to F%d (was F%d)\n", m, f, old_f);

    // Set new connection
    pcal6524_set(pcal, pcal_pin);
    m_to_f[m] = f;
    uint8_t *c = fm_color[m];
    stws281x_set(f, c[0], c[1], c[2]);

    // Commit 
    pcal6524_write(pcal);
    stws281x_write();
}

static int _cmd_configure(int argc, char **argv) {
    (void)argv;
    if (argc <= 4) {
        puts("usage: configure <E1> <E2> <E3> <E4>");
        return 1;
    }
    connect(0, atoi(argv[1])-1);
    connect(1, atoi(argv[2])-1);
    connect(2, atoi(argv[3])-1);
    connect(3, atoi(argv[4])-1);
    return 0;
}

static const shell_command_t shell_commands[] = {
    {"configure", "Connect 4 F electrodes to M side", _cmd_configure},
    {NULL, NULL, NULL}};

int main(void) {
    puts("Initializing LEDs... ");
    gpio_init(V5_EN, GPIO_OUT);
    gpio_set(V5_EN);
    ztimer_sleep(ZTIMER_MSEC, 1);
    stws281x_init();
    puts("OK\n");

    puts("Initializing PCAL IO Expanders... ");
    i2c_init(I2C_DEV(1));
    for (int i = 0; i < 4; i++) {
        if (pcal6524_init(&pcals[i], &pcal6524_params[i]) < 0) {
            printf("FAIL (%d)\n", i);
            for (;;)
                ;
        }
    }
    puts("OK\n");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
