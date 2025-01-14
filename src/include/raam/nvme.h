#ifndef NVME_H
#define NVME_H

#include <stdint.h>

struct common_config_space_header_struct {
	uint16_t vendor_id;
	uint16_t dev_id;
	uint16_t cmd;
	uint16_t status;
	uint8_t revision_id;

	// class code fields
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class_code;

	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;
}__attribute__((packed));

void nvme_init(void);
static void find_controller(void);
static void check_all_buses(void);
static int check_device(uint16_t bus, uint8_t dev);
static uint16_t get_vendor_id(uint32_t bus, uint32_t dev, uint32_t func);
static int search_for_controller(uint32_t bus, uint32_t dev, uint32_t func);
static uint64_t get_config_space_phy_mmio_addr(uint32_t bus, uint32_t dev,
					       uint32_t func);

#endif	// NVME_H
