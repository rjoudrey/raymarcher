#include <stdlib.h>

#include "pixel.h"
#include "bitmap.h"

int main() {
    const int kImageWidth = 256;
    const int kImageHeight = 256;

    Pixel *pixels = malloc(sizeof(Pixel) * kImageWidth * kImageHeight);
    for (int lineIndex = 0; lineIndex < kImageHeight; ++lineIndex) {
        for (int columnIndex = 0; columnIndex < kImageWidth; ++columnIndex) {
            Pixel pixel = { .r=0xFF, .g=0xFF, .b=0xFF  };
            pixels[lineIndex * kImageWidth + columnIndex] = pixel;
        }
    }
    int status = writeBitmap(pixels, kImageWidth, kImageHeight, "image.bmp");
    free(pixels);
    return status;
}