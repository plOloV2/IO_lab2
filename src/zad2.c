#include "libs.h"
#include "tui.h"
#include "image_operations.h"

#define IMG_PIXEL_SPACE 255
#define IMG_HEIGHT 100

void zad2(){

    printf("\nRozpoczynam wykonywac zadanie 2.\n");

    Image_data* priv = create_empty(IMG_PIXEL_SPACE, TYPE_P6, (IMG_PIXEL_SPACE + 1) * 7, IMG_HEIGHT, "w2.ppm");
    if(!priv){
        print_error("Nie udalo sie utworzyc Image_data struct wewnatrz zad2.c");
        return;
    }

    // Wyznaczenie wartości RGB każdego pixela
    #pragma omp parallel for collapse(2) 
    for(int i = 0; i < IMG_HEIGHT; i++){
        for(size_t j = 0; j < priv->width; j++){

            int segment = j / (IMG_PIXEL_SPACE + 1);
            int fraction = j % (IMG_PIXEL_SPACE + 1);

            switch(segment){
                case 0: // 1. Czarny do Niebieskiego
                    priv->pixels[i * priv->width + j].R = 0;
                    priv->pixels[i * priv->width + j].G = 0;
                    priv->pixels[i * priv->width + j].B = fraction;
                    break;
                case 1: // 2. Niebieski do Cyjanowego
                    priv->pixels[i * priv->width + j].R = 0;
                    priv->pixels[i * priv->width + j].G = fraction;
                    priv->pixels[i * priv->width + j].B = IMG_PIXEL_SPACE;
                    break;
                case 2: // 3. Cyjanowy do Zielonego
                    priv->pixels[i * priv->width + j].R = 0;
                    priv->pixels[i * priv->width + j].G = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].B = IMG_PIXEL_SPACE - fraction;
                    break;
                case 3: // 4. Zielony do żółtego
                    priv->pixels[i * priv->width + j].R = fraction;
                    priv->pixels[i * priv->width + j].G = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].B = 0;
                    break;
                case 4: // 5. Żółty do Czerwonego
                    priv->pixels[i * priv->width + j].R = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].G = IMG_PIXEL_SPACE - fraction;
                    priv->pixels[i * priv->width + j].B = 0;
                    break;
                case 5: // 6. Czerwony do Magenty
                    priv->pixels[i * priv->width + j].R = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].G = 0;
                    priv->pixels[i * priv->width + j].B = fraction;
                    break;
                case 6: // 7. Magenta do Białego
                    priv->pixels[i * priv->width + j].R = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].G = fraction;
                    priv->pixels[i * priv->width + j].B = IMG_PIXEL_SPACE;
                    break;
                case 7: // Idealnie Biały na koniec
                    priv->pixels[i * priv->width + j].R = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].G = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].B = IMG_PIXEL_SPACE;
                    break;
            }

        }

    }

    save_image_ppm(priv);
    free_image(priv);

    printf("Zadanie 2 wykonane.\n");

}
