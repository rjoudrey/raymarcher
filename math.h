#ifndef MATH_H
#define MATH_H

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

Point makePoint(float x, float y, float z);

Vector makeVector(float x, float y, float z);

Ray makeRay(Point origin, Vector direction);

float min(float a, float b);

float max(float a, float b);

float clamp(float t, float lowerBound, float upperBound);

float lerp(float t, float lowerBound, float upperBound);

float invLerp(float t, float lowerBound, float upperBound);

float vectorLength(Vector vector);

float dotProduct(Vector a, Vector b);

Vector normalizedVector(Vector vector);

Vector vectorFromPointToPoint(Point start, Point end);

Vector vectorFromOriginToPoint(Point point);

Point vectorToPoint(Vector vector);

Vector directionFromPointToPoint(Point start, Point end);

Vector directionFromPointToPoint(Point start, Point end);

float pointDistanceFromOrigin(Point target);

// Travels from the ray's origin in the ray's direction, passing the current
// point into the specified SDF function. Returns 1 if an intersection was
// found, and returns the intersection point.
typedef float (*SDF)(Point);
int rayMarch(Ray ray, SDF SDF, Point *intersectionPoint);

// Finds the normal for a point. This is a implemented using the generic
// approach of integrating over the signed distance function.
Vector normalForPointAndSDF(Point p, SDF SDF);

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