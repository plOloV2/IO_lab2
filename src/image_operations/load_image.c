#include "libs.h"
#include "tui.h"
#include "img_struct.h"

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
