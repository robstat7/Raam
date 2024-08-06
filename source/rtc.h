#include "port_io.h"

#define NMI_disable_bit 1
#define DAY_OF_MONTH 0x07
#define MONTH 0x08
#define HOUR 0x04
#define MINUTE 0x02

uint8_t get_day_of_month();
uint8_t get_month();
uint8_t get_hour();
uint8_t get_minute();
