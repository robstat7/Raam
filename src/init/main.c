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
#include <raam/process.h>
#include <raam/printk.h>

struct process_control_block_struct current_process;

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


	/* initialize the first process */
	current_process.pid = 1;
	current_process.state = RUNNING;

	printk("@process {d} is running  ", current_process.pid);

	for(int i = 0; i < 5; i++) {
		printk("@process {d} is at step {d}  ", current_process.pid,
			i);
	}

	terminate_process();
	
end:
	return;
}

void terminate_process(void)
{
	current_process.state = TERMINATED;
	printk("@process {d} has terminated  ", current_process.pid);
}
