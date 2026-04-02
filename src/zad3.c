#include "libs.h"
#include "tui.h"
#include "image_operations.h"

void zad3(){

    Image_data* priv = load_image("w2.ppm");
    if(!priv){
        print_error("Loading image w2.ppm from previouse task failed inside zad3.c");
        return;
    }

    strcpy(priv->name, "w3.png");
    save_image_png(priv);
    free_image(priv);

}
