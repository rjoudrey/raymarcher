#include <math.h>
#include <stdbool.h>

#include "bitmap.h"
#include "math.h"

#if HQ
const int kNumPixelRows = 1024;
const int kNumPixelColumns = 1024;
const int kNumSubPixelsDim = 4;
#else
const int kNumPixelRows = 256;
const int kNumPixelColumns = 256;
const int kNumSubPixelsDim = 1;
#endif

// The number of rays to cast per pixel.
const int kNumSubPixels = kNumSubPixelsDim * kNumSubPixelsDim;

// Our scene exists within a 1x1x1 cube centered at the origin.
const Point kCameraPosition = {.x = 0.0, .y = 0.0, .z = 0.5};
const Point kLightPosition = {.x = -0.2, .y = 0.2, .z = 0.5};

const int kMaxBrightness = 0xFF;

float sceneSDF(Point p) {
  Transform transform = makeRotationY(0.8);
  Point planeTx = makePoint(p.x, p.y + 0.25, p.z + 1.0);
  Point planeTx2 = vectorToPoint(
      applyTransform(transform, vectorFromOriginToPoint(planeTx)));

  float result = 1.0;
  result = unionOp(result, sphereSDF(p, 0.2));
  result = unionOp(result, planeSDF(planeTx2, makeVector(0.0, 0.0, 1.0), 0.0));
  result = unionOp(result, planeSDF(planeTx2, makeVector(0.0, 1.0, 0.0), 0.0));
  result = unionOp(result, planeSDF(planeTx2, makeVector(1.0, 0.0, 0.0), 0.0));
  return result;
}

// The color of a point depends on the position of lights and the direction of
// the viewer.
Color pointColor(Point point, Vector direction, bool reflect) {
  Vector normal = normalForPointAndSDF(point, sceneSDF);

  // For reflective materials, perform a ray march from the point in the
  // direction of the reflection to find the color of the object in the
  // reflection.
  Color reflectionColor = makeColor(0.0, 0.0, 0.0);
  if (reflect) {
    Vector reflectionDirection = vectorSubtract(
        direction, scaledVector(normal, 2.0 * dotProduct(direction, normal)));
    Point nearbyIntPoint = addVectorToPoint(point, reflectionDirection, 0.001);
    Ray reflectionRay = makeRay(nearbyIntPoint, reflectionDirection);
    Point reflectIntPoint;
    if (rayMarch(reflectionRay, sceneSDF, &reflectIntPoint)) {
      // We can pass any direction since it is unused when passing false.
      reflectionColor = pointColor(reflectIntPoint, direction, false);
    }
  }

  // When raymarching from the intersection point to the light, we need
  // to start a little ways away from the intersection point so that we
  // don't just hit the same intersection point again.
  Vector pointToLightDir = directionFromPointToPoint(point, kLightPosition);
  Point nearbyIntPoint = addVectorToPoint(point, pointToLightDir, 0.001);
  float shadow =
      lerp(softShadow(nearbyIntPoint, kLightPosition, sceneSDF, 4.0), 0.2, 1.0);

  // dp = 1.0 means the vectors have the same direction.
  // dp = -1.0 means the vectors have opposite directions.
  float dp = dotProduct(normal, pointToLightDir);
  float diffuseT = invLerp(dp, -1.0, 1.0) * shadow;
  Color diffuseColor = makeColor(diffuseT, diffuseT, diffuseT);
  if (!reflect) {
    return diffuseColor;
  }
  return mixColors(diffuseColor, reflectionColor, 0.8, 0.2);
}

Color pixelColor(Point point) {
  Vector cameraToPointDir = directionFromPointToPoint(kCameraPosition, point);
  Ray cameraToPixelRay = makeRay(kCameraPosition, cameraToPointDir);
  Point intPoint;
  if (!rayMarch(cameraToPixelRay, sceneSDF, &intPoint)) {
    return makeColor(0.0, 0.0, 0.0);
  }
  return pointColor(intPoint, cameraToPointDir, true);
}

int main() {
  Pixel pixels[kNumPixelRows * kNumPixelColumns];
  for (int pixelRow = 0; pixelRow < kNumPixelRows; ++pixelRow) {
    for (int pixelColumn = 0; pixelColumn < kNumPixelColumns; ++pixelColumn) {
      Color colorSum = makeColor(0.0, 0.0, 0.0);

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
        Point subPixelPoint = makePoint(x, y, 0.0);
        Color subPixelColor = pixelColor(subPixelPoint);
        colorSum = addColors(colorSum, subPixelColor);
      }

      int pixelIndex = pixelRow * kNumPixelRows + pixelColumn;
      Color avgColor = scaleColor(colorSum, 0xFF / kNumSubPixels);
      pixels[pixelIndex] = makePixel(avgColor.r, avgColor.g, avgColor.b);
    }
  }
  return writeBitmap(pixels, kNumPixelRows, kNumPixelColumns, "image.bmp");
}