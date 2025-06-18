#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "stm32l071xx.h"
#define CONFIG_CLOCK_PLL_DIV            (2)
#define CONFIG_CLOCK_PLL_MUL            (8)
#define CONFIG_BOARD_HAS_HSE 1
#define CONFIG_CLOCK_HSE MHZ(8)

#include "board.h"
#include "cfg_rtt_default.h"
#include "clk_conf.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @name   Timer configuration
 * @{
 */

// TIM2 is used by the LED driver and can't be used by RIOT
static const timer_conf_t timer_config[] = {
    //{
    //    .dev      = TIM2,
    //    .max      = 0x0000ffff,
    //    .rcc_mask = RCC_APB1ENR_TIM2EN,
    //    .bus      = APB1,
    //    .irqn     = TIM2_IRQn
    //}
    {
        .dev      = TIM3,
        .max      = 0x0000ffff,
        .rcc_mask = RCC_APB1ENR_TIM3EN,
        .bus      = APB1,
        .irqn     = TIM3_IRQn
    }
};

#define TIMER_0_ISR         isr_tim3

#define TIMER_NUMOF         ARRAY_SIZE(timer_config)
/** @} */


static const uart_conf_t uart_config[] = {
  {
    .dev = USART1,
    .rcc_mask = RCC_APB2ENR_USART1EN,
    .rx_pin = USB_RX,
    .tx_pin = USB_TX,
    .rx_af = GPIO_AF4,
    .tx_af = GPIO_AF4,
    .bus = APB2,
    .irqn = USART1_IRQn,
    .type = STM32_USART,
    .clk_src = 0, /* Use APB clock */
  },
  {
    .dev = USART4,
    .rcc_mask = RCC_APB1ENR_USART4EN,
    .rx_pin = RS485_RX,
    .tx_pin = RS485_TX,
    .rx_af = GPIO_AF6,
    .tx_af = GPIO_AF6,
    .bus = APB1,
    .irqn = USART4_5_IRQn,
    .type = STM32_USART,
    .clk_src = 0, /* Use APB clock */
  }
};
#define UART_0_ISR          (isr_usart1)
#define UART_1_ISR          (isr_usart4)
#define UART_NUMOF          ARRAY_SIZE(uart_config)

static const i2c_conf_t i2c_config[] = {
    {
        .dev            = I2C1,
        .speed          = I2C_SPEED_NORMAL,
        .scl_pin        = GPIO_PIN(PORT_B, 8),
        .sda_pin        = GPIO_PIN(PORT_B, 9),
        .scl_af         = GPIO_AF4,
        .sda_af         = GPIO_AF4,
        .rcc_mask       = RCC_APB1ENR_I2C1EN,
        .irqn           = I2C1_IRQn,
    },
    {
        .dev            = I2C2,
        .speed          = I2C_SPEED_NORMAL,
        .scl_pin        = GPIO_PIN(PORT_B, 10),
        .sda_pin        = GPIO_PIN(PORT_B, 11),
        .scl_af         = GPIO_AF6,
        .sda_af         = GPIO_AF6,
        .rcc_mask       = RCC_APB1ENR_I2C2EN,
        .irqn           = I2C2_IRQn,
    },
};

#define I2C_0_ISR           isr_i2c1
#define I2C_1_ISR           isr_i2c2

#define I2C_NUMOF           ARRAY_SIZE(i2c_config)

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: PERIPH_CONF_H */
