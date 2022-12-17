
#pragma once

#include <pico/stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void decoderISR(uint gpio, uint32_t event_mask);
uint readState();
int main();

#ifdef __cplusplus
}
#endif
