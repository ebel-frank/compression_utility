#include "../include/bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

BMPResult readImageFile(const char *filename)
{
    BMPResult result;

    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf("Error: Could not open file %s\n", filename);
        exit(1);
        // return;
    }

    // Read File Header
    BMPFileHeader fileHeader;
    if (fread(&fileHeader, sizeof(BMPFileHeader), 1, file) != 1)
    {
        printf("Error: Could not read file header\n");
        fclose(file);
        exit(1);
        // return;
    }

    // Verify BMP signature
    if (fileHeader.signature != 0x4D42)
    { // "BM" in little-endian
        printf("Error: Not a valid BMP file\n");
        fclose(file);
        exit(1);
        // return;
    }

    // Read Info Header
    BMPInfoHeader *infoHeader = malloc(sizeof(BMPInfoHeader));
    if (fread(infoHeader, sizeof(BMPInfoHeader), 1, file) != 1)
    {
        printf("Error: Could not read info header\n");
        fclose(file);
        exit(1);
        // return;
    }

    // Print File Header
    printf("=== BMP FILE HEADER ===\n");
    printf("Signature: %c%c\n", fileHeader.signature & 0xFF,
           (fileHeader.signature >> 8) & 0xFF);
    printf("File Size: %u bytes\n", fileHeader.fileSize);
    printf("Reserved1: %u\n", fileHeader.reserved1);
    printf("Reserved2: %u\n", fileHeader.reserved2);
    printf("Data Offset: %u bytes\n\n", fileHeader.dataOffset);

    // Print Info Header
    printf("=== BMP INFO HEADER ===\n");
    printf("Header Size: %u bytes\n", infoHeader->headerSize);
    printf("Width: %d pixels\n", infoHeader->width);
    printf("Height: %d pixels\n", infoHeader->height);
    printf("Color Planes: %u\n", infoHeader->planes);
    printf("Bits Per Pixel: %u\n", infoHeader->bitsPerPixel);
    printf("Compression: %u\n", infoHeader->compression);
    printf("Image Size: %u bytes\n", infoHeader->imageSize);
    printf("X Pixels Per Meter: %d\n", infoHeader->xPixelsPerMeter);
    printf("Y Pixels Per Meter: %d\n", infoHeader->yPixelsPerMeter);
    printf("Colors Used: %u\n", infoHeader->colorsUsed);
    printf("Important Colors: %u\n\n", infoHeader->importantColors);

    // Calculate row size with padding
    int rowSize = ((infoHeader->bitsPerPixel * infoHeader->width + 31) / 32) * 4;
    int padding = rowSize - (infoHeader->width * infoHeader->bitsPerPixel / 8);

    printf("=== PIXEL DATA INFO ===\n");
    printf("Row Size (with padding): %d bytes\n", rowSize);
    printf("Padding per row: %d bytes\n\n", padding);

    // Allocate memory for one row
    uint8_t *rowData = (uint8_t *)malloc(rowSize);
    // 2d array of image data
    if (!rowData)
    {
        printf("Error: Could not allocate memory\n");
        fclose(file);
        exit(1);
        // return;
    }

    uint8_t **image = (uint8_t **)malloc(infoHeader->height * sizeof(uint8_t *));
    if (!image)
    {
        printf("Error: Could not allocate memory for image\n");
        fclose(file);
        exit(1);
        // return;
    }
    // initialize image data
    for (int i = 0; i < infoHeader->height; i++)
    {
        image[i] = (uint8_t *)malloc(infoHeader->width * sizeof(uint8_t));
    }

    // Seek to pixel data
    fseek(file, fileHeader.dataOffset, SEEK_SET);

    uint8_t temp = 0;

    // Read and print first 5 rows of pixel data
    printf("First 5 rows of pixel data (in hex):\n");
    for (int i = 0; i < infoHeader->height; i++)
    {
        // // if (fread(rowData, 1, rowSize, file) != rowSize)
        // if (fread(rowData, 1, rowSize, file) != (size_t)rowSize)
        // {
        //     printf("Error: Could not read pixel data\n");
        //     break;
        // }
        // printf("Row %d: \n", i);
        // printf("Row data: %hhn\n", rowData);

        // printf("reading...\n");
        for (int j = 0; j < infoHeader->width; j++)
        {
            // printf("printing out..\n");
            // printf("%02X \n", rowData[j]);
            //  temp = temp & 0x0000FF;
            //  image[i][j] = temp;
            // image[i][j] = rowData[j];

            fread(&temp, 3, 1, file);

            // the Image is a
            // 24-bit BMP Image
            temp = temp & 0x0000FF;
            image[i][j] = temp;
        }
        printf("\n");
    }

    result.header = infoHeader;
    result.imageData = image;
    free(rowData);
    fclose(file);
    return result;
}