#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

#pragma pack(push, 1)  // Ensure no padding
typedef struct {
    uint16_t signature;    // "BM"
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
} BMPFileHeader;

typedef struct {
    uint32_t headerSize;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t  xPixelsPerMeter;
    int32_t  yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t importantColors;
} BMPInfoHeader;
#pragma pack(pop)

void readImageFile(const char* filename);

#endif