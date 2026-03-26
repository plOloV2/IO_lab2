#include "libs.h"
#include "image_operations.h"

void zad1(){

    Image_data* priv = create_empty(1024, TYPE_P6, 1024, 1024, "w1_1.ppm");
    save_image(priv);

    convert_P3_P6(priv);
    priv->name = "w1_4_p6.ppm";
    save_image(priv);

    free_image(priv);

}
