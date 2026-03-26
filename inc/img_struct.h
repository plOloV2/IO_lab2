#ifndef IMG_STRUCT_H
#define IMG_STRUCT_H

#include"libs.h"

typedef enum{
    TYPE_P3,
    TYPE_P6
} PPM_TYPE;

typedef struct{
    uint16_t R;
    uint16_t G;
    uint16_t B;
} Pixel_data;

typedef struct{
    uint16_t    max_pixel_val;
    PPM_TYPE    img_type;
    size_t      width;
    size_t      height;
    size_t      pixel_num;
    char*       name;
    Pixel_data* pixels;
} Image_data;

#endif