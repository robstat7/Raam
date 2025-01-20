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
	sys_creat("myfile");
	int inode_nr = sys_open("myfile");
	sys_write(inode_nr, "Raam Raam sa! Hello world!", 27); /* len including null char */
	
	char buf[50];
	
	sys_read(inode_nr, buf, 27);

	printk(buf);

	printk(" ");

	printk("@making second file now...  ");

	sys_creat("atom");
	inode_nr = sys_open("atom");
	sys_write(inode_nr, "#include <iostream> using namespace std;", 41); /* len including null char */
	sys_read(inode_nr, buf, 41);

	printk(buf);

	printk(" ");

	printk("@creating third file now...  ");

	sys_creat("gimp");
	inode_nr = sys_open("gimp");
	sys_write(inode_nr, "no worries! all will be fine.", 30); /* len including null char */
	sys_read(inode_nr, buf, 30);

	printk(buf);

	printk(" ");


end:
	return;
}
