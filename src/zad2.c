#include "libs.h"
#include "tui.h"
#include "image_operations.h"

#define IMG_PIXEL_SPACE 255
#define IMG_HEIGHT 100

void zad2(){

    Image_data* priv = create_empty(IMG_PIXEL_SPACE, TYPE_P6, (IMG_PIXEL_SPACE + 1) * 7, IMG_HEIGHT, "w2.ppm");
    if(!priv){
        print_error("Image_data struct creation failed inside zad2.c");
        return;
    }

    #pragma omp parallel for collapse(2) 
    for(int i = 0; i < IMG_HEIGHT; i++){
        for(size_t j = 0; j < priv->width; j++){

            int segment = j / (IMG_PIXEL_SPACE + 1);
            int fraction = j % (IMG_PIXEL_SPACE + 1);

            switch(segment){
                case 0: // 1. Black to Blue
                    priv->pixels[i * priv->width + j].R = 0;
                    priv->pixels[i * priv->width + j].G = 0;
                    priv->pixels[i * priv->width + j].B = fraction;
                    break;
                case 1: // 2. Blue to Cyan
                    priv->pixels[i * priv->width + j].R = 0;
                    priv->pixels[i * priv->width + j].G = fraction;
                    priv->pixels[i * priv->width + j].B = IMG_PIXEL_SPACE;
                    break;
                case 2: // 3. Cyan to Green
                    priv->pixels[i * priv->width + j].R = 0;
                    priv->pixels[i * priv->width + j].G = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].B = IMG_PIXEL_SPACE - fraction;
                    break;
                case 3: // 4. Green to Yellow
                    priv->pixels[i * priv->width + j].R = fraction;
                    priv->pixels[i * priv->width + j].G = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].B = 0;
                    break;
                case 4: // 5. Yellow to Red
                    priv->pixels[i * priv->width + j].R = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].G = IMG_PIXEL_SPACE - fraction;
                    priv->pixels[i * priv->width + j].B = 0;
                    break;
                case 5: // 6. Red to Magenta
                    priv->pixels[i * priv->width + j].R = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].G = 0;
                    priv->pixels[i * priv->width + j].B = fraction;
                    break;
                case 6: // 7. Magenta to White
                    priv->pixels[i * priv->width + j].R = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].G = fraction;
                    priv->pixels[i * priv->width + j].B = IMG_PIXEL_SPACE;
                    break;
                case 7: // Exactly White
                    priv->pixels[i * priv->width + j].R = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].G = IMG_PIXEL_SPACE;
                    priv->pixels[i * priv->width + j].B = IMG_PIXEL_SPACE;
                    break;
            }

        }

    }

    save_image_ppm(priv);
    free_image(priv);

}
