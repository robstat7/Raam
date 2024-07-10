/*
 * Terminal driver.
 */
#include "fb.h"
#include "fonts.h"
#include <stdint.h>

/* terminal output coordinates */
int tty_x;
int tty_y;

uint32_t tty_fgcolor = 0x000000; /* tty text color = black */
uint32_t tty_bgcolor = 0xffffff; /* white */

const int tty_page_x_coord = 0; /* tty visible page x coord */
const int tty_page_y_coord = 0; /* tty visible page y coord */
const int tty_page_width = 639;
const int line_separator_space = 2; /* in pixels */

const int font_height = 8; /* in pixels */

struct frame_buffer_descriptor frame_buffer;

static inline void write_pixel(uint32_t pixel, int x, int y)
{
        *((uint32_t*)(frame_buffer.frame_buffer_base + frame_buffer.pixels_per_scan_line * y + x)) = pixel;
}
	 
void write_char(unsigned char c)
{
	int cx,cy;
	int mask[8]={128, 64, 32, 16, 8, 4, 2, 1};
	int offset;

	offset = (int) c;
	offset = offset * 8;

	if((int) c == 10) {	 /* LF */
		tty_x = tty_page_x_coord;
		tty_y += font_height + line_separator_space;
		return;
	}
	

	for(cy=0;cy<8;cy++){
		for(cx=0;cx<8;cx++){
			write_pixel(console_font_8x8[offset+cy]&mask[cx] ? tty_fgcolor : tty_bgcolor, tty_x + cx, tty_y + cy);
		}
	}

	/* update tty output coords */
	if((tty_x + cx) >= tty_page_width) { // 615 old
		tty_x = tty_page_x_coord;
		tty_y += cy + line_separator_space;
	} else {
		tty_x = tty_x + cx;
	}
}	

void tty_out_init(struct frame_buffer_descriptor fb) {
	frame_buffer = fb;

	/* init terminal output coordinates */
	tty_x = tty_page_x_coord;
	tty_y = tty_page_y_coord;
/* fill terminal background color with white */
	fill_tty_bgcolor();

	/* draw rectangular borders */
	draw_borders();
}

void fill_tty_bgcolor()
{
	unsigned int pixels = frame_buffer.horizontal_resolution * frame_buffer.vertical_resolution;
	uint32_t* addr = frame_buffer.frame_buffer_base;

	while (pixels--) {
		*addr++ = tty_bgcolor;
	}
}

void put_red_pixel(void)
{
	// unsigned int pixels = frame_buffer.horizontal_resolution * frame_buffer.vertical_resolution;
	volatile uint32_t* addr = frame_buffer.frame_buffer_base;

	// *addr = 0xff0000;
	*(volatile uint32_t *) 0x4000000000 = 0xff0000;

	// printk("@addr = {p}\n", (void *) addr);

	// while (pixels--) {
	// 	*addr++ = bgcolor;
	// }
}

void draw_borders(void)
{
	int rectangle_width = 27;	/* rectanglular border width */
	int horizontal_height = frame_buffer.horizontal_resolution;
	int vertical_height = frame_buffer.vertical_resolution;

	int fill_color = 0xFF0000;	/* red */

	draw_top_border(fill_color, horizontal_height, rectangle_width);
}

void draw_top_border(int fill_color, int horizontal_height, int width)
{
	volatile uint32_t* addr = frame_buffer.frame_buffer_base;

	for (int i = 0; i < width; i++) {
		for(int j = 0; j < horizontal_height; j++) {
			*addr++ = fill_color;
		}
	}
}
