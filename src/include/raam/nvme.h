#ifndef NVME_H
#define NVME_H

#include <stdint.h>

void nvme_init(void);
static void find_controller(void);
static void check_all_buses(void);
static int check_device(uint16_t bus, uint8_t dev);
static uint16_t get_vendor_id(uint32_t bus, uint32_t dev, uint32_t func);
static int search_for_controller(uint32_t bus, uint32_t dev, uint32_t func);
static uint64_t get_config_space_phy_mmio_addr(uint32_t bus, uint32_t dev,
					       uint32_t func);

#endif	// NVME_H
