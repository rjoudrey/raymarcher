#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "bitmap.h"
#include "math.h"

#if HQ
const int kNumPixelRows = 1024;
const int kNumPixelColumns = 1024;
const int kNumSubPixelsDim = 1;
#else
const int kNumPixelRows = 256;
const int kNumPixelColumns = 256;
const int kNumSubPixelsDim = 4;
#endif

// The number of rays to cast per pixel.
const int kNumSubPixels = kNumSubPixelsDim * kNumSubPixelsDim;

// Our scene exists within a 1x1x1 cube centered at the origin.
const Point kCameraPosition = {.x = 0.0, .y = 0.0, .z = 0.5};
const Point kLightPosition = {.x = -0.2, .y = 0.2, .z = 0.5};

const int kMaxBrightness = 0xFF;

typedef struct Material {
  Color diffuse;
  bool isConductive;
  // How much a ray of light bends when passing from one medium to another.
  Color refractiveIndex;
  // The reduction in intensity as light propagates through the material due to
  // scattering of light and absorption.
  Color extinctionCoeff;
} Material;

typedef struct SDFResult {
  float distance;
  Material material;
} SDFResult;

#define kWhiteColor \
  (Color) { .r = 1.0, .g = 1.0, .b = 1.0 }
#define kGreyColor \
  (Color) { .r = 0.7, .g = 0.7, .b = 0.7 }

#define kGoldRefractiveIndex \
  { .r = 0.183, .g = 0.421, .b = 1.373 }
#define kGoldExtinctionCoeff \
  { .r = 3.424, .g = 2.346, .b = 1.77 }

#define kSilverRefractiveIndex \
  { .r = 0.159, .g = 0.145, .b = 0.135 }
#define kSilverExtinctionCoeff \
  { .r = 3.929, .g = 3.190, .b = 2.380 }

const Material kGoldMaterial = {.diffuse = kWhiteColor,
                                .isConductive = true,
                                .refractiveIndex = kGoldRefractiveIndex,
                                .extinctionCoeff = kGoldExtinctionCoeff};

const Material kSilverMaterial = {.diffuse = kWhiteColor,
                                  .isConductive = true,
                                  .refractiveIndex = kSilverRefractiveIndex,
                                  .extinctionCoeff = kSilverExtinctionCoeff};

const Material kPlaneMaterial = {.diffuse = kGreyColor,
                                 .isConductive = false,
                                 .refractiveIndex = kWhiteColor,
                                 .extinctionCoeff = kWhiteColor};

const Vector kIVector = {.x = 1.0, .y = 0.0, .z = 0.0};
const Vector kJVector = {.x = 0.0, .y = 1.0, .z = 0.0};
const Vector kKVector = {.x = 0.0, .y = 0.0, .z = 1.0};

SDFResult unionOp2(SDFResult result, float distance, Material material) {
  float unionDistance = unionOp(result.distance, distance);
  SDFResult otherResult = {.distance = distance, .material = material};
  return unionDistance == result.distance ? result : otherResult;
}

float planeSDF2(Point p, Vector n) { return planeSDF(p, n, 0.0); }

SDFResult sceneSDF2(Point p) {
  Transform planeTransform = makeRotationY(0.8);
  Point planeTx = makePoint(p.x, p.y + 0.25, p.z + 1.0);
  Point planeTx2 = vectorToPoint(
      applyTransform(planeTransform, vectorFromOriginToPoint(planeTx)));

  Point sphere1Tx = makePoint(p.x + 0.15, p.y - 0.2, p.z - 0.15);
  SDFResult result = {.distance = sphereSDF(sphere1Tx, 0.1),
                      .material = kGoldMaterial};

  Point sphere2Tx = makePoint(p.x - 0.2, p.y, p.z + 0);
  result = unionOp2(result, sphereSDF(sphere2Tx, 0.2), kSilverMaterial);
  result = unionOp2(result, planeSDF2(planeTx2, kIVector), kPlaneMaterial);
  result = unionOp2(result, planeSDF2(planeTx2, kJVector), kPlaneMaterial);
  result = unionOp2(result, planeSDF2(planeTx2, kKVector), kPlaneMaterial);
  return result;
}

float sceneSDF(Point p) { return sceneSDF2(p).distance; }

// Returns the amount of incoming light reflected from a conductive material in
// the range of [0.0, 1.0].
// https://www.pbr-book.org/3ed-2018/Reflection_Models/Specular_Reflection_and_Transmission
float conductiveReflectance(float refractiveIndex, float extinctionCoeff,
                            float angle) {
  float n = refractiveIndex;
  float k = extinctionCoeff;
  float t = angle;
  float rp = ((n * n + k * k) * cosf(t) * cosf(t) - 2.0 * n * cosf(t) + 1.0) /
             ((n * n + k * k) * cosf(t) * cosf(t) + 2.0 * n * cosf(t) + 1.0);
  float rs = ((n * n + k * k) - 2.0 * n * cosf(t) + cosf(t) * cosf(t)) /
             ((n * n + k * k) + 2.0 * n * cosf(t) + cosf(t) * cosf(t));
  return (rp + rs) * 0.5;
}

Color conductiveReflectance2(Color refractiveIndex, Color extinctionCoeff,
                             float angle) {
  float r = conductiveReflectance(refractiveIndex.r, extinctionCoeff.r, angle);
  float g = conductiveReflectance(refractiveIndex.g, extinctionCoeff.g, angle);
  float b = conductiveReflectance(refractiveIndex.b, extinctionCoeff.b, angle);
  return makeColor(r, g, b);
}

float angleBetweenVectors(Vector a, Vector b) {
  return acosf(dotProduct(a, b) / (vectorLength(a) * vectorLength(b)));
}

// The color of a point depends on the position of lights and the direction of
// the viewer, the material of the object at the point, etc.
Color pointColor(Point point, Vector direction, int numBounces) {
  Vector normal = normalForPointAndSDF(point, sceneSDF);
  Material material = sceneSDF2(point).material;
  if (!material.isConductive) {
    // When raymarching from the intersection point to the light, we need
    // to start a little ways away from the intersection point so that we
    // don't just hit the same intersection point again.
    Vector pointToLightDir = directionFromPointToPoint(point, kLightPosition);
    Point nearbyIntPoint = addVectorToPoint(point, pointToLightDir, 0.01);
    float shadow = lerp(
        softShadow(nearbyIntPoint, kLightPosition, sceneSDF, 8.0), 0.2, 1.0);
    // dp = 1.0 means the vectors have the same direction.
    // dp = -1.0 means the vectors have opposite directions.
    float dp = dotProduct(normal, pointToLightDir);
    float diffuseT = invLerp(dp, -1.0, 1.0) * shadow;
    return scaleColor(material.diffuse, diffuseT);
  }
  if (numBounces == 0) {
    // The material at this point is conductive, but we are out of bounces, so
    // just return something. If this happens then we need to do more bounces
    // for our specific scene.
    return kWhiteColor;
  }
  // Perform a ray march from the point in the direction of the reflection to
  // find the color of the object in the reflection.
  Vector reflectionDirection = subtractVectors(
      direction, scaleVector(normal, 2.0 * dotProduct(direction, normal)));
  Point nearbyIntPoint = addVectorToPoint(point, reflectionDirection, 0.001);
  Ray reflectionRay = makeRay(nearbyIntPoint, reflectionDirection);
  Point reflectIntPoint;
  if (!rayMarch(reflectionRay, sceneSDF, &reflectIntPoint)) {
    // The reflection hit nothing in our scene.
    return makeColor(0.0, 0.0, 0.0);
  }
  Color colorInReflection =
      pointColor(reflectIntPoint, direction, numBounces - 1);
  float lightAngle =
      angleBetweenVectors(scaleVector(direction, -1.0), normal) * 0.5;
  Color reflectance = conductiveReflectance2(
      material.refractiveIndex, material.extinctionCoeff, lightAngle);
  return multiplyColors(colorInReflection, reflectance);
}

Color pixelColor(Point point) {
  Vector cameraToPointDir = directionFromPointToPoint(kCameraPosition, point);
  Ray cameraToPixelRay = makeRay(kCameraPosition, cameraToPointDir);
  Point intPoint;
  if (!rayMarch(cameraToPixelRay, sceneSDF, &intPoint)) {
    return makeColor(0.0, 0.0, 0.0);
  }
  return pointColor(intPoint, cameraToPointDir, 64);
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