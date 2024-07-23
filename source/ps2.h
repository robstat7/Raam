#include "port_io.h"
#include <stdint.h>
#include "timer.h"

int init_ps2_controller(uint64_t *xsdp);
int send_bytes_to_dev(uint16_t port_id, uint8_t value);
uint8_t read_controller_configuration_byte(void);
int is_dual_channel_controller(uint8_t cc_byte);
void setup_timer(void);
