#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"

#define LED_OUT GPIO_PIN(PORT_B, 13)
#define LED_IN GPIO_PIN(PORT_A, 0)
#define IOEXP_RST GPIO_PIN(PORT_A, 2)
#define USB_CONNECTED GPIO_PIN(PORT_A, 15)
#define BUCK_EN GPIO_PIN(PORT_B, 14)
#define V5_EN GPIO_PIN(PORT_B, 15)


#endif /* end of include guard: BOARD_H */
