#include <math.h>

#include "bitmap.h"
#include "math.h"

const int kNumPixelRows = 256;
const int kNumPixelColumns = 256;

const int kNumSubPixelsDim = 3;
// The number of rays to cast per pixel.
const int kNumSubPixels = kNumSubPixelsDim * kNumSubPixelsDim;

// Our scene exists within a 1x1x1 cube centered at the origin.
const Point kCameraPosition = {.x = 0.0, .y = 0.0, .z = 0.5};
const Point kLightPosition = {.x = -0.2, .y = 0.2, .z = 0.5};

const float kAmbientColor = 0.2;
const float kMaxDiffuseColor = 0.7;
const float kSpecularColor = 0.04;
const float kSpecularCutoff = 0.99;
const int kMaxBrightness = 0xFF;

typedef struct Transform {
  float a, b, c;
  float d, e, f;
  float g, h, i;
} Transform;

Transform makeTransform(float a, float b, float c, float d, float e, float f,
                        float g, float h, float i) {
  return (Transform){
      .a = a, .b = b, .c = c, .d = d, .e = e, .f = f, .g = g, .h = h, .i = i};
}

Transform makeRotationX(float t) {
  return makeTransform(1.0, 0.0, 0.0, 0.0, cos(t), -sin(t), 0.0, sin(t),
                       cos(t));
};

Transform makeRotationZ(float t) {
  return makeTransform(cos(t), -sin(t), 0.0, sin(t), cos(t), 0.0, 0.0, 0.0,
                       1.0);
};

Transform makeRotationY(float t) {
  return makeTransform(cos(t), 0.0, sin(t), 0.0, 1.0, 0.0, -sin(t), 0, cos(t));
};

Vector applyTransform(Transform t, Vector v) {
  float x = t.a * v.x + t.b * v.y + t.c * v.z;
  float y = t.d * v.x + t.e * v.y + t.f * v.z;
  float z = t.g * v.x + t.h * v.y + t.i * v.z;
  return makeVector(x, y, z);
}

Transform combineTransforms(Transform t1, Transform t2) {
  float a = t1.a * t2.a + t1.b * t2.d + t1.c * t2.g;
  float b = t1.a * t2.b + t1.b * t2.e + t1.c * t2.h;
  float c = t1.a * t2.c + t1.b * t2.f + t1.c * t2.i;
  float d = t1.d * t2.a + t1.e * t2.d + t1.f * t2.g;
  float e = t1.d * t2.b + t1.e * t2.e + t1.f * t2.h;
  float f = t1.d * t2.c + t1.e * t2.f + t1.f * t2.i;
  float g = t1.g * t2.a + t1.h * t2.d + t1.i * t2.g;
  float h = t1.g * t2.b + t1.h * t2.e + t1.i * t2.h;
  float i = t1.g * t2.c + t1.h * t2.f + t1.i * t2.i;
  return makeTransform(a, b, c, d, e, f, g, h, i);
}

float sceneSDF(Point p) {
  Transform transform =
      combineTransforms(makeRotationY(0.7), makeRotationX(-0.3));

  Point txp = makePoint(p.x, p.y + 0.25, p.z + 1.0);
  Point tp =
      vectorToPoint(applyTransform(transform, vectorFromOriginToPoint(txp)));

  float result = 1.0;
  result = unionOp(result, sphereSDF(p, 0.1));
  result = unionOp(result, planeSDF(tp, makeVector(0.0, 0.0, 1.0), 0.0));
  result = unionOp(result, planeSDF(tp, makeVector(0.0, 1.0, 0.0), 0.0));
  result = unionOp(result, planeSDF(tp, makeVector(1.0, 0.0, 0.0), 0.0));
  return result;
}

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
        Point pixelPoint = makePoint(x, y, -0.5);

        Vector directionToPixel =
            directionFromPointToPoint(kCameraPosition, pixelPoint);
        Ray cameraToPixelRay = makeRay(kCameraPosition, directionToPixel);
        Point intPoint;
        if (!rayMarch(cameraToPixelRay, sceneSDF, &intPoint)) {
          continue;
        }
        Vector normal = normalForPointAndSDF(intPoint, sceneSDF);
        Vector intPointToLightDir =
            directionFromPointToPoint(intPoint, kLightPosition);

        // When raymarching from the intersection point to the shadow, we need
        // to start a little ways away from the intersection point so that we
        // don't just hit the same intersection point again.
        Point nearbyIntPoint =
            addVectorToPoint(intPoint, intPointToLightDir, 0.01);
        Ray intPointToLightRay = makeRay(nearbyIntPoint, intPointToLightDir);
        float shadow = 1.0 - (float)rayMarch(intPointToLightRay, sceneSDF, 0);

        // dp = 1.0 means the vectors have the same direction.
        // dp = -1.0 means the vectors have opposite directions.
        float dp = dotProduct(normal, intPointToLightDir);
        float specular = invLerp(dp, kSpecularCutoff, 1.0) * kSpecularColor;
        float diffuse = max(dp, 0.0) * kMaxDiffuseColor;
        float normalizedBrightness = kAmbientColor + diffuse + specular;
        colorSum += normalizedBrightness * kMaxBrightness * shadow;
      }

      int pixelIndex = pixelRow * kNumPixelRows + pixelColumn;
      float avgColor = colorSum / kNumSubPixels;
      pixels[pixelIndex] = makePixel(avgColor, avgColor, avgColor);
    }
  }
  return writeBitmap(pixels, kNumPixelRows, kNumPixelColumns, "image.bmp");
}