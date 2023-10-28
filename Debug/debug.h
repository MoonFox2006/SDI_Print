#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#include <ch32v00x.h>
#include <stdio.h>

#define MILLIS
#define MICROS

#ifdef MILLIS
extern volatile uint32_t __ms;

#define millis()    __ms
#endif

void init();
#ifdef MICROS
uint32_t micros();
void delayMicroseconds(uint32_t us);
#endif
#ifdef MILLIS
void delay(uint32_t ms);
#endif

#ifdef __cplusplus
}
#endif
