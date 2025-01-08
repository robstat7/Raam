/*
 * terminal I/O related functions.
 */
#include <raam/tty.h>

struct tty current_tty;

/*
 * tty_init
 *
 * Initialize the tty structure.
 */
void tty_init(struct fb_info fb_info) 
{
	current_tty.fb_base = fb_info.fb_base;	
	current_tty.horizontal_resolution = fb_info.horizontal_resolution;
	current_tty.vertical_resolution = fb_info.vertical_resolution;
	current_tty.pixels_per_scanline = fb_info.pixels_per_scanline;
	current_tty.cursor_x = 0;
	current_tty.cursor_y = 0;
}
