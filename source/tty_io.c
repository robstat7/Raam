/*
 * Terminal driver.
 */
#include "fb.h"
#include "fonts.h"
#include <stdint.h>
#include <stdlib.h>
#include "string.h"
#include "rtc.h"

/* terminal output coordinates */
int tty_x;
int tty_y;
int tty_border_x;
int tty_border_y;

const int font_height = 8; /* in pixels */
const int font_width = 8; /* in pixels */

uint32_t tty_fgcolor = 0x000000; /* tty text color = black */
uint32_t tty_bgcolor = 0xffffff; /* white */

const int tty_border_x_initial = 2; // in pixels
// const int tty_border_y_initial = 5; /* in pixels */
const int tty_border_y_initial = 9; /* in pixels */
// const int tty_page_x_coord = 0; /* tty visible page x coord */
const int tty_page_x_coord = (2 * tty_border_x_initial) + (2 * font_width) + 5; /* tty visible page x coord */
// const int tty_page_y_coord = 0; /* tty visible page y coord */
const int tty_page_y_coord = ( 2 * tty_border_y_initial) + font_height + 5; /* tty visible page y coord */
// const int tty_page_width = 639;
const int tty_page_width = 610; /* previously tested value = 600 */
const int line_separator_space = 2; /* in pixels */
// const int raam_name_separator_space = 20; /* in pixels */
// const int raam_name_separator_space = 25; /* in pixels */
const int raam_name_separator_space = 19; /* in pixels */
// const int rectangle_width = 27;	/* rectanglular border width in pixels */
const int rectangle_width = 40;		/* rectanglular border width in pixels */


struct frame_buffer_descriptor frame_buffer;

void write_top_border_char(unsigned char c);

static inline void write_pixel(uint32_t pixel, int x, int y)
{
        *((volatile uint32_t*)(frame_buffer.frame_buffer_base + frame_buffer.pixels_per_scan_line * y + x)) = pixel;
}
	 
void write_char(unsigned char c)
{
	int cx,cy;
	int mask[8]={128, 64, 32, 16, 8, 4, 2, 1};
	int offset;
	
	if((int)c == 10) {	 /* LF */
		tty_x = tty_page_x_coord;
		tty_y += font_height + line_separator_space;
		return;
	}

	else if(c == 0x8) {	/* backspace */
		if(tty_x == tty_page_x_coord) {
			tty_y -= line_separator_space - font_height;
			tty_x = tty_page_width - font_width;
		}

		else
			tty_x -= font_width;

		c = ' ';

		write_char(c);
		
		if(tty_x == tty_page_x_coord) {
			tty_y -= line_separator_space - font_height;
			tty_x = tty_page_width - font_width;
		}

		else
			tty_x -= font_width;

		return;
	}


	offset = (int) c;
	offset = offset * 8;

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

void write_hindi_char(unsigned char c)
{
	int cx,cy;
	int mask[8]={128, 64, 32, 16, 8, 4, 2, 1};
	int offset;

	offset = (int) c;
	offset = offset * 8;

	// if((int) c == 10) {	 /* LF */
	// 	tty_border_x = tty_page_x_coord;
	// 	tty_border_y += font_height + line_separator_space;
	// 	return;
	// }
	

	for(cy=0;cy<8;cy++){
		for(cx=0;cx<8;cx++){
			write_pixel(hindi_console_font_8x8[offset+cy]&mask[cx] ? 0xffffff : 0xe5322c, tty_border_x + cx, tty_border_y + cy); // bgcolor = red, fgcolor = white
		}
	}

	/* update tty border x coordinate */
	tty_border_x = tty_border_x + cx;
}

void write_top_border_char(unsigned char c)
{
	int cx,cy;
	int mask[8]={128, 64, 32, 16, 8, 4, 2, 1};
	int offset;

	offset = (int) c;
	offset = offset * 8;

	// if((int) c == 10) {	 /* LF */
	// 	tty_border_x = tty_page_x_coord;
	// 	tty_border_y += font_height + line_separator_space;
	// 	return;
	// }
	

	for(cy=0;cy<8;cy++){
		for(cx=0;cx<8;cx++){
			write_pixel(console_font_8x8[offset+cy]&mask[cx] ? 0xffffff : 0xe5322c, tty_border_x + cx, tty_border_y + cy); // bgcolor = red, fgcolor = white
		}
	}

	/* update tty border x coordinate */
	tty_border_x = tty_border_x + cx;
}


void tty_out_init(struct frame_buffer_descriptor fb) {
	frame_buffer = fb;

	// init tty border text coordinates
	tty_border_x = tty_border_x_initial;	
	tty_border_y = tty_border_y_initial;

	/* init terminal output coordinates */
	tty_x = tty_page_x_coord;
	tty_y = tty_page_y_coord;
	
	/* fill terminal background color with white */
	fill_tty_bgcolor();

	/* draw rectangular borders */
	draw_borders();

	/* draw RAAM names */
	draw_raam_names();

	// write_chant_raam_name_msg_at_top();
	
	tty_border_x = 30;
	tty_border_y = tty_border_y_initial - 4;
	
	char msg[43] = "|| Chant Raam name. Wake up your destiny ||";

	write_top_border_eng_text(&msg, 43);


	/* display today's date */
	
	char d = get_day_of_month() + '0';
	char m = get_month() + '0';
	char by = '/';
	char sep[3] = "   ";

	write_top_border_eng_text(&sep, 3);
	write_top_border_eng_text(&d, 1);
	write_top_border_eng_text(&by, 1);
	write_top_border_eng_text(&m, 1);

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
	int horizontal_height = frame_buffer.horizontal_resolution;
	int vertical_height = frame_buffer.vertical_resolution;

	int fill_color = 0xe5322c;	/* red */

	draw_top_border(fill_color, horizontal_height, rectangle_width);

	draw_bottom_border(fill_color, horizontal_height, rectangle_width);

	draw_left_border(fill_color, vertical_height, horizontal_height, rectangle_width);

	draw_right_border(fill_color, vertical_height, horizontal_height, rectangle_width);
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

void draw_bottom_border(int fill_color, int horizontal_height, int width)
{
	int total_pixels = frame_buffer.horizontal_resolution * frame_buffer.vertical_resolution;
	int total_bottom_rect_pixels = horizontal_height * width;
	volatile uint32_t* addr = (uint32_t *) frame_buffer.frame_buffer_base + (total_pixels - total_bottom_rect_pixels);

	for (int i = 0; i < width; i++) {
		for(int j = 0; j < horizontal_height; j++) {
			*addr++ = fill_color;
		}
	}
}

void draw_left_border(int fill_color, int vertical_height, int horizontal_height, int width)
{
	volatile uint32_t* addr = frame_buffer.frame_buffer_base;

	for(int i = 0; i < vertical_height; i++) {
		for(int j = 0; j < width; j++) {
			*addr++ = fill_color;
		}

		addr = (uint32_t *) frame_buffer.frame_buffer_base + ((i+1) * horizontal_height);
	}
}


void draw_right_border(int fill_color, int vertical_height, int horizontal_height, int width)
{
	volatile uint32_t* addr = frame_buffer.frame_buffer_base;

	for(int i = 0; i < vertical_height; i++) {
		addr = (uint32_t *) frame_buffer.frame_buffer_base + (((i + 1) * horizontal_height) - width);

		for(int j = 0; j < width; j++) {
			*addr++ = fill_color;
		}
	}
}

void draw_raam_names(void)
{
	draw_raam_name_in_left_border();

	// printk("hello");

	draw_raam_name_in_bottom_border();

	draw_raam_name_in_right_border();
}

void draw_first_raam_name(void)
{
	int horizontal_height = frame_buffer.horizontal_resolution;
	volatile uint32_t* addr = (uint32_t *) frame_buffer.frame_buffer_base + (5 * horizontal_height) + 5; // x offset = (5 + horizontal_height), y offset = + 5
	
	// // top straight line
	// for (int i = 0; i < 10; i++)
	// 	write_pixel(0xffffff, horizontal_height + 5 + i, 5);

	// // R of RAAM
	// राम

	for(int i = 0; i < 24; i++) {	
		write_hindi_char(0);	
		write_hindi_char(1);	
		write_hindi_char(2);	
	}
}

void draw_raam_name_in_left_border(void)
{
	// int x_offset = 5;	// in pixels
	// int y_offset = 3;

	// for(int i = 0; i < 25; i++) {
	for(int i = 0; i < 19; i++) {
		draw_raam_name();

		/* update tty output coords */
		tty_border_x = tty_border_x_initial;
		tty_border_y += font_height + raam_name_separator_space;
	}

}

void draw_raam_name(void)
{
	// int horizontal_height = frame_buffer.horizontal_resolution;
	// volatile uint32_t* addr = (uint32_t *) frame_buffer.frame_buffer_base + (5 * horizontal_height) + 5; // x offset = (5 + horizontal_height), y offset = + 5
	
	write_hindi_char(0);	
	write_hindi_char(1);	
}

void draw_raam_name_in_bottom_border(void)
{
	// tty_border_y = frame_buffer.vertical_resolution - 1 - tty_border_y_initial - font_height;
	// tty_border_y = frame_buffer.vertical_resolution - font_height;
	tty_border_x = tty_border_x_initial + font_width + raam_name_separator_space;
	tty_border_y = 500;
	// tty_border_y = 100;
	// tty_border_y = frame_buffer.vertical_resolution - 1000;

	for(int i = 0; i < 14; i++) {
		draw_raam_name();
		write_hindi_char(2);

		/* update tty output coords */
		tty_border_x += raam_name_separator_space;
	}
	
}

void draw_raam_name_in_right_border(void)
{
	tty_border_x = frame_buffer.horizontal_resolution - (2 * font_width) - tty_border_x_initial;
	tty_border_y = tty_border_y_initial;

	// for(int i = 0; i < 25; i++) {
	for(int i = 0; i < 19; i++) {
		draw_raam_name();

		/* update tty output coords */
		tty_border_x = frame_buffer.horizontal_resolution - (2 * font_width) - tty_border_x_initial;
		tty_border_y += font_height + raam_name_separator_space;
	}

}

void write_top_border_eng_text(char *msg, int msg_len)
{

	for(int i = 0; i < msg_len; i++) {
		write_top_border_char(msg[i]);
	}
}


void write_chant_raam_name_msg_at_top(void)
{
	// tty_border_x = (2 * tty_border_x_initial) + (2 * font_width) + 10;
	// tty_border_y = tty_border_y_initial;
	tty_border_x = 30;
	// tty_border_y = 3;
	tty_border_y = 0;

	char *msg = "|| Chant Raam name. Wake up your destiny ||";

	int msg_len = strlen(msg);

	// write_top_border_eng_text(msg, msg_len);
	// write_top_border_char('R');
	// write_hindi_char(0);
	draw_raam_name();
	// printk("Hello World RAAM JI!");
	// write_char('A');
	// write_char('B');

	// Explicit memory barrier for use with GCC.
	// asm volatile ("": : :"memory");
	
	// fill_tty_bgcolor();

}


