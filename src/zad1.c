#include "libs.h"
#include "tui.h"
#include "image_operations.h"

#define IMG_SIZE 255

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

// Oblicza spodziewany rozmiar pliku .ppm
static size_t expected_ppm_size(Image_data* img){
    
    if(!img)
        return 0;

    // 1. Wyznacza długość nagłówka:
    // "P3\n" lub "P6\n" -> 3 bajty
    // "width height\n" -> długość(width) + 1(spacja) + długość(height) + 1(\n)
    // "max_val\n"      -> długość(max_pixel_val) + 1(\n)
    size_t header_size = 3 + get_length(img->width) + 1 + get_length(img->height) + 1 + get_length(img->max_pixel_val) + 1;

    size_t data_size = 0;

    // 2. Wyznacza wielkość danych w zależności od rodzaju kodowania 
    if(img->img_type == TYPE_P6){
        
        // Zliczenie ilości bajtów
        if(img->max_pixel_val <= 255){
            data_size = img->pixel_num * 3;
        }else {
            data_size = img->pixel_num * 6;
        }

    }else if(img->img_type == TYPE_P3){
        
        // Całkowita długość -> (długość(maks_wartość_pixela) + 1(spacja)) * 3(ilość kanałów RGB) * ilość_pixeli + znak nowej lini na końcu linijki(\n)
        size_t max_chars_per_val = get_length(img->max_pixel_val);
        size_t max_chars_per_pixel = (max_chars_per_val + 1) * 3;
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
    printf("\n\tSpodziewany rozmiar w bajtach każdego obrazka.ppm:\n\t1. w1_1_p3.ppm -> %zu\n\t2. w1_1_p6.ppm -> %zu\n\t3. w1_2_p3.ppm -> %zu\n\t4. w1_2_p6.ppm -> %zu\n\nZadanie 1 wykonane.\n", all_sizes[0], all_sizes[1], all_sizes[3], all_sizes[2]);

}
