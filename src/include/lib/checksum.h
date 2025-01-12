#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdint.h>

uint8_t validate_checksum(const uint8_t *table, uint32_t length);

#endif	// CHECKSUM_H
