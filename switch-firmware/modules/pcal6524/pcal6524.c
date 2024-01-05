#include "pcal6524.h"
#include "periph/i2c.h"
#include <sys/errno.h>

#define ENABLE_DEBUG 1
#include "debug.h"
#define D(fmt, ...) DEBUG("[%s] " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ADDR_DEVICE_ID 0b01111100
#define PART_ID 0b100000110
#define REG_AI 0x80
#define REG_OUTPUT_0 0x04
#define REG_CONFIG_0 0x0C

#define ADDR p->params.addr
#define DEV p->params.i2c

typedef struct device_id_t {
    uint16_t manufacturer : 12;
    uint16_t part_id : 9;
    uint8_t revision : 3;
} device_id_t;

int read_device_id(pcal6524_t *p, device_id_t *result) {
    uint8_t buf[3];
    i2c_acquire(DEV);
    int err = i2c_read_regs(DEV, ADDR_DEVICE_ID, ADDR << 1, buf, 3, 0);
    i2c_release(DEV);

    result->manufacturer = (buf[0] << 4) | ((buf[1] & 0xF0) >> 4);
    result->part_id = ((buf[1] & 0x0F) << 5) | ((buf[2] & 0xF8) >> 3);
    result->revision = (buf[2] & 0x07);
    return err;
}

int read_output_pins(pcal6524_t *p, pcal6524_pins_t *pins) {
    int err = 0;

    i2c_acquire(DEV);
    err += i2c_read_regs(ADDR, ADDR, REG_OUTPUT_0 | REG_AI, &pins->bytes, 3, 0);
    i2c_release(DEV);

    return err;
}

int pcal6524_init(pcal6524_t *p, const pcal6524_params_t *params) {
    p->params = *params;

    device_id_t devid;
    int err = 0;
    if ((err = read_device_id(p, &devid)) < 0) {
        D("unable to read device id for %x, error %d\n", ADDR, err);
        return err;
    }
    D("read identifier. Manufacturer: %x, PartID: %x, Rev: %x\n",
      devid.manufacturer, devid.part_id, devid.revision);
    if (devid.part_id != PART_ID) {
        D("part_id different than expected for %x, expected: %x, got %x\n",
          ADDR, PART_ID, devid.part_id);
        return -ENODEV;
    }

    // Set all pins as outputs
    uint8_t tmp[3] = {0};
    i2c_acquire(DEV);
    if ((err = i2c_write_regs(DEV, ADDR, REG_CONFIG_0, tmp, 3, 0)) < 0) {
        i2c_release(DEV);
        return err;
    }
    i2c_release(DEV);

    return 0;
}

int pcal6524_set(pcal6524_t *p, pcal6524_pins_t pins) {
    int err = 0;
    pcal6524_pins_t current = {0};
    if ((err = read_output_pins(p, &current)) < 0) {
        D("could not read output reg for %x, error: %d\n", ADDR, err);
        return err;
    }
    current.raw |= pins.raw;
    if ((err = pcal6524_write(p, current)) < 0) {
        return err;
    }
    return 0;
}

int pcal6524_clear(pcal6524_t *p, pcal6524_pins_t pins) {
    int err = 0;
    pcal6524_pins_t current = {0};
    if ((err = read_output_pins(p, &current)) < 0) {
        D("could not write output reg for %x, error: %x\n", ADDR, err);
        return err;
    }
    current.raw &= ~(pins.raw);
    if ((err = pcal6524_write(p, current)) < 0) {
        return err;
    }
    return 0;
}

int pcal6524_write(pcal6524_t *p, pcal6524_pins_t pins) {
    i2c_acquire(DEV);
    int err =
        i2c_write_regs(DEV, ADDR, REG_OUTPUT_0 | REG_AI, pins.bytes, 3, 0);
    if (err < 0) {
        D("could not write output reg for %x, error: %x\n", ADDR, err);
        return err;
        i2c_release(DEV);
    }
    i2c_release(DEV);
    return err;
}
