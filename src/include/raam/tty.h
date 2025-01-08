#ifndef TTY_H
#define TTY_H

#include <boot/fb.h>

struct tty {
	unsigned long long fb_base;	// framebuffer base address

	// horizontal and vertical screen resolutions
	unsigned long horizontal_resolution;
	unsigned long vertical_resolution;

	unsigned long pixels_per_scanline;

	unsigned long cursor_x;		// current x-coordinate of the cursor
	unsigned long cursor_y;		// current y-coordinate of the cursor
};

void tty_init(struct fb_info fb_info);

#endif	// TTY_H
