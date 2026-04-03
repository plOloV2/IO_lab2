#include "libs.h"
#include "tui.h"
#include "img_struct.h"

// Deklaracja funkcji pomicniczej 
Image_data* create_empty(uint16_t max_pixel_val, PPM_TYPE img_type, size_t width, size_t height, char* name);

// Utworzenie głębokiej kopii obrazka
Image_data* copy_image(Image_data* img, char* name){

    if(!img){
        print_error("Image_data* in NULL (but why?)");    
        return NULL;
    }

    if(!name){
        print_error("No name provided for cloned image");
        return NULL;
    }

    Image_data* new_img = create_empty(img->max_pixel_val, img->img_type, img->width, img->height, name);

    #pragma omp parallel for
    for(size_t i = 0; i < img->pixel_num; i++)
        new_img->pixels[i] = img->pixels[i];

    return new_img;

}

// Zwolnienie obrazka z pamięci
void free_image(Image_data* img){

    if(!img){
        print_error("Image already free");
        return;
    }

    if(img->name)
        free(img->name);
    
    if(img->pixels)
        free(img->pixels);
    
    free(img);

    img = NULL;

}

// Zmiana typu kodowania .ppm na P6
void convert_P3_P6(Image_data* img){

    if(img)
        img->img_type = TYPE_P6;
    
}

// Zmiana typu kodowania .ppm na P6
void convert_P6_P3(Image_data* img){

    if(img)
        img->img_type = TYPE_P3;

}

// Zmiana maksymalnej wartości pikseli i przeskalowanie obecnych pikseli do niej
void change_pixel_max_val(Image_data* img, uint16_t new_value){

    if(new_value == img->max_pixel_val)
        return;

    if(new_value == 0){
        print_error("Max_pixel_value cannot be 0");
        return;
    }

    #pragma omp parallel for
    for(size_t i = 0; i < img->pixel_num; i++){
        img->pixels[i].R = ((uint32_t)img->pixels[i].R * new_value) / img->max_pixel_val;
        img->pixels[i].G = ((uint32_t)img->pixels[i].G * new_value) / img->max_pixel_val;
        img->pixels[i].B = ((uint32_t)img->pixels[i].B * new_value) / img->max_pixel_val;
    }
        
    img->max_pixel_val = new_value;

}
