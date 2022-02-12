#ifndef MATH_H
#define MATH_H

#include <stdbool.h>

typedef struct Point {
  float x, y, z;
} Point;

typedef struct Vector {
  float x, y, z;
} Vector;

typedef struct Ray {
  Point origin;
  Vector direction;
} Ray;

typedef struct Transform {
  float a, b, c;
  float d, e, f;
  float g, h, i;
} Transform;

typedef struct Color {
  float r, g, b;
} Color;

Point makePoint(float x, float y, float z);

Vector makeVector(float x, float y, float z);

Ray makeRay(Point origin, Vector direction);

Color makeColor(float r, float g, float b);

float min(float a, float b);

float max(float a, float b);

float clamp(float t, float lowerBound, float upperBound);

float lerp(float t, float lowerBound, float upperBound);

float invLerp(float t, float lowerBound, float upperBound);

float vectorLength(Vector vector);

float dotProduct(Vector a, Vector b);

Vector normalizedVector(Vector vector);

Vector scaledVector(Vector v, float t);

Vector vectorSubtract(Vector a, Vector b);

Vector vectorFromPointToPoint(Point start, Point end);

Vector vectorFromOriginToPoint(Point point);

Vector directionFromPointToPoint(Point start, Point end);

Point vectorToPoint(Vector vector);

Point addVectorToPoint(Point p, Vector v, float t);

float pointDistanceFromOrigin(Point target);

Transform makeTransform(float a, float b, float c, float d, float e, float f,
                        float g, float h, float i);

Transform makeRotationX(float t);

Transform makeRotationZ(float t);

Transform makeRotationY(float t);

Vector applyTransform(Transform t, Vector v);

Transform combineTransforms(Transform t1, Transform t2);

Color mixColors(Color c1, Color c2, float t1, float t2);

Color addColors(Color c1, Color c2);

Color scaleColor(Color c, float t);

// Travels from the ray's origin in the ray's direction, passing the current
// point into the specified SDF function. Returns 1 if an intersection was
// found, and returns the intersection point.
typedef float (*SDF)(Point);
bool rayMarch(Ray ray, SDF SDF, Point *intersectionPoint);

// Finds the normal for a point. This is a implemented using the generic
// approach of integrating over the signed distance function.
Vector normalForPointAndSDF(Point p, SDF SDF);

float softShadow(Point start, Point end, SDF SDF, float k);

/**
 * Signed distance functions. More info at:
 * https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
 */

// Returns the union of two signed distance function values.
float unionOp(float v1, float v2);

// The signed distance function for a sphere with the specified radius.
float sphereSDF(Point p, float radius);

// The signed distance function for a plane with the specified normal and
// h-value.
float planeSDF(Point p, Vector normal, float h);

#endif /* MATH_H */