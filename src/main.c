
#include "main.h"

#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>
#include <stdlib.h>

#define ENCODER_A 2
#define ENCODER_B 3
#define TICK_DIVISION 4

volatile int64_t ticks = 0;
volatile uint lastState = 0;

int main() {
	stdio_init_all();
	
	gpio_set_dir(ENCODER_A, false);
	gpio_set_dir(ENCODER_B, false);
	gpio_pull_up(ENCODER_A);
	gpio_pull_up(ENCODER_B);
	sleep_ms(1);
	lastState = readState();
	
	gpio_set_irq_enabled_with_callback(
		ENCODER_A,
		GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
		true,
		decoderISR
	);
	gpio_set_irq_enabled_with_callback(
		ENCODER_B,
		GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
		true,
		decoderISR
	);
	int64_t tickReport = 0;
	
	while (1) {
		int64_t ticks0 = ticks / TICK_DIVISION;
		if (tickReport != ticks0) {
			printf("%lld ticks              \r", ticks0);
			tickReport = ticks0;
		}
	}
}

void decoderISR(uint gpio, uint32_t event_mask) {
	uint newState = readState();
	
	if (newState == (lastState + 1) % 4) ticks++;
	if (newState == (lastState - 1) % 4) ticks--;
	
	lastState = newState;
}

uint readState() {
	// Read GPIOs at the same time.
	uint32_t raw = gpio_get_all();
	bool     a   = raw & (1u << ENCODER_A);
	bool     b   = raw & (1u << ENCODER_B);
	
	// A simple LUT.
	uint8_t lut[] = {0, 1, 3, 2};
	return lut[a+(b<<1)];
}
