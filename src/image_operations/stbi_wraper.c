#include "libs.h"
#include "tui.h"
#include "img_struct.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Deklaracja funkcji pomocniczej
Image_data* create_empty(uint16_t max_pixel_val, PPM_TYPE img_type, size_t width, size_t height, char* name);

// Wczytanie z pliku za pomocą biblioteki stb_image
Image_data* load_image_stb(char* path){

    int width, height, channels;
    
    unsigned char *stb_data = stbi_load(path, &width, &height, &channels, 3);
    if(!stb_data){
        print_error("stb_image failed to load the image. File may not exist or is unsupported.");
        return NULL;
    }

    Image_data* img = create_empty(255, TYPE_P6, (size_t)width, (size_t)height, path);
    if(!img){
        stbi_image_free(stb_data);
        return NULL;
    }

    #pragma omp parallel for
    for (size_t i = 0; i < img->pixel_num; i++) {
        img->pixels[i].R = stb_data[i * 3 + 0];
        img->pixels[i].G = stb_data[i * 3 + 1];
        img->pixels[i].B = stb_data[i * 3 + 2];
    }

    stbi_image_free(stb_data);
    
    return img;
}

// Zapisanie obrazu za pomocą biblioteki stb_image
void save_image_stb(Image_data* img){

    if(!img || !img->name){
        print_error("Podano niepoprawna strukture Image_data albo brakuje w niej sciezki (nazwa pliku)");
        return;
    }

    uint8_t *raw_pixels = malloc(img->pixel_num * 3);
    if(!raw_pixels){
        print_error("Nieudalo sie zaalokowac tablicy pikseli dla stb_image");
        return;
    }

    // Skalowanie pikseli do max_val = 255
    #pragma omp parallel for
    for(size_t i = 0; i < img->pixel_num; i++){
        raw_pixels[i * 3 + 0] = (uint8_t)(((uint32_t)img->pixels[i].R * 255) / img->max_pixel_val);
        raw_pixels[i * 3 + 1] = (uint8_t)(((uint32_t)img->pixels[i].G * 255) / img->max_pixel_val);
        raw_pixels[i * 3 + 2] = (uint8_t)(((uint32_t)img->pixels[i].B * 255) / img->max_pixel_val);
    }

    // Wyznaczenie rozszerzenia pliku i wykorzystanie odpowiedniego zapisu
    const char *ext = strrchr(img->name, '.');
    int success = 0;
    
    if(ext){
        if(strcmp(ext, ".png") == 0){

            success = stbi_write_png(img->name, img->width, img->height, 3, raw_pixels, img->width * 3);

        }else if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0){

            // Wykorzystuje ustawienie jakości = 90
            success = stbi_write_jpg(img->name, img->width, img->height, 3, raw_pixels, 90);


        }else if(strcmp(ext, ".bmp") == 0){

            success = stbi_write_bmp(img->name, img->width, img->height, 3, raw_pixels);

        }else if(strcmp(ext, ".tga") == 0){

            success = stbi_write_tga(img->name, img->width, img->height, 3, raw_pixels);

        }else {
            print_error("Unsupported file extension. Please use .png, .jpg, .bmp, or .tga");
        }
    }else {
        print_error("No file extension found in the output path.");
    }

    if(!success)
        print_error("stb_image_write failed to save the file.");

    free(raw_pixels);

}
