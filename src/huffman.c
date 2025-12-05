#include "../include/huffman.h"

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

// Function to flush remaining bits (ADDED)
void flushBits(FILE *f)
{
    if (bit_count > 0)
    {
        fwrite(&buffer, 1, 1, f);
        buffer = 0;
        bit_count = 0;
    }
}

// Global variables for Bit Reading (ADDED)
unsigned char read_buffer = 0;
int read_bit_count = 8; // Start at 8 to force a read on first call

// Function to read a single bit from file (ADDED)
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

// --- ENCODER ---
int compress(char *fileName, char *outputfile, int pipe_write_fd)
{
    int progress = 0;

    // Open file as binary
    FILE *input_file = fopen(fileName, "rb");
    if (!input_file)
    {
        printf("Error Opening File!\n");
        return EXIT_FAILURE;
    }

    // Get file size
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    if (file_size == 0)
    {
        printf("Error: Empty file!\n");
        fclose(input_file);
        return EXIT_FAILURE;
    }

    progress = 5;
    write(pipe_write_fd, &progress, sizeof(progress));

    // Read entire file into memory
    unsigned char *file_data = (unsigned char *)malloc(file_size);
    if (!file_data)
    {
        printf("Error: Memory allocation failed!\n");
        fclose(input_file);
        return EXIT_FAILURE;
    }

    size_t bytes_read = fread(file_data, 1, file_size, input_file);
    fclose(input_file);

    if (bytes_read != file_size)
    {
        printf("Error: Failed to read file completely!\n");
        free(file_data);
        return EXIT_FAILURE;
    }

    progress = 10;
    write(pipe_write_fd, &progress, sizeof(progress));

    // --- GENERATE HISTOGRAM ---
    int hist[256];
    for (int i = 0; i < 256; i++)
        hist[i] = 0;

    for (long i = 0; i < file_size; i++)
    {
        hist[file_data[i]] += 1;
    }

    int nodes = 0;
    for (int i = 0; i < 256; i++)
        if (hist[i] != 0)
            nodes++;

    progress = 20;
    write(pipe_write_fd, &progress, sizeof(progress));

    // --- HUFFMAN TREE BUILDING ---
    float p = 1.0, ptemp;
    for (int i = 0; i < 256; i++)
    {
        ptemp = (hist[i] / (float)file_size);
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
            huffcodes[j].freq = (float)hist[i] / (float)file_size;
            pix_freq[j].pix = i;
            pix_freq[j].freq = huffcodes[j].freq;
            pix_freq[j].left = NULL;
            pix_freq[j].right = NULL;
            pix_freq[j].code = (char *)malloc(maxcodelen);
            pix_freq[j].code[0] = '\0';
            j++;
        }
    }

    progress = 30;
    write(pipe_write_fd, &progress, sizeof(progress));

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

    progress = 40;
    write(pipe_write_fd, &progress, sizeof(progress));

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

    progress = 50;
    write(pipe_write_fd, &progress, sizeof(progress));

    // Generate Codes
    for (int i = 2 * nodes - 2; i >= nodes; i--)
    {
        if (pix_freq[i].left)
            strconcat(pix_freq[i].left->code, pix_freq[i].code, '0');
        if (pix_freq[i].right)
            strconcat(pix_freq[i].right->code, pix_freq[i].code, '1');
    }

    progress = 60;
    write(pipe_write_fd, &progress, sizeof(progress));

    // Lookup Table
    char *codeTable[256] = {0};
    for (int i = 0; i < nodes; i++)
        codeTable[pix_freq[i].pix] = pix_freq[i].code;

    progress = 70;
    write(pipe_write_fd, &progress, sizeof(progress));

    // --- SAVE TO BINARY FILE ---
    FILE *outfile = fopen(outputfile, "wb");
    if (!outfile)
    {
        printf("Error: Cannot create output file!\n");
        free(file_data);
        return EXIT_FAILURE;
    }

    // Write custom GFW header (binary format)
    // Magic number "GFW" + version
    fwrite("GFW", 1, 3, outfile);
    unsigned char version = 1;
    fwrite(&version, 1, 1, outfile);

    // Write original file size
    fwrite(&file_size, sizeof(long), 1, outfile);

    // Write number of nodes
    fwrite(&nodes, sizeof(int), 1, outfile);

    // Write histogram (frequency table)
    for (int i = 0; i < 256; i++)
    {
        if (hist[i] != 0)
        {
            unsigned char byte_val = (unsigned char)i;
            fwrite(&byte_val, 1, 1, outfile);
            fwrite(&hist[i], sizeof(int), 1, outfile);
        }
    }

    progress = 75;
    write(pipe_write_fd, &progress, sizeof(progress));

    // Reset globals
    buffer = 0;
    bit_count = 0;

    // Write Encoded Data using Bit Packing
    long progress_interval = file_size / 20; // Update every 5% (20 updates from 75% to 95%)
    if (progress_interval == 0) progress_interval = 1;

    for (long i = 0; i < file_size; i++)
    {
        char *code = codeTable[file_data[i]]; // Get the string "101" etc.

        // Loop through the character string "101"
        for (int k = 0; code[k] != '\0'; k++)
        {
            if (code[k] == '1')
                writeBit(1, outfile);
            else
                writeBit(0, outfile);
        }

        // Report progress during encoding (75-95%)
        if (i % progress_interval == 0)
        {
            progress = 75 + (i * 20) / file_size;
            write(pipe_write_fd, &progress, sizeof(progress));
        }
    }

    progress = 95;
    write(pipe_write_fd, &progress, sizeof(progress));

    // IMPORTANT: Write the last partial byte if exists
    flushBits(outfile);

    fclose(outfile);
    free(file_data);

    // Free allocated memory
    for (int i = 0; i < 2 * nodes - 1; i++)
    {
        if (pix_freq[i].code)
            free(pix_freq[i].code);
    }
    free(pix_freq);
    free(huffcodes);

    progress = 100;
    write(pipe_write_fd, &progress, sizeof(progress));

    return EXIT_SUCCESS;
}

// --- DECODER ---
int decompress(char *fileName, int pipe_write_fd)
{
    int progress = 0;
    
    FILE *encoded_file = fopen(fileName, "rb");
    if (!encoded_file)
    {
        perror("gfwrap error: ");
        return EXIT_FAILURE;
    }

    // Read and verify GFW header
    char magic[4] = {0};
    fread(magic, 1, 3, encoded_file);
    
    if (strncmp(magic, "GFW", 3) != 0)
    {
        perror("gfwrap error: Invalid input file format");
        fclose(encoded_file);
        return EXIT_FAILURE;
    }

    unsigned char version;
    fread(&version, 1, 1, encoded_file);

    progress = 5;
    write(pipe_write_fd, &progress, sizeof(progress));

    // Read original file size
    long file_size;
    fread(&file_size, sizeof(long), 1, encoded_file);

    // Read number of nodes
    int nodes;
    fread(&nodes, sizeof(int), 1, encoded_file);

    progress = 10;
    write(pipe_write_fd, &progress, sizeof(progress));

    // --- REBUILD HUFFMAN TREE ---
    struct huffcode *huffcodes = (struct huffcode *)malloc(sizeof(struct huffcode) * nodes);
    struct pixfreq *pix_freq = (struct pixfreq *)malloc(sizeof(struct pixfreq) * (2 * nodes - 1));
    
    // Read frequency table
    for (int i = 0; i < nodes; i++)
    {
        unsigned char byte_val;
        int freq_val;
        fread(&byte_val, 1, 1, encoded_file);
        fread(&freq_val, sizeof(int), 1, encoded_file);
        
        huffcodes[i].pix = byte_val;
        huffcodes[i].freq = (float)freq_val / file_size;
        huffcodes[i].arrloc = i;
        pix_freq[i].pix = byte_val;
        pix_freq[i].freq = huffcodes[i].freq;
        pix_freq[i].left = NULL;
        pix_freq[i].right = NULL;
    }

    progress = 20;
    write(pipe_write_fd, &progress, sizeof(progress));

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

    progress = 30;
    write(pipe_write_fd, &progress, sizeof(progress));

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

    progress = 40;
    write(pipe_write_fd, &progress, sizeof(progress));

    // --- DECODE STREAM ---
    // Allocate memory for the decoded data
    unsigned char *file_data = (unsigned char *)malloc(file_size);
    long idx = 0;

    struct pixfreq *current = root;
    int bit;

    // Reset binary reader globals
    read_buffer = 0;
    read_bit_count = 8;

    // Calculate progress interval for decoding (40-80%)
    long progress_interval = file_size / 20; // Update every 2%
    if (progress_interval == 0) progress_interval = 1;

    // Read bits and decode
    while (idx < file_size)
    {
        bit = readBit(encoded_file);

        if (bit == 0)
            current = current->left;
        else
            current = current->right;

        if (!current->left && !current->right)
        {
            file_data[idx++] = (unsigned char)current->pix;
            current = root;
            
            // Report progress during decoding (40-80%)
            if (idx % progress_interval == 0)
            {
                progress = 40 + (idx * 40) / file_size;
                write(pipe_write_fd, &progress, sizeof(progress));
            }
        }
    }
    fclose(encoded_file);

    progress = 80;
    write(pipe_write_fd, &progress, sizeof(progress));

    // --- DETERMINE FILE TYPE FROM HEADER ---
    char *extension = ".bin"; // Default extension
    
    // Check for common file signatures
    if (file_size >= 2 && file_data[0] == 0xFF && file_data[1] == 0xD8)
    {
        extension = ".jpg"; // JPEG
    }
    else if (file_size >= 8 && file_data[0] == 0x89 && file_data[1] == 'P' && 
             file_data[2] == 'N' && file_data[3] == 'G')
    {
        extension = ".png"; // PNG
    }
    else if (file_size >= 2 && file_data[0] == 'B' && file_data[1] == 'M')
    {
        extension = ".bmp"; // BMP
    }

    progress = 85;
    write(pipe_write_fd, &progress, sizeof(progress));

    // Create output filename
    char output_filename[256];
    // Find the last dot in the input file name to replace the extension (.gfw)
    const char *dot = strrchr(fileName, '.');
    size_t base_len = 0;

    if (dot && dot != fileName) {
        // Calculate the length of the base name including the path
        base_len = dot - fileName;
    } else {
        // If no dot, use the full length of the filename as base
        base_len = strlen(fileName);
    }

    // Ensure the base name fits in the buffer, leaving space for the extension and null terminator
    if (base_len >= sizeof(output_filename) - 1) {
        base_len = sizeof(output_filename) - 1;
    }
    
    // Copy the base name (including path) safely
    strncpy(output_filename, fileName, base_len);
    output_filename[base_len] = '\0'; // Null-terminate the base
    snprintf(output_filename + base_len, sizeof(output_filename) - base_len, "%s", extension);

    // Write the decompressed file
    FILE *out_file = fopen(output_filename, "wb");
    if (!out_file)
    {
        // Cannot create output file
        free(file_data);
        free(pix_freq);
        free(huffcodes);
        return EXIT_FAILURE;
    }

    progress = 90;
    write(pipe_write_fd, &progress, sizeof(progress));

    // Write all data at once
    fwrite(file_data, 1, file_size, out_file);

    fclose(out_file);
    free(file_data);
    free(pix_freq);
    free(huffcodes);

    progress = 100;
    write(pipe_write_fd, &progress, sizeof(progress));

    return EXIT_SUCCESS;
}