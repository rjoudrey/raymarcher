#include "bitmap.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef struct {
  uint16_t magic;
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t pixelsOffset;
} BitmapFileHeader;
#pragma pack(pop)

typedef struct {
  uint32_t headerSize;
  uint32_t bitmapWidth;
  uint32_t bitmapHeight;
  uint16_t numColorPlanes;
  uint16_t numBitsPerPixel;
  uint32_t compressionMethod;
  uint32_t imageSize;
  uint32_t horizontalResolution;
  uint32_t verticalResolution;
  uint32_t numColorPaletteColors;
  uint32_t numImportantColorsUsed;
} BitmapInfoHeader;

Pixel makePixel(uint8_t r, uint8_t g, uint8_t b) {
  return (Pixel){.r = r, .g = g, .b = b};
}

int writeBitmap(Pixel *pixels, int imageWidth, int imageHeight,
                const char *filename) {
  int numPixels = imageWidth * imageHeight;

  BitmapFileHeader header;
  header.magic = *((uint16_t *)"BM");
  header.fileSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) +
                    sizeof(Pixel) * numPixels;
  header.reserved1 = 0;
  header.reserved2 = 0;
  header.pixelsOffset = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

  BitmapInfoHeader coreHeader;
  coreHeader.headerSize = sizeof(BitmapInfoHeader);
  coreHeader.bitmapWidth = imageWidth;
  coreHeader.bitmapHeight = imageHeight;
  coreHeader.numColorPlanes = 1;
  coreHeader.numBitsPerPixel = sizeof(Pixel) * CHAR_BIT;
  coreHeader.compressionMethod = 0;
  coreHeader.imageSize = sizeof(Pixel) * numPixels;
  coreHeader.horizontalResolution = 0;
  coreHeader.verticalResolution = 0;
  coreHeader.numColorPaletteColors = 0;
  coreHeader.numImportantColorsUsed = 0;

  FILE *imageFile = fopen(filename, "w+");
  if (errno != 0) {
    printf("Failed to open image file: %d", errno);
    return 1;
  }

  fwrite(&header, sizeof(BitmapFileHeader), 1, imageFile);
  fwrite(&coreHeader, sizeof(BitmapInfoHeader), 1, imageFile);
  fwrite(pixels, sizeof(Pixel), numPixels, imageFile);
  fclose(imageFile);

  return 0;
}