#include "printk.h"
#include "string.h"
#include "msr_io.h"
#include "isr.h"

int timer_init(void);
void set_divide_value(int value);
int set_mode(uint32_t mode);
void set_initial_count(uint64_t count);
uint64_t read_current_count(void);
void stop_timer(void);
