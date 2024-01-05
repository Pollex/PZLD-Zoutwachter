#ifndef PCAL6524_PARAMS_H
#define PCAL6524_PARAMS_H

#include "pcal6524.h"

#ifndef PCAL6524_PARAM_I2C
#define PCAL6524_PARAM_I2C I2C_DEV(1)
#endif

#ifndef PCAL6524_PARAM_ADDR
#define PCAL6524_PARAM_ADDR 0x23
#endif

#ifndef PCAL6524_PARAMS
#define PCAL6524_PARAMS                                                        \
    { .i2c = PCAL6524_PARAM_I2C, .addr = PCAL6524_PARAM_ADDR }
#endif

static const pcal6524_params_t pcal6524_params[] = {PCAL6524_PARAMS};
#endif
