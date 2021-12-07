
/************************************************************
 Utilisation du SGF
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "sgf-header.h"


int main() {
    OFILE* file;
    int c;
    
    init_sgf();
    
    printf("\nLISTE DES FICHIERS\n\n");
    list_directory();
    
    printf("\nCONTENU DE essai.txt\n\n");
    file = sgf_open_read("essai.txt");
    while ((c = sgf_getc(file)) > 0) {
        putchar(c);
    }
    sgf_close(file);
    
    return (EXIT_SUCCESS);
}

