#ifndef HUFFMAN_H
#define HUFFMAN_H
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


void generateHuffman(char *fileName, char *outputfile, int pipe_write_fd);
void decompress(char *fileName, char *outputfile, int pipe_write_fd);

#endif