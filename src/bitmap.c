#include "../include/bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void readImageFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    // Read File Header
    BMPFileHeader fileHeader;
    if (fread(&fileHeader, sizeof(BMPFileHeader), 1, file) != 1) {
        printf("Error: Could not read file header\n");
        fclose(file);
        return;
    }

    // Verify BMP signature
    if (fileHeader.signature != 0x4D42) { // "BM" in little-endian
        printf("Error: Not a valid BMP file\n");
        fclose(file);
        return;
    }

    // Read Info Header
    BMPInfoHeader infoHeader;
    if (fread(&infoHeader, sizeof(BMPInfoHeader), 1, file) != 1) {
        printf("Error: Could not read info header\n");
        fclose(file);
        return;
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
    printf("Header Size: %u bytes\n", infoHeader.headerSize);
    printf("Width: %d pixels\n", infoHeader.width);
    printf("Height: %d pixels\n", infoHeader.height);
    printf("Color Planes: %u\n", infoHeader.planes);
    printf("Bits Per Pixel: %u\n", infoHeader.bitsPerPixel);
    printf("Compression: %u\n", infoHeader.compression);
    printf("Image Size: %u bytes\n", infoHeader.imageSize);
    printf("X Pixels Per Meter: %d\n", infoHeader.xPixelsPerMeter);
    printf("Y Pixels Per Meter: %d\n", infoHeader.yPixelsPerMeter);
    printf("Colors Used: %u\n", infoHeader.colorsUsed);
    printf("Important Colors: %u\n\n", infoHeader.importantColors);

    // Calculate row size with padding
    int rowSize = ((infoHeader.bitsPerPixel * infoHeader.width + 31) / 32) * 4;
    int padding = rowSize - (infoHeader.width * infoHeader.bitsPerPixel / 8);

    printf("=== PIXEL DATA INFO ===\n");
    printf("Row Size (with padding): %d bytes\n", rowSize);
    printf("Padding per row: %d bytes\n\n", padding);

    // Allocate memory for one row
    uint8_t *rowData = (uint8_t*)malloc(rowSize);
    if (!rowData) {
        printf("Error: Could not allocate memory\n");
        fclose(file);
        return;
    }

    // Seek to pixel data
    fseek(file, fileHeader.dataOffset, SEEK_SET);


    // Read and print first 5 rows of pixel data
    printf("First 5 rows of pixel data (in hex):\n");
    for (int i = 0; i < 5 && i < infoHeader.height; i++) {
        if (fread(rowData, 1, rowSize, file) != rowSize) {
            printf("Error: Could not read pixel data\n");
            break;
        }
        printf("Row %d: ", i);
        for (int j = 0; j < rowSize; j++) {
            printf("%02X ", rowData[j]);
        }
        printf("\n");
    }

    free(rowData);
    fclose(file);
}