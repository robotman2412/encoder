
#include "main.h"

#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>
#include <stdlib.h>

#define ENCODER_A 2
#define ENCODER_B 3
#define ENCODER_Z 4
#define TICK_DIVISION 4

int64_t ticksPerRev = 20 * TICK_DIVISION;
volatile int64_t ticks = 0;
volatile uint lastState = 0;
volatile bool skipDetected = false;
volatile int64_t skipEstimate = 0;

int main() {
	// Initialise I/O.
	stdio_init_all();
	
	// Initialise input pins.
	gpio_set_dir(ENCODER_A, false);
	gpio_set_dir(ENCODER_B, false);
	gpio_set_dir(ENCODER_Z, false);
	gpio_pull_up(ENCODER_A);
	gpio_pull_up(ENCODER_B);
	gpio_pull_up(ENCODER_Z);
	// Small delay to allow stabilisation.
	sleep_ms(1);
	// Read initial state.
	lastState = readState();
	
	// Attach interrupt to encoder A.
	gpio_set_irq_enabled_with_callback(
		ENCODER_A,
		GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
		true,
		decoderISR
	);
	// Attach interrupt to encoder B.
	gpio_set_irq_enabled_with_callback(
		ENCODER_B,
		GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
		true,
		decoderISR
	);
	// Attach interrupt to encoder Z.
	gpio_set_irq_enabled_with_callback(
		ENCODER_Z,
		GPIO_IRQ_EDGE_FALL,
		true,
		decoderISR
	);
	
	// Simple loop for printening out the ticks.
	int64_t tickReport = 0;
	while (1) {
		int64_t ticks0 = ticks / TICK_DIVISION;
		if (tickReport != ticks0) {
			printf("\r                    \r%lld ticks", ticks0);
			tickReport = ticks0;
		}
		if (skipDetected) {
			printf("\nSkip detected (estimate: %lld)\n", skipEstimate / TICK_DIVISION);
			skipDetected = false;
		}
	}
}

uint readState() {
	// Read GPIOs at the same time.
	uint32_t raw = gpio_get_all();
	// Split out the A and B bits.
	bool a = raw & (1u << ENCODER_A);
	bool b = raw & (1u << ENCODER_B);
	
	// A simple LUT turns a (0, 1, 3, 2) loop into a (0, 1, 2, 3) loop.
	uint8_t lut[] = {0, 1, 3, 2};
	return lut[a+(b<<1)];
}

void decoderISR(uint gpio, uint32_t event_mask) {
	if (gpio == ENCODER_Z) {
		zeroISR();
	} else {
		// Get new position in cycle.
		uint newState = readState();
		
		// Detect direction.
		if (newState == (lastState + 2) % 4) {
			skipDetected = true;
			skipEstimate = 1;
		}
		if (newState == (lastState + 1) % 4) ticks++;
		if (newState == (lastState - 1) % 4) ticks--;
		
		// Update current cycle position.
		lastState = newState;
	}
}

void zeroISR() {
	// Check against tick count.
	if (ticks % ticksPerRev) {
		skipEstimate = ticks % ticksPerRev;
		skipDetected = true;
	}
	
	// Reset teh ticks lol.
	ticks = 0;
}
