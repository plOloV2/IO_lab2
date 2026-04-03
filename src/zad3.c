#include "libs.h"
#include "tui.h"
#include "image_operations.h"

void zad3(){

    printf("\nRozpoczynam wykonywac zadanie 3.\n");

    Image_data* priv = load_image("w2.ppm");
    if(!priv){
        print_error("Zaladowanie obrazka w2.ppm spadlo z rowerka w zad3.c");
        return;
    }

    strcpy(priv->name, "w3.png");
    save_image_png(priv);
    free_image(priv);

    printf("Zadanie 3 wykonane.\n");

}
