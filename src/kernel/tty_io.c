/*
 * terminal subsystem I/O related functions.
 */
#include <raam/tty.h>
#include <raam/font_8x16.h>

struct tty default_tty;

// mask for each pixel in a row
const unsigned char mask[8] = {128, 64, 32, 16, 8, 4, 2, 1}; 

/*
 * tty_init
 *
 * This function initializes the tty structure.
 */
void tty_init(struct fb_info_struct fb_info) 
{
	default_tty.fb_base = (volatile uint32_t *) fb_info.fb_base;
	default_tty.horizontal_resolution = fb_info.horizontal_resolution;
	default_tty.vertical_resolution = fb_info.vertical_resolution;
	default_tty.pixels_per_scanline = fb_info.pixels_per_scanline;
	default_tty.cursor_x = 0;
	default_tty.cursor_y = 0;
	default_tty.fg_color = 0xffffff;	// white
}

void tty_put_char(char c)
{
	int offset = c * 16;	// 16 is the font height

	for(int row = 0; row < 16; row++) {	// 16 rows in a 8x16 font
		unsigned char row_data = fontdata_8x16[offset + row];
		for (int col = 0; col < 8; col++) {
			// check if we need to write a pixel in the row pixels data
			if(row_data & mask[col]) {	
				write_pixel(default_tty.fg_color, default_tty.cursor_x + col, default_tty.cursor_y + row);
			}
		}
	}

	// update tty cursors' positions 
	default_tty.cursor_x += 8;	// 8 is the font width	
	if(default_tty.cursor_x >= default_tty.horizontal_resolution) {
		default_tty.cursor_x = 0;	// wrap around to the next line
		default_tty.cursor_y += 16;	// move down by the font height
	}
}

static inline void write_pixel(uint32_t pixel_color, int x, int y)
{
	volatile uint32_t *pixel_address = (volatile uint32_t *)(default_tty.fb_base + default_tty.pixels_per_scanline * y + x);
	*pixel_address = pixel_color;
}
