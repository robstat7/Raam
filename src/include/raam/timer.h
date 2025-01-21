#ifndef TIMER_H
#define TIMER_H

#define INPUT_CLOCK_FREQUENCY		1193180
#define HZ				100

#define TIMER_COMMAND			0x43
#define TIMER_DATA			0x40
#define TIMER_ICW			0x36

void timer_init(void);
static void set_frequency(void);

#endif	/* TIMER_H */
