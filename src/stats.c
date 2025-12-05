
#include "../include/stats.h"

// Helper: Convert bytes to human readable string
void format_size(long bytes, char *buffer)
{
    if (bytes >= 1024 * 1024)
        sprintf(buffer, "%.2f MB", bytes / (1024.0 * 1024.0));
    else if (bytes >= 1024)
        sprintf(buffer, "%.2f KB", bytes / 1024.0);
    else
        sprintf(buffer, "%ld B", bytes);
}

// Helper: Get file size
long get_file_size(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1;
}

void printStats(char *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (!file)
    {
        printf("Error: Could not open file %s\n", fileName);
        return;
    }

    long original_size_bytes;
    double time_taken;
    int width, height;

    // 1. Read the Header to get stored stats
    if (fscanf(file, "%ld %lf %d %d", &original_size_bytes, &time_taken, &width, &height) != 4)
    {
        printf("Error: Invalid file format.\n");
        fclose(file);
        return;
    }
    fclose(file);

    // 2. Get Compressed Size (Actual size of the .gfw file)
    long compressed_size_bytes = get_file_size(fileName);

    // 3. Calculate Ratio
    // Ratio = (1 - (compressed / original)) * 100
    float ratio = 100.0 * (1.0 - ((float)compressed_size_bytes / (float)original_size_bytes));
    if (ratio < 0)
        ratio = 0; // handle edge case if compressed is larger

    // 4. Format Output
    char orig_str[20], comp_str[20];
    format_size(original_size_bytes, orig_str);
    format_size(compressed_size_bytes, comp_str);

    printf("Compression Ratio: %.0f%%\n", ratio);
    printf("Original Size:     %s\n", orig_str);
    printf("Compressed Size:   %s\n", comp_str);
    printf("Time Elapsed:      %.2fs\n", time_taken);
}