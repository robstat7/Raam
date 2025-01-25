/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel's main function.
 */
#include <raam/main.h>
#include <raam/tty.h>
#include <raam/acpi.h>
#include <raam/pcie.h>
#include <raam/nvme.h>
#include <raam/timer.h>
#include <raam/pic.h>
#include <asm/system.h>

void main(struct boot_params boot_params)
{
	/* interrupts are still disabled. */ 

	tty_init(boot_params.fb_info);

	int ret = acpi_init(boot_params.xsdp);
	if(ret == -1) {		// error
		goto end;
	}

	pcie_init();

	if(nvme_init(boot_params.system_variables) == -1) {
		goto end;
	}

	timer_init();

	pic_init();

	/* enable interrupts */
	sti();

	// asm volatile ("int $0x4");

end:
	return;
}
