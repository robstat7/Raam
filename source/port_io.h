#include <stdint.h>

void outportb(uint16_t port_id, uint8_t value);
uint8_t inportb(uint16_t portid);
uint16_t inportw(uint16_t portid);
