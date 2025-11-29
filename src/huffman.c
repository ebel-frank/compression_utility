

#include <stdio.h>
#include <stdlib.h>
#include "../include/huffman.h"

// Function to calculate word length
int codelen(char *code)
{
    int l = 0;
    while (*(code + l) != '\0') // '\0' is the null terminator
        l++;
    return l;
}

// Function to concatenate the words
void strconcat(char *str, char *parentcode, char add)
{
    int i = 0;
    while (*(parentcode + i) != '\0')
    {
        *(str + i) = *(parentcode + i);
        i++;
    }
    if (add != '2') // If add is not '2', append the character and null terminator
    {
        str[i] = add;
        str[i + 1] = '\0';
    }
    else
        str[i] = '\0';
}

// Function to find Fibonacci number
int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

void generateHuffman(char *fileName)
{
    int i, j;
    // int data = 0;
    // int offset;
    // int bpp = 0;
    int width, height;
    // int **image;
    // int temp;

    // test input for height and width
    // height = 2;
    // width = 2;

    BMPResult bmpRes = readImageFile(fileName);
    uint8_t **image = bmpRes.imageData;
    width = bmpRes.header->width;
    height = bmpRes.header->height;
    // creating dummy image values
    // image = (int **)malloc(bmpRes.header->height * sizeof(int *));
    // for (i = 0; i < height; i++)
    // {
    //     image[i] = (int *)malloc(ih->width * sizeof(int));
    // }

    // // sample image
    // image[0][0] = 12;
    // image[0][1] = 5;
    // image[1][0] = 16;
    // image[0][0] = 20;

    // Creating the Histogram
    int hist[256];

    for (i = 0; i < 256; i++)
        hist[i] = 0;

    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++)
            hist[image[i][j]] += 1;

    // Finding number of
    // non-zero occurrences
    int nodes = 0;
    for (i = 0; i < 256; i++)
    {
        if (hist[i] != 0)
            nodes += 1;
    }

    // Calculating minimum probability
    float p = 1.0, ptemp;
    for (i = 0; i < 256; i++)
    {
        ptemp = (hist[i] / (float)(height * width));
        if (ptemp > 0 && ptemp <= p)
            p = ptemp;
    }

    // Calculating max length of code word
    i = 0;
    while ((1 / p) > fib(i))
        i++;
    int maxcodelen = i - 3;

    // Defining Structures pixfreq
    struct pixfreq
    {
        int pix, larrloc, rarrloc;
        float freq;
        struct pixfreq *left, *right;
        char code[maxcodelen];
    };

    // Defining Structures huffcode
    struct huffcode
    {
        int pix, arrloc;
        float freq;
    };

    // Declaring structs
    struct pixfreq *pix_freq;
    struct huffcode *huffcodes;
    int totalnodes = 2 * nodes - 1;
    pix_freq = (struct pixfreq *)malloc(sizeof(struct pixfreq) * totalnodes);
    huffcodes = (struct huffcode *)malloc(sizeof(struct huffcode) * nodes);

    // Initializing
    j = 0;
    int totpix = height * width;
    float tempprob;
    for (i = 0; i < 256; i++)
    {
        if (hist[i] != 0)
        {
            // pixel intensity value
            huffcodes[j].pix = i;
            pix_freq[j].pix = i;

            // location of the node in the pix_freq array
            huffcodes[j].arrloc = j;

            // probability of occurrence
            tempprob = (float)hist[i] / (float)totpix;
            pix_freq[j].freq = tempprob;
            huffcodes[j].freq = tempprob;

            // Declaring the child of leaf node as NULL pointer
            pix_freq[j].left = NULL;
            pix_freq[j].right = NULL;

            // initializing the code word as end of line
            pix_freq[j].code[0] = '\0';
            j++;
        }
    }

    // Sorting the histogram
    struct huffcode temphuff;

    // Sorting w.r.t probability of occurrence
    for (i = 0; i < nodes; i++)
    {
        for (j = i + 1; j < nodes; j++)
        {
            if (huffcodes[i].freq < huffcodes[j].freq)
            {
                temphuff = huffcodes[i];
                huffcodes[i] = huffcodes[j];
                huffcodes[j] = temphuff;
            }
        }
    }

    // Building Huffman Tree
    float sumprob;
    int sumpix;
    int n = 0, k = 0;
    int nextnode = nodes;

    // Since total number of nodes in Huffman Tree is 2*nodes-1
    while (n < nodes - 1)
    {
        // Adding the lowest two probabilities
        sumprob = huffcodes[nodes - n - 1].freq + huffcodes[nodes - n - 2].freq;
        sumpix = huffcodes[nodes - n - 1].pix + huffcodes[nodes - n - 2].pix;

        // Appending to the pix_freq Array
        pix_freq[nextnode].pix = sumpix;
        pix_freq[nextnode].freq = sumprob;
        pix_freq[nextnode].left = &pix_freq[huffcodes[nodes - n - 2].arrloc];
        pix_freq[nextnode].right = &pix_freq[huffcodes[nodes - n - 1].arrloc];
        pix_freq[nextnode].code[0] = '\0';
        i = 0;

        // Sorting and Updating the huffcodes array simultaneously
        while (sumprob <= huffcodes[i].freq)
            i++;

        // inserting the new node in the huffcodes array
        for (k = nodes - 1; k >= 0; k--)
        {
            if (k == i)
            {
                huffcodes[k].pix = sumpix;
                huffcodes[k].freq = sumprob;
                huffcodes[k].arrloc = nextnode;
            }
            else if (k > i)
                huffcodes[k] = huffcodes[k - 1];
        }
        n += 1;
        nextnode += 1;
    }

    // Assigning Code through backtracking
    char left = '0';
    char right = '1';
    // int index;
    for (i = totalnodes - 1; i >= nodes; i--)
    {
        if (pix_freq[i].left != NULL)
            strconcat(pix_freq[i].left->code, pix_freq[i].code, left);
        if (pix_freq[i].right != NULL)
            strconcat(pix_freq[i].right->code, pix_freq[i].code, right);
    }

    // Encode the Image
    int pix_val;
    int l;

    // Writing the Huffman encoded Image into a text file
    FILE *imagehuff = fopen("encoded_image.txt", "wb");
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++)
        {
            pix_val = image[i][j];
            for (l = 0; l < nodes; l++)
                if (pix_val == pix_freq[l].pix)
                    fprintf(imagehuff, "%s", pix_freq[l].code);
        }

    // Printing Huffman Codes
    printf("Huffman Codes::\n\n");
    printf("pixel values ->   Code\n\n");
    for (i = 0; i < nodes; i++)
    {
        printf("    %d      -> %s\n", pix_freq[i].pix, pix_freq[i].code);
    }

    // Calculating Average Bit Length
    float avgbitnum = 0;
    for (i = 0; i < nodes; i++)
        avgbitnum += pix_freq[i].freq * codelen(pix_freq[i].code);
    printf("Average number of bits:: %f\n", avgbitnum);

    // return 0;
}