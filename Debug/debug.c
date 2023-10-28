#include "debug.h"

#define DEBUG_DATA0_ADDRESS  ((volatile uint32_t*)0xE00000F4)
#define DEBUG_DATA1_ADDRESS  ((volatile uint32_t*)0xE00000F8)

#ifdef MILLIS
volatile uint32_t __ms = 0;
#endif
#ifdef MICROS
volatile uint32_t __us = 0;
#endif

void init() {
    *DEBUG_DATA0_ADDRESS = 0;

#ifdef MILLIS
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->SR &= ~(1 << 0);
    SysTick->CMP = SystemCoreClock / 1000 - 1;
    SysTick->CNT = 0;
    SysTick->CTLR = 0xF;
#endif

#ifdef MICROS
    RCC->APB1PCENR |= RCC_TIM2EN;

    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CTLR1 = 0; // TIM_ARPE
    TIM2->PSC = SystemCoreClock / 1000000 - 1; // 1 MHz
    TIM2->CNT = 0;
//    TIM2->ATRLR = 0xFFFF;
    TIM2->SWEVGR = TIM_UG;
    TIM2->INTFR = 0; // ~TIM_UIF
    TIM2->DMAINTENR |= TIM_UIE;
    TIM2->CTLR1 |= TIM_CEN;
#endif
}

#ifdef MILLIS
void delay(uint32_t ms) {
    ms += __ms;
    while ((int32_t)__ms < (int32_t)ms) {
        __WFI();
    }
}
#endif

#ifdef MICROS
uint32_t micros() {
    return __us | TIM2->CNT;
}

void delayMicroseconds(uint32_t us) {
    us += __us;
    while ((int32_t)(__us | TIM2->CNT) < (int32_t)us) {}
}
#endif

#ifdef MILLIS
__attribute__((interrupt("WCH-Interrupt-fast"))) void SysTick_Handler(void) {
    ++__ms;
    SysTick->SR = 0;
}
#endif

#ifdef MICROS
__attribute__((interrupt("WCH-Interrupt-fast"))) void TIM2_IRQHandler(void) {
    if (TIM2->INTFR & TIM_UIF) {
        __us += 0x10000;
        TIM2->INTFR &= ~TIM_UIF;
    }
}
#endif

/*********************************************************************
 * @fn      _write
 *
 * @brief   Support Printf Function
 *
 * @param   *buf - UART send Data.
 *          size - Data length.
 *
 * @return  size - Data length
 */
__attribute__((used)) int _write(int fd, char *buf, int size) {
    int writeSize = size;

    do {
        /**
         * data0  data1 8 bytes
         * data0 The lowest byte storage length, the maximum is 7
         *
         */
        while (*DEBUG_DATA0_ADDRESS) {}

        *DEBUG_DATA1_ADDRESS = (*(buf + 3)) | (*(buf + 4) << 8) | (*(buf + 5) << 16) | (*(buf + 6) << 24);
        *DEBUG_DATA0_ADDRESS = ((writeSize > 7) ? 7 : writeSize) | (*buf << 8) | (*(buf + 1) << 16) | (*(buf + 2) << 24);

        if (writeSize > 7) {
            writeSize -= 7;
            buf += 7;
        } else
            writeSize = 0;
    } while (writeSize);
    return size;
}

/*********************************************************************
 * @fn      _sbrk
 *
 * @brief   Change the spatial position of data segment.
 *
 * @return  size: Data length
 */
__attribute__((used)) void *_sbrk(ptrdiff_t incr) {
    extern char _end[];
    extern char _heap_end[];
    static char *curbrk = _end;

    if ((curbrk + incr < _end) || (curbrk + incr > _heap_end))
        return NULL - 1;

    curbrk += incr;
    return curbrk - incr;
}
