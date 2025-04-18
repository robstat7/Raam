/*
 * Programmable Interval Timer (PIT) driver
 */
#include <raam/timer.h>
#include <raam/port_io.h>
#include <raam/isr.h>

#define LATCH			(INPUT_CLOCK_FREQUENCY/HZ)

void timer_init(void)
{
	set_frequency();

	/* register interrupt handler for the timer interrupt */                
        register_interrupt_handler(32, timer_handler);
}

/* irq0 will fire 100 times per second */
static void set_frequency(void)
{
	/* init. Square wave mode (mode 3), 16-bit binary, */
	/* first transfer LSB then MSB */
	outportb(TIMER_COMMAND, TIMER_ICW);
	outportb(TIMER_DATA, LATCH & 0xff);    /* LSB */
        outportb(TIMER_DATA, LATCH >> 8);        /* MSB */
}

