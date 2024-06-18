/* structure to hold frame buffer information */
struct frame_buffer_descriptor {
	long unsigned int *frame_buffer_base;
	unsigned int frame_buffer_size;
	unsigned short horizontal_resolution;
	unsigned short vertical_resolution;
	unsigned short pixels_per_scan_line;
};
