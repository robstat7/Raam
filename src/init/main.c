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
#include <raam/fs.h>
#include <raam/printk.h>

void main(struct boot_params boot_params)
{
	tty_init(boot_params.fb_info);

	int ret = acpi_init(boot_params.xsdp);
	if(ret == -1) {		// error
		goto end;
	}

	pcie_init();

	if(nvme_init(boot_params.system_variables) == -1) {
		goto end;
	}

	// test
	// sys_creat("myfile");
	sys_open("myfile");
	//printk("init/main.c: fd = {d}  ", fd);

end:
	return;
}
