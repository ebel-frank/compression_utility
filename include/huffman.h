#ifndef HUFFMAN_H
#define HUFFMAN_H

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

#include "../include/bitmap.h"

void generateHuffman(char *fileName);
void decompress(char *fileName);

#endif