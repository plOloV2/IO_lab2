#include "libs.h"
#include "tui.h"
#include "image_operations.h"

#define IMG_SIZE 1024

// Wyznacza ilosc znakow potrzebnych do zapisania liczby w systemie dziesietnym
static size_t get_length(size_t var){
    
    if(var == 0)
        return 1; 
    
    size_t length = 0;
    while(var > 0){
        var /= 10;
        length++;
    }

    return length;

}

// oblicza spodziewany rozmiar pliku .ppm
static size_t expected_ppm_size(Image_data* img){
    
    if(!img)
        return 0;

    // 1. Calculate the exact header size based on how save_image_ppm() formats it:
    // "P3\n" or "P6\n" -> 3 bytes
    // "width height\n" -> length(width) + 1 (space) + length(height) + 1 (\n)
    // "max_val\n"      -> length(max_pixel_val) + 1 (\n)
    size_t header_size = 3 + get_length(img->width) + 1 + get_length(img->height) + 1 + get_length(img->max_pixel_val) + 1;

    size_t data_size = 0;

    // 2. Calculate pixel data size based on the image type
    if(img->img_type == TYPE_P6){
        
        // P6 is binary data, so the size is EXACT.
        if (img->max_pixel_val <= 255) {
            data_size = img->pixel_num * 3; // 1 byte per R, G, B channel
        } else {
            data_size = img->pixel_num * 6; // 2 bytes per R, G, B channel
        }

    }else if(img->img_type == TYPE_P3){
        
        // P3 is ASCII data, so we calculate the MAXIMUM possible size.
        size_t max_chars_per_val = get_length(img->max_pixel_val);
        
        // 3 colors per pixel. Each color is up to `max_chars_per_val` characters + 1 space character
        size_t max_chars_per_pixel = (max_chars_per_val + 1) * 3; 
        
        // Total pixels + 1 newline character for each row (img->height)
        data_size = (max_chars_per_pixel * img->pixel_num) + img->height; 
    }

    return header_size + data_size;

}

void zad1(){

    printf("Rozpoczynam wykonywac zadanie 1.\n");

    size_t all_sizes[4] = {0, 0, 0, 0};

    // Pierwsza część zadania, utworzenie szkicu RGB i zapis go jako .ppm
    Image_data* priv = create_empty(IMG_SIZE, TYPE_P3, IMG_SIZE, IMG_SIZE, "w1_1_p3.ppm");
    if(!priv){
        print_error("Nie udalo sie utworzyc Image_data struct wewnatrz zad1.c");
        return;
    }

    #pragma omp parallel for collapse(2)
    for(int i = 0; i < IMG_SIZE; i++){
        for(int j = 0; j < IMG_SIZE; j++){

            priv->pixels[i * IMG_SIZE + j].R = i;
            priv->pixels[i * IMG_SIZE + j].G = j;
            priv->pixels[i * IMG_SIZE + j].B = IMG_SIZE / 2;

        }

    }

    all_sizes[0] = expected_ppm_size(priv);
    save_image_ppm(priv);

    convert_P3_P6(priv);
    strcpy(priv->name, "w1_1_p6.ppm");
    all_sizes[1] = expected_ppm_size(priv);
    save_image_ppm(priv);

    free_image(priv);


    // Druga część zadania, odczytanie wcześniej utworzonych obrazów .ppm
    priv = load_image("w1_1_p3.ppm");
    strcpy(priv->name, "w1_2_p3.png");
    save_image_stb(priv);
    free_image(priv);


    priv = load_image("w1_1_p6.ppm");
    strcpy(priv->name, "w1_2_p6.png");
    save_image_stb(priv);
    free_image(priv);


    // Trzecia część zadania, odczytanie zwykłego zdjęcia i zapis jako .ppm
    priv = load_image_stb("o.jpg");
    convert_P3_P6(priv);
    strcpy(priv->name, "w1_3_p6.ppm");
    all_sizes[2] = expected_ppm_size(priv);
    save_image_ppm(priv);

    convert_P6_P3(priv);
    strcpy(priv->name, "w1_3_p3.ppm");
    all_sizes[3] = expected_ppm_size(priv);
    save_image_ppm(priv);
    free_image(priv);

    // Wypisanie spodziewanych rozmiarów plików .ppm
    printf("\n\tExpected sizes (in bytes) for each .ppm photo:\n\t1. w1_1_p3.ppm -> %zu\n\t2. w1_1_p6.ppm -> %zu\n\t3. w1_2_p3.ppm -> %zu\n\t4. w1_2_p6.ppm -> %zu\n\nZadanie 1 wykonane.\n", all_sizes[0], all_sizes[1], all_sizes[3], all_sizes[2]);

}
