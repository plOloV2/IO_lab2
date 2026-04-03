#ifndef IMAGE_OPERATIONS_H
#define IMAGE_OPERATIONS_H

#include "libs.h"
#include "img_struct.h"

Image_data* create_empty(uint16_t max_pixel_val, PPM_TYPE img_type, size_t width, size_t height, char* name);

Image_data* load_image(char* path);

Image_data* load_image_stb(char* path);

void save_image_ppm(Image_data* img);

void save_image_png(Image_data* img);

void save_image_stb(Image_data* img);

Image_data* copy_image(Image_data* img, char* name);

void free_image(Image_data* img);

void convert_P3_P6(Image_data* img);

void convert_P6_P3(Image_data* img);

void change_pixel_max_val(Image_data* img, uint16_t new_value);

void process_jpeg_pipeline(Image_data* input, int subsample_factor);

#endif
