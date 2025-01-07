#ifndef BOOT_H
#define BOOT_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS exit_boot_services(EFI_HANDLE image_handle);

#endif	// BOOT_H
