#include <stdio.h>
#include "ansi_terminal.h"

void print_error([[maybe_unused]] const char* erro_msg){

    #ifndef NDEBUG
        fprintf(stderr, ANSI_COLOR_RED ANSI_STYLE_BOLD "ERROR: %s\n", erro_msg);
    #endif

}

void print_time(const char* op_name, double time){

    fprintf(stdout, "%s took %0.3lf ms\n", op_name, (time * 1000.0));

}

void get_user_input(char* in_path){

    fprintf(stdout, "Enter input file path: ");
    scanf("%255s", in_path); 

}
