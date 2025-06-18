#ifndef BOARD_H
#define BOARD_H

#define LED_DATA GPIO_PIN(PORT_A, 5)
#define IOEXP_RST GPIO_PIN(PORT_A, 2)
#define USB_CONNECTED GPIO_PIN(PORT_A, 15)
#define HOST_INT GPIO_PIN(PORT_B, 3)
#define BUCK_EN GPIO_PIN(PORT_B, 14)
#define V5_EN GPIO_PIN(PORT_B, 15)

#define RS485_TX GPIO_PIN(PORT_A, 0)
#define RS485_RX GPIO_PIN(PORT_A, 1)
#define USB_TX GPIO_PIN(PORT_A, 9)
#define USB_RX GPIO_PIN(PORT_A, 10)

#endif /* end of include guard: BOARD_H */
