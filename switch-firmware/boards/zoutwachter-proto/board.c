#include "board.h"
#include "periph/gpio.h"

void board_init(void)
{
    gpio_init(GPIO_PIN(PORT_A, 2), GPIO_OUT);
    gpio_set(GPIO_PIN(PORT_A, 2));
}
