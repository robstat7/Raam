#include "port_io.h"
#include <stdint.h>
#include "timer.h"

int init_ps2_controller(void);
int send_bytes_to_dev(uint16_t port_id, uint8_t value);
void setup_timer(void);
