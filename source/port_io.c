#include "port_io.h"

/* Output byte AL to immediate port number */
void outportb(uint16_t port_id, uint8_t value)
{
	asm volatile("out dx, al": :"d" (port_id), "a" (value));
}

/* Input byte from immediate port into AL */
uint8_t inportb(uint16_t portid)
{
	uint8_t ret;
	asm volatile("in al, dx":"=a"(ret):"d"(portid));
	return ret;
}
