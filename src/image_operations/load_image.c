#include "libs.h"
#include "tui.h"
#include "img_struct.h"

// Funkcja pomocnicza: pomijanie komentarzy wewnątrz plików .ppm
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

// Utworzenie pustej struktury obrazu
Image_data* create_empty(uint16_t max_pixel_val, PPM_TYPE img_type, size_t width, size_t height, char* name){

    if(max_pixel_val == 0 || width == 0 || height == 0 || !name){
        print_error("Jedna lub wiecej zminnych przekazanych do create_empty() jest niepoprawna");
        return NULL;
    }

    Image_data* new = malloc(sizeof(Image_data));
    if(!new){
        print_error("Image_data* alloc spadl z rowerka");
        return NULL;
    }

    new->max_pixel_val = max_pixel_val;
    new->img_type = img_type;
    new->height = height;
    new->width = width;
    new->pixel_num = height * width;

    new->pixels = calloc(new->pixel_num, sizeof(Pixel_data));
    if(!new->pixels){
        print_error("Pixels array alloc spadl z rowerka");
        free(new);
        return NULL;
    }

    new->name = malloc(PATH_SIZE_M * sizeof(char));
    if(!new->name){
        print_error("Alokacja nazwy obrazka spadla z rowerka");
        free(new->pixels);
        free(new);
        return NULL;
    }
    strcpy(new->name, name);

    return new;

}

// Wczytywanie obrazu w formacie .ppm
Image_data* load_image(char* path){

    FILE *file = fopen(path, "rb");
    if(!file){
        print_error("Nie udalo sie otworzyc pliku. Moze podana siecka nie jest prawidlowa?");
        return NULL;
    }

    char type[3];
    if(fscanf(file, "%2s", type) != 1){
        print_error("Nie mozna odczytac typu pliku");
        fclose(file);
        return NULL;
    }

    PPM_TYPE img_type;

    if(strcmp(type, "P3") == 0){

        img_type = TYPE_P3;

    }else if(strcmp(type, "P6") == 0){

        img_type = TYPE_P6;

    }else {
        print_error("Wykryto niepoprawny nieobslugiwany typ pliku");
        fclose(file);
        return NULL;
    }

    // Pobranie wysokości i szerokości grafiki z pliku
    size_t width, height;
    uint16_t max_pixel_val;

    skip_comments(file);
    fscanf(file, "%zu", &width);
    skip_comments(file);
    fscanf(file, "%zu", &height);
    skip_comments(file);
    fscanf(file, "%hu", &max_pixel_val);
    fgetc(file);

    // Utworzenie pustego obrazka
    Image_data* img = create_empty(max_pixel_val, img_type, width, height, path);
    if(!img){
        fclose(file);
        return NULL;
    }
    
    // Odczytanie danych pikseli z pliku i zapisanie w pamięci
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
                print_error("Raw_pixels array alloc spadl z rowerka");
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
                print_error("Raw_pixels array alloc spadl z rowerka");
                fclose(file);
                return NULL;
            }

            fread(raw_pixels, 1, read_size, file);
            
            #pragma omp parallel for
            for(size_t i = 0; i < img->pixel_num; i++){
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
