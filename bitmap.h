#ifndef BITMAP_H
#define BITMAP_H

typedef struct Pixel Pixel;

int writeBitmap(Pixel *pixels, int imageWidth, int imageHeight, const char *filename);

#endif /* BITMAP_H */