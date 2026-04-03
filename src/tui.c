#include <stdio.h>
#include "ansi_terminal.h"

// Wypisanie błędów do konsoli w trybie DEBUG
void print_error([[maybe_unused]] const char* erro_msg){

    #ifndef NDEBUG
        fprintf(stderr, ANSI_COLOR_RED ANSI_STYLE_BOLD "ERROR: %s\n", erro_msg);
    #endif

}
