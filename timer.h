#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// PIT tick rate in Hz. It must be high enough to fit in a 16-bit divisor.
// anything above 100 Hz is fine
#define TIMER_FREQ 1000

// Initializes the PIT to a specific frequency
void timer_init(uint32_t frequency);

// The callback function called by the assembly stub
void timer_handler();

// Returns the number of ticks since boot. mainly not used directly
uint32_t timer_get_ticks();

// sleeps for x ticks. if you want to do based on one sec you can do:
// sleep(TIMER_FREQ) for one sec
void sleep(int);

#endif
