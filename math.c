#include "math.h"

#include <math.h>
#include <stdint.h>

#define SDF_EPSILON 0.0001

const Point kPointOrigin = {.x = 0.0, .y = 0.0, .z = 0.0};

Point makePoint(float x, float y, float z) {
  return (Point){.x = x, .y = y, .z = z};
}

Vector makeVector(float x, float y, float z) {
  return (Vector){.x = x, .y = y, .z = z};
}

Ray makeRay(Point origin, Vector direction) {
  return (Ray){.origin = origin, .direction = direction};
}

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

Vector vectorFromPointToPoint(Point start, Point end) {
  return (Vector){
      .x = end.x - start.x,
      .y = end.y - start.y,
      .z = end.z - start.z,
  };
}

Vector vectorFromOriginToPoint(Point point) {
  return vectorFromPointToPoint(kPointOrigin, point);
}

Point vectorToPoint(Vector vector) {
  return makePoint(vector.x, vector.y, vector.z);
}

Vector directionFromPointToPoint(Point start, Point end) {
  return normalizedVector(vectorFromPointToPoint(start, end));
}

float pointDistanceFromOrigin(Point target) {
  return vectorLength(vectorFromOriginToPoint(target));
}

Point addVectorToPoint(Point p, Vector v, float t) {
  return makePoint(p.x + v.x * t, p.y + v.y * t, p.z + v.z * t);
}

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

int rayMarch(Ray ray, SDF SDF, Point *intersectionPoint) {
  Point point = ray.origin;
  for (int i = 0; i < 64; ++i) {
    float d = SDF(point);
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

Vector normalForPointAndSDF(Point p, SDF SDF) {
  float x = SDF((Point){.x = p.x + SDF_EPSILON, .y = p.y, .z = p.z}) -
            SDF((Point){.x = p.x - SDF_EPSILON, .y = p.y, .z = p.z});
  float y = SDF((Point){.x = p.x, .y = p.y + SDF_EPSILON, .z = p.z}) -
            SDF((Point){.x = p.x, .y = p.y - SDF_EPSILON, .z = p.z});
  float z = SDF((Point){.x = p.x, .y = p.y, .z = p.z + SDF_EPSILON}) -
            SDF((Point){.x = p.x, .y = p.y, .z = p.z - SDF_EPSILON});
  return normalizedVector((Vector){.x = x, .y = y, .z = z});
}

float unionOp(float v1, float v2) { return min(v1, v2); }

float sphereSDF(Point p, float radius) {
  return pointDistanceFromOrigin(p) - radius;
}

float planeSDF(Point p, Vector normal, float h) {
  return dotProduct(vectorFromOriginToPoint(p), normal) + h;
}