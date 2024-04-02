#include "stws281x.h"
#include "stm32l071xx.h"
#include <string.h>

#define LED_L_CCR 13
#define LED_H_CCR 26

#define LED_COUNT 24
#define LED_IDLE_COUNT 120
#define LED_BUFFER_SIZE 24 * LED_COUNT + LED_IDLE_COUNT
uint8_t led_buffer[LED_BUFFER_SIZE] = {0};

void stws281x_init(void) {
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

    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_CSELR->CSELR |= 0b1000 << DMA_CSELR_C5S_Pos;
    DMA1_Channel5->CPAR = (uint32_t)(&TIM2->CCR1);
    // Periph is 32bit, Source is 8 bit
    DMA1_Channel5->CCR |= DMA_CCR_PSIZE_1 //| DMA_CCR_MSIZE_1 // WORD (32 bits)
                          | DMA_CCR_MINC  // Increment memory addr
                          | DMA_CCR_DIR;  //
                                          //| DMA_CCR_CIRC; //
                                          //| DMA_CCR_TCIE  //
    // DMA1_Channel5->CCR |= DMA_CCR_EN;

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
    TIM2->CR1 = TIM_CR1_ARPE;
    //|TIM_CR1_CEN     // CEN Counter ENable
    //
    for (int ix = 0; ix < LED_IDLE_COUNT; ix++)
        led_buffer[ix] = 0;
    for (int ix = LED_IDLE_COUNT; ix < LED_BUFFER_SIZE; ix++)
        led_buffer[ix] = LED_L_CCR;
}

void stws281x_set(uint8_t n, uint8_t R, uint8_t G, uint8_t B) {
    uint8_t *ptr = &led_buffer[LED_IDLE_COUNT + n * 24];
    for (int ix = 0; ix < 8; ix++) {
        if (G & 0x80)
            *ptr++ = LED_H_CCR;
        else 
            *ptr++ = LED_L_CCR;
        G <<= 1;
    }
    for (int ix = 0; ix < 8; ix++) {
        if (R & 0x80)
            *ptr++ = LED_H_CCR;
        else 
            *ptr++ = LED_L_CCR;
        R <<= 1;
    }
    for (int ix = 0; ix < 8; ix++) {
        if (B & 0x80)
            *ptr++ = LED_H_CCR;
        else 
            *ptr++ = LED_L_CCR;
        B <<= 1;
    }
}

void stws281x_clear(uint8_t n) {
    uint8_t *ptr = &led_buffer[LED_IDLE_COUNT + n * 24];
    for (int i = 0; i < 24; i++) {
        *ptr++ = LED_L_CCR;
    }
}

void stws281x_write(void) {
    // Reload configuration
    DMA1_CSELR->CSELR |= 0b1000 << DMA_CSELR_C5S_Pos;
    // Periph is 32bit, Source is 8 bit
    DMA1_Channel5->CCR |= DMA_CCR_PSIZE_1 //| DMA_CCR_MSIZE_1 // WORD (32 bits)
                          | DMA_CCR_MINC  // Increment memory addr
                          | DMA_CCR_DIR;  //
    DMA1_Channel5->CMAR = (uint32_t)led_buffer;
    DMA1_Channel5->CNDTR = LED_BUFFER_SIZE;
    DMA1_Channel5->CPAR = (uint32_t)(&TIM2->CCR1);

    // Enable
    TIM2->CR1 |= TIM_CR1_CEN; // CEN Counter ENable
    DMA1_Channel5->CCR |= DMA_CCR_EN;

    // Wait till finished
    while ((DMA1->ISR & (DMA_ISR_TCIF5 | DMA_ISR_TEIF5)) == 0);

    // Disable TIMER  + DMA
    TIM2->CR1 &= ~TIM_CR1_CEN; 
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;
    // Clear flags
    DMA1->IFCR = DMA_IFCR_CTEIF5 | DMA_IFCR_CTCIF5;
}
