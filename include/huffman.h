#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <time.h>
#include <string.h> // Added for strstr
#include "../include/bitmap.h"

// Defining Structures pixfreq
struct pixfreq
{
    int pix, larrloc, rarrloc;
    float freq;
    struct pixfreq *left, *right;
    char *code;
};

// Defining Structures huffcode
struct huffcode
{
    int pix, arrloc;
    float freq;
};


int compress(char *fileName, char *outputfile, int pipe_write_fd);
int decompress(char *fileName, int pipe_write_fd);

#endif