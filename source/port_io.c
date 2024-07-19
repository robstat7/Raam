#include "port_io.h"

/* Output byte AL to immediate port number */
void outportb(uint16_t port_id, uint8_t value)
{
	asm volatile("out dx, al": :"d" (port_id), "a" (value));
}

/* Input byte from port DX into AL */
uint8_t inportb(uint16_t portid)
{
	uint8_t ret;
	asm volatile("in al, dx":"=a"(ret):"d"(portid));
	return ret;
}

/* Input word from port DX into AX */
uint16_t inportw(uint16_t portid)
{
	uint16_t ret;
	asm volatile("in ax, dx":"=a"(ret):"d"(portid));
	return ret;
}
