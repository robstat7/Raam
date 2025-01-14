#ifndef NVME_H
#define NVME_H

#include <stdint.h>

void nvme_init(void);
static void find_controller(void);
static void check_all_buses(void);
static void check_device(uint16_t bus, uint8_t dev);
static uint16_t get_vendor_id(uint32_t bus, uint32_t dev, uint32_t func);

#endif	// NVME_H
