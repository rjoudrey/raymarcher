#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

typedef struct Pixel {
  uint8_t b;
  uint8_t g;
  uint8_t r;
} Pixel;

int writeBitmap(Pixel *pixels, int imageWidth, int imageHeight,
                const char *filename);

#endif /* BITMAP_H */