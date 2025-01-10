/*
 * 'tty.h' defines the structure used by tty_io.c.
 */
#ifndef TTY_H
#define TTY_H

#include <boot/fb.h>
#include <stdint.h>

struct tty {
	volatile uint32_t *fb_base;	// framebuffer base address pointer

	// horizontal and vertical screen resolutions
	unsigned long horizontal_resolution;
	unsigned long vertical_resolution;

	unsigned long pixels_per_scanline;

	unsigned long cursor_x;		// current x-coordinate of the cursor
	unsigned long cursor_y;		// current y-coordinate of the cursor

	// tty foreground color
	uint32_t fg_color;
};

void tty_init(struct fb_info_struct fb_info);
void tty_put_char(char c);
static inline void tty_put_pixel(uint32_t pixel_color, int x, int y);

#endif	// TTY_H
