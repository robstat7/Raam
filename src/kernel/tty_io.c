/*
 * terminal I/O related functions.
 */
#include <raam/tty.h>

struct tty default_tty;

/*
 * tty_init
 *
 * Initialize the tty structure.
 */
void tty_init(struct fb_info fb_info) 
{
	default_tty.fb_base = fb_info.fb_base;
	default_tty.horizontal_resolution = fb_info.horizontal_resolution;
	default_tty.vertical_resolution = fb_info.vertical_resolution;
	default_tty.pixels_per_scanline = fb_info.pixels_per_scanline;
	default_tty.cursor_x = 0;
	default_tty.cursor_y = 0;
}
