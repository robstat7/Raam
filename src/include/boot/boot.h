#ifndef BOOT_H
#define BOOT_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS exit_boot_services(void);
EFI_STATUS get_memory_map(void);
EFI_STATUS get_memory_map_size(void);

#endif	// BOOT_H
