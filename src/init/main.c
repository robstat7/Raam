/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel's main function.
 */
#include <raam/main.h>
#include <raam/tty.h>
#include <raam/acpi.h>
#include <raam/pcie.h>

void main(struct boot_params boot_params)
{
	tty_init(boot_params.fb_info);
	int ret = acpi_init(boot_params.xsdp);
	if(ret == -1) {		// error
		goto end;
	}

	pcie_init();
end:
	return;
}
