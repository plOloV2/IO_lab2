#include "libs.h"
#include "tui.h"
#include "img_struct.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static void skip_comments(FILE *fp){
    
    int ch;

    while(1){

        while((ch = fgetc(fp)) != EOF && isspace(ch));

        if(ch == '#'){
            while((ch = fgetc(fp)) != '\n' && ch != EOF);
        }else {
            ungetc(ch, fp);
            break;
        }

    }

}

Image_data* create_empty(uint16_t max_pixel_val, PPM_TYPE img_type, size_t width, size_t height, char* name){

    if(max_pixel_val == 0 || width == 0 || height == 0 || !name){
        print_error("One or more variables passed to create_empty() are incorect");
        return NULL;
    }

    Image_data* new = malloc(sizeof(Image_data));
    if(!new){
        print_error("Image_data* alloc failed");
        return NULL;
    }

    new->max_pixel_val = max_pixel_val;
    new->img_type = img_type;
    new->height = height;
    new->width = width;
    new->pixel_num = height * width;

    new->pixels = malloc(new->pixel_num * sizeof(Pixel_data));
    if(!new->pixels){
        print_error("Pixels array malloc failed");
        free(new);
        return NULL;
    }

    new->name = malloc(PATH_SIZE_M * sizeof(char));
    if(!new->name){
        print_error("New image name alloc failed");
        free(new->pixels);
        free(new);
        return NULL;
    }
    strcpy(new->name, name);

    return new;

}

Image_data* load_image(char* path){

    FILE *file = fopen(path, "rb");
    if(!file){
        print_error("Opening file failed. Maybe the path is wrong?");
        return NULL;
    }

    char type[3];
    if(fscanf(file, "%2s", type) != 1){
        print_error("Cannot read image type");
        fclose(file);
        return NULL;
    }

    PPM_TYPE img_type;

    if(strcmp(type, "P3") == 0){

        img_type = TYPE_P3;

    }else if(strcmp(type, "P6") == 0){

        img_type = TYPE_P6;

    }else {
        print_error("Incorect image type detected");
        fclose(file);
        return NULL;
    }

    size_t width, height;
    uint16_t max_pixel_val;

    skip_comments(file);
    fscanf(file, "%zu", &width);
    skip_comments(file);
    fscanf(file, "%zu", &height);
    skip_comments(file);
    fscanf(file, "%hu", &max_pixel_val);
    fgetc(file);

    Image_data* img = create_empty(max_pixel_val, img_type, width, height, path);
    if(!img){
        fclose(file);
        return NULL;
    }
    
    if(img->img_type == TYPE_P3){
        
        for(size_t i = 0; i < img->pixel_num; i++){

            unsigned int r, g, b;
            fscanf(file, "%u %u %u", &r, &g, &b);

            img->pixels[i].R = (uint16_t)r;
            img->pixels[i].G = (uint16_t)g;
            img->pixels[i].B = (uint16_t)b;

        }

    }else {

        if(img->max_pixel_val <= 255){
            
            uint8_t *raw_pixels = malloc(img->pixel_num * 3);
            if(!raw_pixels){
                print_error("Raw_pixels array alloc failed");
                fclose(file);
                return NULL;
            }

            fread(raw_pixels, 1, img->pixel_num * 3, file);
            
            #pragma omp parallel for
            for(size_t i = 0; i < img->pixel_num; i++){
                img->pixels[i].R = raw_pixels[i * 3];
                img->pixels[i].G = raw_pixels[i * 3 + 1];
                img->pixels[i].B = raw_pixels[i * 3 + 2];
            }
            free(raw_pixels);

        }else {

            size_t read_size = img->pixel_num * 3 * sizeof(uint8_t) * 2;
            
            uint8_t *raw_pixels = malloc(read_size);
            if(!raw_pixels){
                print_error("Raw_pixels array alloc failed");
                fclose(file);
                return NULL;
            }

            fread(raw_pixels, 1, read_size, file);
            
            #pragma omp parallel for
            for (size_t i = 0; i < img->pixel_num; i++) {
                img->pixels[i].R = (raw_pixels[i * 6] << 8) | raw_pixels[i * 6 + 1];
                img->pixels[i].G = (raw_pixels[i * 6 + 2] << 8) | raw_pixels[i * 6 + 3];
                img->pixels[i].B = (raw_pixels[i * 6 + 4] << 8) | raw_pixels[i * 6 + 5];
            }
            free(raw_pixels);

        }

    }

    fclose(file);
    return img;

}

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

void save_image(Image_data* img){

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

void save_image_stb(Image_data* img){
    if (!img || !img->name) {
        print_error("Invalid image or output path provided to save_image_stb.");
        return;
    }

    uint8_t *raw_pixels = malloc(img->pixel_num * 3);
    if (!raw_pixels) {
        print_error("Failed to allocate raw_pixels array for STB saving.");
        return;
    }

    #pragma omp parallel for
    for (size_t i = 0; i < img->pixel_num; i++) {
        raw_pixels[i * 3 + 0] = (uint8_t)(((uint32_t)img->pixels[i].R * 255) / img->max_pixel_val);
        raw_pixels[i * 3 + 1] = (uint8_t)(((uint32_t)img->pixels[i].G * 255) / img->max_pixel_val);
        raw_pixels[i * 3 + 2] = (uint8_t)(((uint32_t)img->pixels[i].B * 255) / img->max_pixel_val);
    }

    const char *ext = strrchr(img->name, '.');
    int success = 0;
    
    if(ext){
        if(strcmp(ext, ".png") == 0){

            // width * 3 is the 'stride' (bytes per row)
            success = stbi_write_png(img->name, img->width, img->height, 3, raw_pixels, img->width * 3);

        }else if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0){

            // 90 represents the JPEG quality (1-100)
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

void convert_P3_P6(Image_data* img){

    if(img)
        img->img_type = TYPE_P6;
    
}

void convert_P6_P3(Image_data* img){

    if(img)
        img->img_type = TYPE_P3;

}

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
