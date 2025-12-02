

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
    FILE *image_file = fopen(fileName, "rb");
    if (!image_file)
    {
        printf("Error Opening Image File!\n");
        return;
    }

    int width, height, offset, bpp;

    // Read BMP Metadata
    fseek(image_file, 10, SEEK_SET);
    fread(&offset, 4, 1, image_file);
    fseek(image_file, 18, SEEK_SET);
    fread(&width, 4, 1, image_file);
    fread(&height, 4, 1, image_file);
    fseek(image_file, 28, SEEK_SET);
    fread(&bpp, 2, 1, image_file);

    // Calculate Padding (BMP rows must be multiple of 4 bytes)
    int padding = (4 - (width * 3) % 4) % 4;

    // Allocate memory for the ENTIRE image (3 bytes per pixel)
    // We treat the image as a long 1D array of bytes for easier processing
    long total_bytes = width * height * 3;
    unsigned char *img_data = (unsigned char *)malloc(total_bytes);

    // Read Image Data
    fseek(image_file, offset, SEEK_SET);

    // We assume the input BMP is stored Bottom-Up (Standard).
    // We read it row by row to handle padding.
    long count = 0;
    // unsigned char pad[3];

    // Read row by row
    // Note: We store it exactly as it comes (Bottom-Up)
    for (int i = 0; i < height; i++)
    {
        fread(&img_data[count], 1, width * 3, image_file);
        count += width * 3;
        fseek(image_file, padding, SEEK_CUR); // Skip padding in input
    }
    fclose(image_file);

    // --- GENERATE HISTOGRAM (Color Aware) ---
    int hist[256];
    for (int i = 0; i < 256; i++)
        hist[i] = 0;

    // Count every single byte (R, G, and B separately)
    for (long i = 0; i < total_bytes; i++)
    {
        hist[img_data[i]] += 1;
    }

    int nodes = 0;
    for (int i = 0; i < 256; i++)
        if (hist[i] != 0)
            nodes++;

    // --- STANDARD HUFFMAN TREE BUILDING (Same as before) ---
    // (This logic remains identical, it just processes more data now)

    // ... [Calculations for maxcodelen] ...
    float p = 1.0, ptemp;
    for (int i = 0; i < 256; i++)
    {
        ptemp = (hist[i] / (float)total_bytes);
        if (ptemp > 0 && ptemp <= p)
            p = ptemp;
    }
    int i = 0;
    while ((1 / p) > fib(i))
        i++;
    int maxcodelen = i - 3;
    if (maxcodelen < 50)
        maxcodelen = 50;

    struct pixfreq *pix_freq = (struct pixfreq *)malloc(sizeof(struct pixfreq) * (2 * nodes - 1));
    struct huffcode *huffcodes = (struct huffcode *)malloc(sizeof(struct huffcode) * nodes);

    int j = 0;
    for (int i = 0; i < 256; i++)
    {
        if (hist[i] != 0)
        {
            huffcodes[j].pix = i;
            huffcodes[j].arrloc = j;
            huffcodes[j].freq = (float)hist[i] / (float)total_bytes;
            pix_freq[j].pix = i;
            pix_freq[j].freq = huffcodes[j].freq;
            pix_freq[j].left = NULL;
            pix_freq[j].right = NULL;
            pix_freq[j].code = (char *)malloc(maxcodelen);
            pix_freq[j].code[0] = '\0';
            j++;
        }
    }

    // Sort
    struct huffcode temphuff;
    for (int i = 0; i < nodes; i++)
    {
        for (int k = i + 1; k < nodes; k++)
        {
            if (huffcodes[i].freq < huffcodes[k].freq)
            {
                temphuff = huffcodes[i];
                huffcodes[i] = huffcodes[k];
                huffcodes[k] = temphuff;
            }
        }
    }

    // Build Tree
    float sumprob;
    int sumpix;
    int n = 0, k = 0, nextnode = nodes;
    while (n < nodes - 1)
    {
        sumprob = huffcodes[nodes - n - 1].freq + huffcodes[nodes - n - 2].freq;
        sumpix = huffcodes[nodes - n - 1].pix + huffcodes[nodes - n - 2].pix;
        pix_freq[nextnode].pix = sumpix;
        pix_freq[nextnode].freq = sumprob;
        pix_freq[nextnode].left = &pix_freq[huffcodes[nodes - n - 2].arrloc];
        pix_freq[nextnode].right = &pix_freq[huffcodes[nodes - n - 1].arrloc];
        pix_freq[nextnode].code = (char *)malloc(maxcodelen);
        pix_freq[nextnode].code[0] = '\0';
        int x = 0;
        while (sumprob <= huffcodes[x].freq && x < nodes - n - 2)
            x++;
        for (k = nodes - 1; k >= 0; k--)
        {
            if (k == x)
            {
                huffcodes[k].pix = sumpix;
                huffcodes[k].freq = sumprob;
                huffcodes[k].arrloc = nextnode;
            }
            else if (k > x)
                huffcodes[k] = huffcodes[k - 1];
        }
        n++;
        nextnode++;
    }

    // Generate Codes
    for (int i = 2 * nodes - 2; i >= nodes; i--)
    {
        if (pix_freq[i].left)
            strconcat(pix_freq[i].left->code, pix_freq[i].code, '0');
        if (pix_freq[i].right)
            strconcat(pix_freq[i].right->code, pix_freq[i].code, '1');
    }

    // Lookup Table for Speed
    char *codeTable[256] = {0};
    for (int i = 0; i < nodes; i++)
        codeTable[pix_freq[i].pix] = pix_freq[i].code;

    // --- SAVE TO FILE ---
    FILE *outfile = fopen("encoded_image.txt", "w");
    fprintf(outfile, "%d %d\n%d\n", width, height, nodes); // Header
    for (int i = 0; i < 256; i++)
    {
        if (hist[i] != 0)
            fprintf(outfile, "%d %d\n", i, hist[i]);
    }
    fprintf(outfile, "\n--DATA--\n");

    // Write Encoded Data (Color!)
    for (long i = 0; i < total_bytes; i++)
    {
        fprintf(outfile, "%s", codeTable[img_data[i]]);
    }

    fclose(outfile);
    free(img_data);
    printf("Color Image Encoded Successfully.\n");
}

void decompress(char *fileName)
{
    printf("Decompressing Color Image...\n");
    FILE *encoded_file = fopen(fileName, "r");
    if (!encoded_file)
    {
        printf("File not found!\n");
        return;
    }

    int width, height, nodes;
    fscanf(encoded_file, "%d %d", &width, &height);
    fscanf(encoded_file, "%d", &nodes);

    // --- REBUILD TREE (Standard) ---
    // (Variable declarations and tree building logic same as before)
    struct huffcode *huffcodes = (struct huffcode *)malloc(sizeof(struct huffcode) * nodes);
    struct pixfreq *pix_freq = (struct pixfreq *)malloc(sizeof(struct pixfreq) * (2 * nodes - 1));
    long total_bytes = width * height * 3; // Note: Multiplied by 3 for RGB
    int pix_val, freq_val;

    for (int i = 0; i < nodes; i++)
    {
        fscanf(encoded_file, "%d %d", &pix_val, &freq_val);
        huffcodes[i].pix = pix_val;
        huffcodes[i].freq = (float)freq_val / total_bytes;
        huffcodes[i].arrloc = i;
        pix_freq[i].pix = pix_val;
        pix_freq[i].freq = huffcodes[i].freq;
        pix_freq[i].left = NULL;
        pix_freq[i].right = NULL;
    }

    // Sort
    struct huffcode temphuff;
    for (int i = 0; i < nodes; i++)
    {
        for (int k = i + 1; k < nodes; k++)
        {
            if (huffcodes[i].freq < huffcodes[k].freq)
            {
                temphuff = huffcodes[i];
                huffcodes[i] = huffcodes[k];
                huffcodes[k] = temphuff;
            }
        }
    }
    // Build
    float sumprob;
    int sumpix;
    int n = 0, k = 0, nextnode = nodes;
    while (n < nodes - 1)
    {
        sumprob = huffcodes[nodes - n - 1].freq + huffcodes[nodes - n - 2].freq;
        sumpix = huffcodes[nodes - n - 1].pix + huffcodes[nodes - n - 2].pix;
        pix_freq[nextnode].pix = sumpix;
        pix_freq[nextnode].freq = sumprob;
        pix_freq[nextnode].left = &pix_freq[huffcodes[nodes - n - 2].arrloc];
        pix_freq[nextnode].right = &pix_freq[huffcodes[nodes - n - 1].arrloc];
        int x = 0;
        while (sumprob <= huffcodes[x].freq && x < nodes - n - 2)
            x++;
        for (k = nodes - 1; k >= 0; k--)
        {
            if (k == x)
            {
                huffcodes[k].pix = sumpix;
                huffcodes[k].freq = sumprob;
                huffcodes[k].arrloc = nextnode;
            }
            else if (k > x)
                huffcodes[k] = huffcodes[k - 1];
        }
        n++;
        nextnode++;
    }
    struct pixfreq *root = &pix_freq[nextnode - 1];

    // --- DECODE STREAM ---
    char buffer[100];
    fgets(buffer, 100, encoded_file);
    while (fgets(buffer, 100, encoded_file))
    {
        if (buffer[0] == '-')
            break;
    }

    // Allocate memory for the decoded RGB data
    unsigned char *img_data = (unsigned char *)malloc(total_bytes);
    long idx = 0;

    struct pixfreq *current = root;
    char bit;

    // Read bits and traverse tree
    while (idx < total_bytes)
    {
        bit = fgetc(encoded_file);
        if (bit == EOF)
            break;
        if (bit == '\n' || bit == '\r')
            continue;

        if (bit == '0')
            current = current->left;
        else if (bit == '1')
            current = current->right;

        if (!current->left && !current->right)
        {
            img_data[idx++] = (unsigned char)current->pix;
            current = root;
        }
    }
    fclose(encoded_file);

    // --- WRITE BMP FILE (Orientation Fix) ---
    FILE *out_file = fopen("restored_image.bmp", "wb");

    int padding = (4 - (width * 3) % 4) % 4;
    int filesize = 54 + (3 * width + padding) * height;

    unsigned char header[54] = {
        'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    *(int *)&header[2] = filesize;
    *(int *)&header[18] = width;
    *(int *)&header[22] = height; // POSITIVE height = Bottom-Up storage

    fwrite(header, 1, 54, out_file);

    unsigned char padBytes[3] = {0};

    // Note: The encoder read the file Bottom-Up (standard BMP).
    // So 'img_data' contains the bottom row at index 0.
    // Therefore, we just write it out linearly to maintain the Bottom-Up format.

    long row_size = width * 3;
    for (int i = 0; i < height; i++)
    {
        // Write the row
        fwrite(&img_data[i * row_size], 1, row_size, out_file);
        // Write padding
        fwrite(padBytes, 1, padding, out_file);
    }

    fclose(out_file);
    free(img_data);
    printf("Decompression Complete. Orientation corrected.\n");
}