#ifndef PCAL6524_H
#define PCAL6524_H

#include "periph/i2c.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union pcal6524_pins_t {
    union {
        uint8_t bytes[3];
        uint32_t raw : 24;
        struct __attribute__((packed)) {
            uint8_t pin0 : 1;
            uint8_t pin1 : 1;
            uint8_t pin2 : 1;
            uint8_t pin3 : 1;
            uint8_t pin4 : 1;
            uint8_t pin5 : 1;
            uint8_t pin6 : 1;
            uint8_t pin7 : 1;
            uint8_t pin8 : 1;
            uint8_t pin9 : 1;
            uint8_t pin10 : 1;
            uint8_t pin11 : 1;
            uint8_t pin12 : 1;
            uint8_t pin13 : 1;
            uint8_t pin14 : 1;
            uint8_t pin15 : 1;
            uint8_t pin16 : 1;
            uint8_t pin17 : 1;
            uint8_t pin18 : 1;
            uint8_t pin19 : 1;
            uint8_t pin20 : 1;
            uint8_t pin21 : 1;
            uint8_t pin22 : 1;
            uint8_t pin23 : 1;
        };
    };
} pcal6524_pins_t;

typedef struct {
    i2c_t i2c;
    uint8_t addr;
} pcal6524_params_t;

typedef struct {
    pcal6524_params_t params;
    pcal6524_pins_t pins;
} pcal6524_t;

int pcal6524_init(pcal6524_t *p, const pcal6524_params_t *params);
int pcal6524_set(pcal6524_t *p, uint8_t pin);
int pcal6524_clear(pcal6524_t *p, uint8_t pin);
int pcal6524_write(pcal6524_t *p);

#ifdef __cplusplus
}
#endif

#endif // !PCAL6524_H
