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

struct process_control_block_struct process_1;
struct process_control_block_struct process_2;
struct process_control_block_struct *current_process;

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


	/* initialize two processes */
	process_1.pid = 1;
	process_1.state = RUNNING;
	
	process_2.pid = 2;
	process_2.state = READY;

	/* start with process 1 */
	current_process = &process_1;
	printk("@starting with process {d}  ", current_process->pid);

	/* simulate switching to process 2 */
	switch_process(&process_2);

	
end:
	return;
}

void switch_process(struct process_control_block_struct *next_process)
{
	current_process->state = READY;
	next_process->state = RUNNING;
	current_process = next_process;
	printk("@switched to process {d}  ", current_process->pid);	
}

void terminate_process(void)
{
	current_process->state = TERMINATED;
	printk("@process {d} has terminated  ", current_process->pid);
}
