#include "math.h"

#include <math.h>
#include <stdint.h>

float min(float a, float b) { return a < b ? a : b; }

float max(float a, float b) { return a > b ? a : b; }

float clamp(float t, float lowerBound, float upperBound) {
  return min(max(t, lowerBound), upperBound);
}

float lerp(float t, float lowerBound, float upperBound) {
  return lowerBound + (upperBound - lowerBound) * clamp(t, 0.0, 1.0);
}

float invLerp(float t, float lowerBound, float upperBound) {
  return clamp((t - lowerBound) / (upperBound - lowerBound), 0.0, 1.0);
}

float vectorLength(Vector vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

float dotProduct(Vector a, Vector b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector normalizedVector(Vector vector) {
  float length = vectorLength(vector);
  return (Vector){
      .x = vector.x / length, .y = vector.y / length, .z = vector.z / length};
}

Vector directionFromPointToPoint(Point start, Point end) {
  return normalizedVector((Vector){
      .x = end.x - start.x,
      .y = end.y - start.y,
      .z = end.z - start.z,
  });
}

float pointDistanceFromOrigin(Point target) {
  Vector v = {.x = target.x, .y = target.y, .z = target.z};
  return vectorLength(v);
}

// The signed distance function for a sphere with the specified radius.
// https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sphereSDF(Point p, float radius) {
  return pointDistanceFromOrigin(p) - radius;
}

#define SDF_EPSILON 0.0001

// This function checks whether a ray intersects with a sphere at the origin
// with the specified radius, and returns the intersection point through a
// pointer. This is implemented via ray marching and signed distance functions.
int rayIntersectsSphere(Ray ray, float radius, Point *intersectionPoint) {
  Point point = ray.origin;
  for (int i = 0; i < 64; ++i) {
    float d = sphereSDF(point, radius);
    if (d <= SDF_EPSILON) {
      if (intersectionPoint) {
        *intersectionPoint = point;
      }
      return 1;
    }
    point = (Point){.x = point.x + ray.direction.x * d,
                    .y = point.y + ray.direction.y * d,
                    .z = point.z + ray.direction.z * d};
  }
  return 0;
}

// This function returns the normal associated with a particular point on the
// sphere. This is a implemented using the generic approach of integrating over
// the signed distance function.
Vector sphereNormal(Point p, float radius) {
  float x =
      sphereSDF((Point){.x = p.x + SDF_EPSILON, .y = p.y, .z = p.z}, radius) -
      sphereSDF((Point){.x = p.x - SDF_EPSILON, .y = p.y, .z = p.z}, radius);
  float y =
      sphereSDF((Point){.x = p.x, .y = p.y + SDF_EPSILON, .z = p.z}, radius) -
      sphereSDF((Point){.x = p.x, .y = p.y - SDF_EPSILON, .z = p.z}, radius);
  float z =
      sphereSDF((Point){.x = p.x, .y = p.y, .z = p.z + SDF_EPSILON}, radius) -
      sphereSDF((Point){.x = p.x, .y = p.y, .z = p.z - SDF_EPSILON}, radius);
  return normalizedVector((Vector){.x = x, .y = y, .z = z});
}