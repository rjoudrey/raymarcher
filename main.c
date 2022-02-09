#include "bitmap.h"
#include "math.h"

const int kNumPixelRows = 256;
const int kNumPixelColumns = 256;

// Our scene exists within a 1x1x1 cube centered at the origin.
const Point kCameraPosition = {.x = 0.0, .y = 0.0, .z = 0.5};
const Vector kCameraDirection = {.x = 0.0, .y = 0.0, .z = -1.0};
const Point kLightPosition = {.x = -0.2, .y = 0.2, .z = 0.5};

const float kPixelWidth = 1.0;
const float kPixelHeight = 1.0;

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
      // Convert from ([0, numPixelRows], [0, numPixelColumns]) to ([0.0, 1.0],
      // [0.0, 1.0]), and then to ([-0.5, 0.5], [-0.5, 0.5])
      float x = lerp(invLerp(pixelColumn, 0, kNumPixelColumns), -0.5, 0.5);
      float y = lerp(invLerp(pixelRow, 0, kNumPixelRows), -0.5, 0.5);
      Point pixelPoint = {.x = x, .y = y, .z = -0.5};
      Vector directionToPixel =
          directionFromPointToPoint(kCameraPosition, pixelPoint);
      Ray ray = {.origin = kCameraPosition, .direction = directionToPixel};
      int pixelIndex = pixelRow * kNumPixelRows + pixelColumn;
      Point intersectionPoint;
      if (!rayIntersectsSphere(ray, kSphereRadius, &intersectionPoint)) {
        pixels[pixelIndex] = (Pixel){.r = 0x0, .g = 0x0, .b = 0x0};
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
      float color = normalizedBrightness * kMaxBrightness;
      pixels[pixelIndex] = (Pixel){.r = color, .g = color, .b = color};
    }
  }
  return writeBitmap(pixels, kNumPixelRows, kNumPixelColumns, "image.bmp");
}