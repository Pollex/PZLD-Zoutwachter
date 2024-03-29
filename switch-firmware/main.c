#include "board.h"
#include "pcal6524.h"
#include "periph/gpio.h"
#include "periph/i2c.h"
#include "stm32l071xx.h"
#include "ztimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pcal6524_t pcal;
pcal6524_pins_t pins;

#define LED_OUT_COUNT 24
#define LED_IDLE_COUNT 1000
#define LED_BUF_SIZE 24 * LED_OUT_COUNT + LED_IDLE_COUNT
#define LED_L_CCR 13
#define LED_H_CCR 26

static const pcal6524_params_t pcal6524_params[] = {
    {.i2c = I2C_DEV(1), .addr = 0x20},
    {.i2c = I2C_DEV(1), .addr = 0x21},
    {.i2c = I2C_DEV(1), .addr = 0x22},
    {.i2c = I2C_DEV(1), .addr = 0x23},
};

void isr_dma1_channel4_5_6_7(void) {
    // DMA1_Channel5->CNDTR = 8*3*LED_OUT_COUNT+15;
}

static uint32_t led_buf[LED_BUF_SIZE] = {0};

void led_set(int led, uint8_t R, uint8_t G, uint8_t B) {
    uint32_t p = (G << 16) | (R << 8) | B;
    uint32_t *ptr = &led_buf[led * 24];
    while (p > 0) {
        if ((p & 1) > 0) {
            *ptr = LED_H_CCR;
        } else {
            *ptr = LED_L_CCR;
        }
        ptr++;
        p >>= 1;
    }
}
void led_clear(int led) {
    for (int i = led * 24; i < led * 24 + 24; i++) {
        led_buf[i] = LED_L_CCR;
    }
}

int main(void) {
    // 32 MHz
    // Period  = 1.25us
    // Duty lo = 33%
    // Duty hi = 66%
    //
    // Fpwm = FCLK / (ARR+1) * (PSC+1))
    // 800kHz = 32MHz / (39 * 1)
    //
    // Duty lo = 33% = 13
    // Duty hi = 66% = 26
    //
    // PB13 = LED_OUT = TIM21_CH1
    // PA0 = LED_IN = TIM2_CH1
    //
    gpio_init(V5_EN, GPIO_OUT);
    gpio_set(V5_EN);

    // Setup DMA
    // CxS = 1000
    // TIM2_CH1 on CH5
    //
    // ADDR in DMA_CPARx or DMA_CMARx in circular mode
    // DMA_CNDTRx = total number of data to transfer
    // DMA_CCRx for configuration
    //  - direction (1 = MEM2PER)
    //      DMAR for memory
    //      DPAR for periph
    //  - circular
    //  - other stuff
    // Activate with ENABLE bit in DMA_CCRx
    // DMA Interrupts on TCIFx TCIEx = transfer complete channel X

    // 8 bits, 3 bytes, 24 LEDs + 15 RESET cycles

    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_CSELR->CSELR |= 0b1000 << DMA_CSELR_C5S_Pos;
    DMA1_Channel5->CPAR = (uint32_t)(&TIM2->CCR1);
    DMA1_Channel5->CMAR = (uint32_t)led_buf;
    DMA1_Channel5->CNDTR = LED_BUF_SIZE;
    DMA1_Channel5->CCR |= DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1 // WORD (32 bits)
                          | DMA_CCR_MINC // Increment memory addr
                          //| DMA_CCR_TCIE                    //
                          | DMA_CCR_DIR   //
                          | DMA_CCR_CIRC; //
    DMA1_Channel5->CCR |= DMA_CCR_EN;

    // ====
    // TIM2
    // ====
    GPIOA->MODER |= GPIO_MODER_MODE0_1;
    GPIOA->MODER &= ~GPIO_MODER_MODE0_0;
    GPIOA->AFR[0] |= 2 << 0; // Set AF2 for Pin 0
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEED0_1 | GPIO_OSPEEDER_OSPEED0_0;

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC = 0;
    TIM2->ARR = 39;
    TIM2->CCR1 = 0;

    TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
    TIM2->CCER |= TIM_CCER_CC1E; // CC1E Capture/Compare 1 output Enable
    TIM2->EGR = TIM_EGR_UG; // Set UG bit to trigger shadow register transfer
    TIM2->DIER = TIM_DIER_CC1DE;
    TIM2->CR1 = TIM_CR1_CEN     // CEN Counter ENable
                | TIM_CR1_ARPE; //

    int led = 0;
    int dir = 1;
    int colors[3] = {rand(), rand(), rand()};
    for (;;) {
        led_set(led, colors[0], colors[1], colors[2]);
        ztimer_sleep(ZTIMER_MSEC, 30);
        led_clear(led);
        led += dir;
        if (led == LED_OUT_COUNT || led < 0) {
            colors[0] = rand();
            colors[1] = rand();
            colors[2] = rand();
            dir *= -1;
        led += dir;
        }
    }

    return 0;
    puts("Zoutwachter Switch getting ready...");
    i2c_init(I2C_DEV(1));

    if (pcal6524_init(&pcal, pcal6524_params) < 0) {
        puts("Failed to initialize PCAL6524...");
        for (;;)
            ;
    }

    for (;;) {
        ztimer_sleep(ZTIMER_MSEC, 1000);
        pins.raw <<= 1;
        if ((pins.raw & 0x0F) == 0) {
            pins.raw = 1;
        }
        pcal6524_write(&pcal, pins);
        printf("Updated pins: %02X%02X%02X   %d   %d%d%d%d\n", pins.bytes[0],
               pins.bytes[1], pins.bytes[2], pins.raw, pins.pin0, pins.pin1,
               pins.pin2, pins.pin3);
    }
    return 0;
}
