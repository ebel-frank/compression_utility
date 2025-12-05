#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>     // Added for clock()
#include <sys/stat.h> // Added for file size
#include "../include/huffman.h"

// --- STATS HELPERS ---

// // Helper: Convert bytes to human readable string
// void format_size(long bytes, char *buffer)
// {
//     if (bytes >= 1024 * 1024)
//         sprintf(buffer, "%.2f MB", bytes / (1024.0 * 1024.0));
//     else if (bytes >= 1024)
//         sprintf(buffer, "%.2f KB", bytes / 1024.0);
//     else
//         sprintf(buffer, "%ld B", bytes);
// }

// // Helper: Get file size
// long get_file_size(const char *filename)
// {
//     struct stat st;
//     if (stat(filename, &st) == 0)
//         return st.st_size;
//     return -1;
// }

// --- BIT PACKING GLOBALS & HELPERS ---

unsigned char buffer = 0;
int bit_count = 0;

// Function to write a single bit to a file
void writeBit(int bit, FILE *f)
{
    if (bit == 1)
    {
        buffer = buffer | (1 << (7 - bit_count)); // Set bit at position
    }
    // If bit is 0, we just skip it (buffer is init to 0)

    bit_count++;
    if (bit_count == 8)
    {
        fwrite(&buffer, 1, 1, f); // Buffer full, write to file
        buffer = 0;
        bit_count = 0;
    }
}

// Function to flush remaining bits
void flushBits(FILE *f)
{
    if (bit_count > 0)
    {
        fwrite(&buffer, 1, 1, f);
        buffer = 0;
        bit_count = 0;
    }
}

// Global variables for Bit Reading
unsigned char read_buffer = 0;
int read_bit_count = 8; // Start at 8 to force a read on first call

// Function to read a single bit from file
int readBit(FILE *f)
{
    if (read_bit_count == 8)
    {
        fread(&read_buffer, 1, 1, f);
        read_bit_count = 0;
    }

    int bit = (read_buffer >> (7 - read_bit_count)) & 1;
    read_bit_count++;
    return bit;
}

// --- STANDARD UTILS ---

// Function to calculate word length
int codelen(char *code)
{
    int l = 0;
    while (*(code + l) != '\0')
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
    if (add != '2')
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

// --- ENCODER ---

void generateHuffman(char *fileName, char *outputfile, int pipe_write_fd)
{
    clock_t start_time = clock(); // Start Timer

    FILE *image_file = fopen(fileName, "rb");
    if (!image_file)
    {
        printf("Error Opening Image File!\n");
        return;
    }

    // Get Original Size for stats
    long original_size = get_file_size(fileName);

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

    // Allocate memory for the ENTIRE image
    long total_bytes = width * height * 3;
    unsigned char *img_data = (unsigned char *)malloc(total_bytes);

    // Read Image Data
    fseek(image_file, offset, SEEK_SET);

    long count = 0;

    // Read row by row (Bottom-Up)
    for (int i = 0; i < height; i++)
    {
        fread(&img_data[count], 1, width * 3, image_file);
        count += width * 3;
        fseek(image_file, padding, SEEK_CUR); // Skip padding in input
    }
    fclose(image_file);

    // --- GENERATE HISTOGRAM ---
    int hist[256];
    for (int i = 0; i < 256; i++)
        hist[i] = 0;

    for (long i = 0; i < total_bytes; i++)
    {
        hist[img_data[i]] += 1;
    }

    int nodes = 0;
    for (int i = 0; i < 256; i++)
        if (hist[i] != 0)
            nodes++;

    // --- HUFFMAN TREE BUILDING ---
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

    // Lookup Table
    char *codeTable[256] = {0};
    for (int i = 0; i < nodes; i++)
        codeTable[pix_freq[i].pix] = pix_freq[i].code;

    // --- STOP TIMER & SAVE TO FILE ---

    FILE *outfile = fopen(outputfile, "w");

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    // Header Format: OriginalSize TimeTaken Width Height
    fprintf(outfile, "%ld %f %d %d\n", original_size, time_taken, width, height);
    fprintf(outfile, "%d\n", nodes); // Write nodes on next line

    for (int i = 0; i < 256; i++)
    {
        if (hist[i] != 0)
            fprintf(outfile, "%d %d\n", i, hist[i]);
    }
    fprintf(outfile, "\n--DATA--\n");

    // Reset globals
    buffer = 0;
    bit_count = 0;

    // Write Encoded Data using Bit Packing
    for (long i = 0; i < total_bytes; i++)
    {
        char *code = codeTable[img_data[i]];

        for (int k = 0; code[k] != '\0'; k++)
        {
            if (code[k] == '1')
                writeBit(1, outfile);
            else
                writeBit(0, outfile);
        }
    }

    // IMPORTANT: Write the last partial byte if exists
    flushBits(outfile);

    fclose(outfile);
    free(img_data);
    printf("Compression Complete: Saved to %s\n", outputfile);
}

// --- DECODER ---

void decompress(char *fileName, char *outputfile, int pipe_write_fd)
{
    FILE *encoded_file = fopen(fileName, "r");
    if (!encoded_file)
    {
        printf("File not found!\n");
        return;
    }

    int width, height, nodes;
    long original_size;
    double time_taken;

    // --- READ UPDATED HEADER ---
    // We scan original_size and time_taken, but ignore them for decompression logic
    fscanf(encoded_file, "%ld %f %d %d", &original_size, &time_taken, &width, &height);
    fscanf(encoded_file, "%d", &nodes);

    // --- REBUILD TREE ---
    struct huffcode *huffcodes = (struct huffcode *)malloc(sizeof(struct huffcode) * nodes);
    struct pixfreq *pix_freq = (struct pixfreq *)malloc(sizeof(struct pixfreq) * (2 * nodes - 1));
    long total_bytes = width * height * 3;
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
    char line_buffer[100];

    // Read lines until we find the marker "--DATA--"
    while (fgets(line_buffer, 100, encoded_file))
    {
        if (strstr(line_buffer, "--DATA--"))
            break;
    }

    // Allocate memory for the decoded RGB data
    unsigned char *img_data = (unsigned char *)malloc(total_bytes);
    long idx = 0;

    struct pixfreq *current = root;
    int bit;

    // Reset binary reader globals
    read_buffer = 0;
    read_bit_count = 8;

    // Read bits using helper function (BINARY MODE)
    while (idx < total_bytes)
    {
        bit = readBit(encoded_file);

        if (bit == 0)
            current = current->left;
        else
            current = current->right;

        if (!current->left && !current->right)
        {
            img_data[idx++] = (unsigned char)current->pix;
            current = root;
        }
    }
    fclose(encoded_file);

    // --- WRITE BMP FILE ---
    FILE *out_file = fopen(outputfile, "wb");

    int padding = (4 - (width * 3) % 4) % 4;
    int filesize = 54 + (3 * width + padding) * height;

    unsigned char header[54] = {
        'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    *(int *)&header[2] = filesize;
    *(int *)&header[18] = width;
    *(int *)&header[22] = height; // Positive height = Bottom-Up

    fwrite(header, 1, 54, out_file);

    unsigned char padBytes[3] = {0};
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
    printf("Decompression Complete: Saved to %s\n", outputfile);
}