/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Kernel's main function.
 */
#include <raam/main.h>
#include <raam/tty.h>

void main(struct boot_params boot_params)
{
	tty_init(boot_params.fb_info);

	// test
	tty_put_char('A');
	tty_put_char('b');
	tty_put_char('Q');
	tty_put_char('R');
}
