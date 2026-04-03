#include "libs.h"
#include "tui.h"
#include "img_struct.h"
#include <zlib.h>

void save_image_ppm(Image_data* img){

    if(!img || !img->name){
        print_error("Image or image.name is missing (null pointer)");
        return;
    }

    FILE *file = fopen(img->name, "wb");
    if(!file){
        print_error("Failed to create file");
        return;
    }

    fprintf(file, "%s\n", img->img_type == TYPE_P3 ? "P3" : "P6");
    fprintf(file, "%zu %zu\n", img->width, img->height);
    fprintf(file, "%u\n", img->max_pixel_val);

    if(img->img_type == TYPE_P3){
        for(size_t i = 0; i < img->pixel_num; i++){

            fprintf(file, "%u %u %u ", img->pixels[i].R, img->pixels[i].G, img->pixels[i].B);

            if((i + 1) % img->width == 0)
                fprintf(file, "\n");
            
        }

    }else {
        if(img->max_pixel_val <= 255){

            uint8_t *raw_pixels = malloc(img->pixel_num * 3);
            if(!raw_pixels){
                print_error("Raw_pixels array alloc failed");
                fclose(file);
                return;
            }
            
            #pragma omp parallel for
            for(size_t i = 0; i < img->pixel_num; i++){
                raw_pixels[i * 3]     = (uint8_t)img->pixels[i].R;
                raw_pixels[i * 3 + 1] = (uint8_t)img->pixels[i].G;
                raw_pixels[i * 3 + 2] = (uint8_t)img->pixels[i].B;
            }
            fwrite(raw_pixels, 1, img->pixel_num * 3, file);
            free(raw_pixels);

        }else {

            uint8_t *raw_pixels = malloc(img->pixel_num * 3 * 2);
            if(!raw_pixels){
                print_error("Raw_pixels array alloc failed");
                fclose(file);
                return;
            }
            
            #pragma omp parallel for
            for (size_t i = 0; i < img->pixel_num; i++){
                raw_pixels[i * 6]     = (uint8_t)(img->pixels[i].R >> 8);
                raw_pixels[i * 6 + 1] = (uint8_t)(img->pixels[i].R & 0xFF);
                
                raw_pixels[i * 6 + 2] = (uint8_t)(img->pixels[i].G >> 8);
                raw_pixels[i * 6 + 3] = (uint8_t)(img->pixels[i].G & 0xFF);
                
                raw_pixels[i * 6 + 4] = (uint8_t)(img->pixels[i].B >> 8);
                raw_pixels[i * 6 + 5] = (uint8_t)(img->pixels[i].B & 0xFF);
            }

            fwrite(raw_pixels, 1, img->pixel_num * 3 * 2, file);
            free(raw_pixels);

        }

    }

    fclose(file);

}

static void write_uint32(FILE *file, uint32_t val){

    uint8_t bytes[4] = {
        (val >> 24) & 0xFF,
        (val >> 16) & 0xFF,
        (val >> 8) & 0xFF,
        val & 0xFF
    };

    fwrite(bytes, 1, 4, file);

}

static void write_chunk(FILE *file, const char *type, const uint8_t *data, uint32_t len){

    write_uint32(file, len);
    
    fwrite(type, 1, 4, file);
    
    if(len > 0 && data != NULL)
        fwrite(data, 1, len, file);
    
    uint32_t crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const unsigned char*)type, 4);
    if(len > 0 && data != NULL)
        crc = crc32(crc, data, len);
    
    write_uint32(file, crc);

}

void save_image_png(Image_data* img){

    if(!img || !img->name){
        print_error("Invalid image or output path provided to save_image_png.");
        return;
    }

    FILE *file = fopen(img->name, "wb");
    if(!file){
        print_error("Failed to create file");
        return;
    }


    const uint8_t png_sig[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    fwrite(png_sig, sizeof(uint8_t), 8, file);

    uint8_t ihdr_data[13];
    ihdr_data[0] = (img->width >> 24) & 0xFF; ihdr_data[1] = (img->width >> 16) & 0xFF;
    ihdr_data[2] = (img->width >> 8) & 0xFF;  ihdr_data[3] = img->width & 0xFF;
    
    ihdr_data[4] = (img->height >> 24) & 0xFF; ihdr_data[5] = (img->height >> 16) & 0xFF;
    ihdr_data[6] = (img->height >> 8) & 0xFF;  ihdr_data[7] = img->height & 0xFF;
    
    ihdr_data[8] = 8;
    ihdr_data[9] = 2;
    ihdr_data[10] = 0;
    ihdr_data[11] = 0;
    ihdr_data[12] = 0;
    
    write_chunk(file, "IHDR", ihdr_data, 13);

    size_t row_bytes = img->width * 3;
    size_t raw_size = img->height * (row_bytes + 1);
    uint8_t *raw_data = malloc(raw_size);
    
    for(size_t y = 0; y < img->height; y++){

        raw_data[y * (row_bytes + 1)] = 0; 
        
        for(size_t x = 0; x < img->width; x++){

            size_t src_idx = y * img->width + x;
            size_t dest_idx = y * (row_bytes + 1) + 1 + (x * 3);
            
            raw_data[dest_idx]     = (uint8_t)(((uint32_t)img->pixels[src_idx].R * 255) / img->max_pixel_val);
            raw_data[dest_idx + 1] = (uint8_t)(((uint32_t)img->pixels[src_idx].G * 255) / img->max_pixel_val);
            raw_data[dest_idx + 2] = (uint8_t)(((uint32_t)img->pixels[src_idx].B * 255) / img->max_pixel_val);

        }

    }

    uLongf compressed_size = compressBound(raw_size);
    uint8_t *compressed_data = malloc(compressed_size);
    
    if(compress(compressed_data, &compressed_size, raw_data, raw_size) == Z_OK){
        write_chunk(file, "IDAT", compressed_data, compressed_size);
    }else {
        print_error("Zlib compression failed!");
    }

    write_chunk(file, "IEND", NULL, 0);

    free(raw_data);
    free(compressed_data);
    fclose(file);

}
