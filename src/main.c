#include "libs.h"
#include "tui.h"
#include "zadanka.h"
#include "image_operations.h"

int main(){

    double time = omp_get_wtime();

    zad1();
    zad2();
    zad3();
    zad4();

    print_time("\nall", omp_get_wtime() - time);

    return 0;

}
