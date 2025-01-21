#ifndef PORT_IO_H
#define PORT_IO_H

#include <stdint.h>

void outportb(uint16_t port_id, uint8_t value);
uint8_t inportb(uint16_t port_id);

#endif	/* PORT_IO_H */
