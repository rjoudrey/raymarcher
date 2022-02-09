#include "bitmap.h"
#include "math.h"

const int kNumPixelRows = 256;
const int kNumPixelColumns = 256;

const int kNumSubPixelsDim = 3;
// The number of rays to cast per pixel.
const int kNumSubPixels = kNumSubPixelsDim * kNumSubPixelsDim;

// Our scene exists within a 1x1x1 cube centered at the origin.
const Point kCameraPosition = {.x = 0.0, .y = 0.0, .z = 0.5};
const Vector kCameraDirection = {.x = 0.0, .y = 0.0, .z = -1.0};
const Point kLightPosition = {.x = -0.2, .y = 0.2, .z = 0.5};

const float kSphereRadius = 0.2;
const float kAmbientColor = 0.2;
const float kMaxDiffuseColor = 0.7;
const float kSpecularColor = 0.04;
const float kSpecularCutoff = 0.99;
const int kMaxBrightness = 0xFF;

int main() {
  Pixel pixels[kNumPixelRows * kNumPixelColumns];
  for (int pixelRow = 0; pixelRow < kNumPixelRows; ++pixelRow) {
    for (int pixelColumn = 0; pixelColumn < kNumPixelColumns; ++pixelColumn) {
      float colorSum = 0.0;

      for (int subPixel = 0; subPixel < kNumSubPixels; ++subPixel) {
        // Adjust pixelRow and pixelColumn to account for subsampling.
        float offset = 1.0 / kNumSubPixelsDim / 2.0;
        float subPixelRowOffset =
            (subPixel / kNumSubPixelsDim) / (float)kNumSubPixelsDim + offset;
        float subPixelColumnOffset =
            (subPixel % kNumSubPixelsDim) / (float)kNumSubPixelsDim + offset;
        float pixelRowAdjusted = pixelRow + subPixelRowOffset;
        float pixelColumnAdjusted = pixelColumn + subPixelColumnOffset;

        // Convert from ([0, numPixelRows], [0, numPixelColumns]) to
        // ([0.0, 1.0], [0.0, 1.0]), and then to ([-0.5, 0.5], [-0.5, 0.5])
        float x = lerp(pixelColumnAdjusted / kNumPixelColumns, -0.5, 0.5);
        float y = lerp(pixelRowAdjusted / kNumPixelRows, -0.5, 0.5);
        Point pixelPoint = {.x = x, .y = y, .z = -0.5};

        Vector directionToPixel =
            directionFromPointToPoint(kCameraPosition, pixelPoint);
        Ray ray = {.origin = kCameraPosition, .direction = directionToPixel};
        Point intersectionPoint;
        if (!rayIntersectsSphere(ray, kSphereRadius, &intersectionPoint)) {
          continue;
        }
        Vector normal = sphereNormal(intersectionPoint, kSphereRadius);
        Vector intersectionPointToLight = normalizedVector(
            (Vector){.x = kLightPosition.x - intersectionPoint.x,
                     .y = kLightPosition.y - intersectionPoint.y,
                     .z = kLightPosition.z - intersectionPoint.z});

        // dp = 1.0 means the vectors have the same direction.
        // dp = -1.0 means the vectors have opposite directions.
        float dp = dotProduct(normal, intersectionPointToLight);
        float specular = invLerp(dp, kSpecularCutoff, 1.0) * kSpecularColor;
        float diffuse = max(dp, 0.0) * kMaxDiffuseColor;
        float normalizedBrightness = kAmbientColor + diffuse + specular;
        colorSum += normalizedBrightness * kMaxBrightness;
      }

      int pixelIndex = pixelRow * kNumPixelRows + pixelColumn;
      float avgColor = colorSum / kNumSubPixels;
      pixels[pixelIndex] = (Pixel){.r = avgColor, .g = avgColor, .b = avgColor};
    }
  }
  return writeBitmap(pixels, kNumPixelRows, kNumPixelColumns, "image.bmp");
}