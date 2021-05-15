#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    int inp = 0;                    //0 is synonymous to birp
    int out = 0;                    //0 is synonymous to birp
    if(validargs(argc, argv)) {
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }
    if(global_options & HELP_OPTION) {
        USAGE(*argv, EXIT_SUCCESS);
        return EXIT_SUCCESS;
    }
    if (global_options | 0x2) {         //test for non birp, aka pgm or ascii or pgm
        inp = 1;
    }
    if (global_options | 0x20) {
        out = 1;
    } else if (global_options | 0x30) {
        out = 2;
    }
    if (inp == 0 && out == 1) {
        pgm_to_birp(stdin, stdout);
    } else if (inp == 0 && out == 2) {
        pgm_to_ascii(stdin, stdout);
    } else if (inp == 1 && out == 0) {
        birp_to_pgm(stdin, stdout);
    } else if (inp == 1 && out == 2) {
        birp_to_ascii(stdin, stdout);
    } else if (inp == 1 && out == 1) {
        birp_to_birp(stdin, stdout);
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
