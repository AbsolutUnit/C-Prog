/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"

int pgm_to_birp(FILE *in, FILE *out) {
    if (in == NULL){
        return -1;
    }
    return -1;
}

int birp_to_pgm(FILE *in, FILE *out) {
    if (in == NULL){
        return -1;
    }
    return -1;
}

int birp_to_birp(FILE *in, FILE *out) {
    if (in == NULL){
        return -1;
    }
    return -1;
}

int pgm_to_ascii(FILE *in, FILE *out) {
    if (in == NULL){
        return -1;
    }
    int h = 0;
    int w = 0;
    img_read_pgm(in, &w, &h, raster_data, RASTER_SIZE_MAX);
    for (int i = 0; i < w * h; i++){
        int data = (int)(*(raster_data + i));
        if (data > 255 || data < 0){
            return -1;
        }
        char datum = data - '0';
        fputc(datum, out);
    }
    return 0;
}

int birp_to_ascii(FILE *in, FILE *out) {
    if (in == NULL){
        return -1;
    }

    return -1;
}

unsigned int atoiReplacement(char *c){
    int value = 0;
    for (int i = 0; *c != '\0'; ++i){
        value = value * 10 + (*c - '0');
        c++;
    }
    return value;
}

int compStr(char *a, char *b){
    int i = 0;
    int ind = 0;
    while (ind == 0) {
        if (*(a + i) > *(b + i)){
            ind = 1;
        }
        else if (*(a + i) < *(b + i)){
            ind = -1;
        }
        if (*(a + i) == '\0') {
            break;
        }
        i++;
    }
    if (ind == 0){
        return 1;
    } else {
        return 0;
    }
}
/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specifed will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere int the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv) {
    global_options = 0x0;                           // initialize global parameters value
    if (argc <= 1) {                                // check for valid amount of args
        return -1;                                  // return negatory
    }
    if (compStr(*(argv + 1), "-h")){          // check for help tag beginning
        global_options = HELP_OPTION;               // set to help value (MSB 1)
        return 0;                                   // return success
    }
    int ibirp = 1;                                  // scoped value to track if input is birp
    int obirp = 1;                                  // scoped value to track if output is birp
    int idef = 1;
    int odef = 1;
    int tdef = 1;
    int transforms = 0;                                    // scoped value to check if transformation specified
    for (int i = 1; i < argc; i++) {
        if (compStr(*(argv + i), "-i")) {                       // check for input param
            if (!transforms) {                                          // check if transform already declared
                if (idef) {
                    if (compStr(*(argv + i + 1), "pgm")) {               // pgm
                        global_options = global_options | 0x1;
                        ibirp = 0;                                            // input no longer birp
                        idef = 0;
                    } else if (compStr(*(argv + i + 1), "birp")) {       // birp
                        global_options = global_options | 0x2;
                        ibirp = 1;
                        idef = 0;
                    } else {
                        return -1;
                    }
                } else {
                    return -1;
                }
                i++;                                                      // increment to skip value
            } else {
                return -1;
            }
        } else if (compStr(*(argv + i), "-o")) {                // output param check
            if (!transforms) {                                          // check if transform already declared
                if (odef) {
                    if (compStr(*(argv + i + 1), "pgm")) {              // pgm
                        global_options = global_options | 0x10;
                        obirp = 0;                                           // output no longer birp
                        odef = 0;
                    } else if (compStr(*(argv + i + 1), "birp")) {      // birp
                        global_options = global_options | 0x20;
                        obirp = 1;
                        odef = 0;
                    } else if (compStr(*(argv + i + 1), "ascii")) {      // ascii
                        global_options = global_options | 0x30;
                        obirp = 0;                                            // output no longer birp
                        odef = 0;
                    } else {
                        return -1;
                    }
                } else {
                    return -1;
                }
                i++;
            } else {
                return -1;
            }
        } else if (compStr(*(argv + i), "-n")) {
            if (tdef) {
                if (obirp && ibirp) {
                    transforms = 1;
                    tdef = 0;
                    global_options = global_options | 0x100;
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        } else if (compStr(*(argv + i), "-t")) {
            if (tdef) {
                if (obirp && ibirp) {
                    if (atoiReplacement(*(argv + i + 1)) <= 255 && atoiReplacement(*(argv + i + 1)) >= 0){
                        unsigned int val = atoiReplacement((*(argv + i + 1)));
                        for (int y = 0; y < 16; y++){
                            val *= 2;
                        }
                        transforms = 1;
                        tdef = 0;
                        global_options = global_options | 0x200;
                        global_options = global_options | val;
                        i++;
                    } else {
                        return -1;
                    }
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        } else if (compStr(*(argv + i), "-z")) {
            if (tdef) {
                if (obirp && ibirp) {
                    if (atoiReplacement(*(argv + i + 1)) <= 16 && atoiReplacement(*(argv + i + 1)) >= 0) {
                        unsigned int val = atoiReplacement((*(argv + i + 1)));
                        val = -(val) + 1;
                        for (int y = 0; y < 16; y++){
                            val *= 2;
                        }
                        transforms = 1;
                        global_options = global_options | 0x300;
                        global_options = global_options | val;
                        i++;
                        tdef = 0;
                    } else {
                        return -1;
                    }
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        } else if (compStr(*(argv + i), "-Z")) {
            if (tdef) {
                if (obirp && ibirp) {
                    if (atoiReplacement(*(argv + i + 1)) <= 16 && atoiReplacement(*(argv + i + 1)) >= 0) {
                        unsigned int val = atoiReplacement((*(argv + i + 1)));
                        for (int y = 0; y < 16; y++) {
                            val *= 2;
                        }
                        transforms = 1;
                        global_options = global_options | 0x300;
                        global_options = global_options | val;
                        i++;
                        tdef = 0;
                    } else {
                        return -1;
                    }
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        } else if (compStr(*(argv + i), "-r")) {
            if (tdef) {
                if (obirp && ibirp) {
                    transforms = 1;
                    global_options = global_options | 0x400;
                    tdef = 0;
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        } else if (compStr(*(argv + i), "-h") && i > 1) {
            return -1;
        } else if (compStr(*(argv + i), NULL)) {
            break;
        } else {
            return -1;
        }
    }
    if (idef){
        global_options = global_options | 0x2;
    }
    if (odef){
        global_options = global_options | 0x20;
    }
    return 0;
}

