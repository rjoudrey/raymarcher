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

float min(float a, float b);

float max(float a, float b);

float clamp(float t, float lowerBound, float upperBound);

float lerp(float t, float lowerBound, float upperBound);

float invLerp(float t, float lowerBound, float upperBound);

float vectorLength(Vector vector);

float dotProduct(Vector a, Vector b);

Vector normalizedVector(Vector vector);

Vector directionFromPointToPoint(Point start, Point end);

float pointDistanceFromOrigin(Point target);

// The signed distance function for a sphere with the specified radius.
float sphereSDF(Point p, float radius);

// This function checks whether a ray intersects with a sphere at the origin
// with the specified radius, and returns the intersection point through a
// pointer. This is implemented via ray marching and signed distance functions.
int rayIntersectsSphere(Ray ray, float radius, Point *intersectionPoint);

// This function returns the normal associated with a particular point on the
// sphere. This is a implemented using the generic approach of integrating over
// the signed distance function.
Vector sphereNormal(Point p, float radius);

#endif /* MATH_H */