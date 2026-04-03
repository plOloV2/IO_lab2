#include "libs.h"
#include "tui.h"
#include "image_operations.h"

void zad4(){

    printf("\nRozpoczynam wykonywac zadanie 4.\n");

    Image_data* input = load_image_stb("w3.png");
    if(!input){
        print_error("Nie udalo sie wczytac obrazka w3.png w zad4.c");
        return;
    }

    // Odpalenie algorytmu zgodnie z wymaganiami z zadania:
    process_jpeg_pipeline(input, 1); // 1. Bez próbkowania
    process_jpeg_pipeline(input, 2); // 2. Co drugi element
    process_jpeg_pipeline(input, 4); // 3. Co czwarty element

    free_image(input);

    printf("Zadanie 4 wykonane.\n");

}
