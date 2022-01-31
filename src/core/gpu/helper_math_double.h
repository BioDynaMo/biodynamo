// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_GPU_HELPER_MATH_DOUBLE_H_
#define CORE_GPU_HELPER_MATH_DOUBLE_H_

#ifdef __CUDACC__

inline double fminf(double a, double b) { return a < b ? a : b; }

inline double fmaxf(double a, double b) { return a > b ? a : b; }

inline __device__ double rsqrtf(double x) { return 1.0f / sqrtf(x); }

////////////////////////////////////////////////////////////////////////////////
// constructors
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 make_double2(double s) { return make_double2(s, s); }
inline __device__ double2 make_double2(double3 a) {
  return make_double2(a.x, a.y);
}
inline __device__ double2 make_double2(int2 a) {
  return make_double2(double(a.x), double(a.y));
}
inline __device__ double2 make_double2(uint2 a) {
  return make_double2(double(a.x), double(a.y));
}

inline __device__ int2 make_int2(double2 a) {
  return make_int2(int(a.x), int(a.y));
}

inline __device__ double3 make_double3(double s) {
  return make_double3(s, s, s);
}
inline __device__ double3 make_double3(double2 a) {
  return make_double3(a.x, a.y, 0.0f);
}
inline __device__ double3 make_double3(double2 a, double s) {
  return make_double3(a.x, a.y, s);
}
inline __device__ double3 make_double3(double4 a) {
  return make_double3(a.x, a.y, a.z);
}
inline __device__ double3 make_double3(int3 a) {
  return make_double3(double(a.x), double(a.y), double(a.z));
}
inline __device__ double3 make_double3(uint3 a) {
  return make_double3(double(a.x), double(a.y), double(a.z));
}

inline __device__ int3 make_int3(double3 a) {
  return make_int3(int(a.x), int(a.y), int(a.z));
}

inline __device__ double4 make_double4(double s) {
  return make_double4(s, s, s, s);
}
inline __device__ double4 make_double4(double3 a) {
  return make_double4(a.x, a.y, a.z, 0.0f);
}
inline __device__ double4 make_double4(double3 a, double w) {
  return make_double4(a.x, a.y, a.z, w);
}
inline __device__ double4 make_double4(int4 a) {
  return make_double4(double(a.x), double(a.y), double(a.z), double(a.w));
}
inline __device__ double4 make_double4(uint4 a) {
  return make_double4(double(a.x), double(a.y), double(a.z), double(a.w));
}

inline __device__ int4 make_int4(double4 a) {
  return make_int4(int(a.x), int(a.y), int(a.z), int(a.w));
}

////////////////////////////////////////////////////////////////////////////////
// negate
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 operator-(double2 &a) {
  return make_double2(-a.x, -a.y);
}
inline __device__ double3 operator-(double3 &a) {
  return make_double3(-a.x, -a.y, -a.z);
}
inline __device__ double4 operator-(double4 &a) {
  return make_double4(-a.x, -a.y, -a.z, -a.w);
}

////////////////////////////////////////////////////////////////////////////////
// addition
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 operator+(double2 a, double2 b) {
  return make_double2(a.x + b.x, a.y + b.y);
}
inline __device__ void operator+=(double2 &a, double2 b) {
  a.x += b.x;
  a.y += b.y;
}
inline __device__ double2 operator+(double2 a, double b) {
  return make_double2(a.x + b, a.y + b);
}
inline __device__ double2 operator+(double b, double2 a) {
  return make_double2(a.x + b, a.y + b);
}
inline __device__ void operator+=(double2 &a, double b) {
  a.x += b;
  a.y += b;
}

inline __device__ double3 operator+(double3 a, double3 b) {
  return make_double3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline __device__ void operator+=(double3 &a, double3 b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
}
inline __device__ double3 operator+(double3 a, double b) {
  return make_double3(a.x + b, a.y + b, a.z + b);
}
inline __device__ void operator+=(double3 &a, double b) {
  a.x += b;
  a.y += b;
  a.z += b;
}

inline __device__ double3 operator+(double b, double3 a) {
  return make_double3(a.x + b, a.y + b, a.z + b);
}

inline __device__ double4 operator+(double4 a, double4 b) {
  return make_double4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline __device__ void operator+=(double4 &a, double4 b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  a.w += b.w;
}
inline __device__ double4 operator+(double4 a, double b) {
  return make_double4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline __device__ double4 operator+(double b, double4 a) {
  return make_double4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline __device__ void operator+=(double4 &a, double b) {
  a.x += b;
  a.y += b;
  a.z += b;
  a.w += b;
}

////////////////////////////////////////////////////////////////////////////////
// subtract
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 operator-(double2 a, double2 b) {
  return make_double2(a.x - b.x, a.y - b.y);
}
inline __device__ void operator-=(double2 &a, double2 b) {
  a.x -= b.x;
  a.y -= b.y;
}
inline __device__ double2 operator-(double2 a, double b) {
  return make_double2(a.x - b, a.y - b);
}
inline __device__ double2 operator-(double b, double2 a) {
  return make_double2(b - a.x, b - a.y);
}
inline __device__ void operator-=(double2 &a, double b) {
  a.x -= b;
  a.y -= b;
}

inline __device__ double3 operator-(double3 a, double3 b) {
  return make_double3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline __device__ void operator-=(double3 &a, double3 b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}
inline __device__ double3 operator-(double3 a, double b) {
  return make_double3(a.x - b, a.y - b, a.z - b);
}
inline __device__ double3 operator-(double b, double3 a) {
  return make_double3(b - a.x, b - a.y, b - a.z);
}
inline __device__ void operator-=(double3 &a, double b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
}

inline __device__ double4 operator-(double4 a, double4 b) {
  return make_double4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline __device__ void operator-=(double4 &a, double4 b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  a.w -= b.w;
}
inline __device__ double4 operator-(double4 a, double b) {
  return make_double4(a.x - b, a.y - b, a.z - b, a.w - b);
}
inline __device__ void operator-=(double4 &a, double b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
  a.w -= b;
}

////////////////////////////////////////////////////////////////////////////////
// multiply
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 operator*(double2 a, double2 b) {
  return make_double2(a.x * b.x, a.y * b.y);
}
inline __device__ void operator*=(double2 &a, double2 b) {
  a.x *= b.x;
  a.y *= b.y;
}
inline __device__ double2 operator*(double2 a, double b) {
  return make_double2(a.x * b, a.y * b);
}
inline __device__ double2 operator*(double b, double2 a) {
  return make_double2(b * a.x, b * a.y);
}
inline __device__ void operator*=(double2 &a, double b) {
  a.x *= b;
  a.y *= b;
}

inline __device__ double3 operator*(double3 a, double3 b) {
  return make_double3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline __device__ void operator*=(double3 &a, double3 b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}
inline __device__ double3 operator*(double3 a, double b) {
  return make_double3(a.x * b, a.y * b, a.z * b);
}
inline __device__ double3 operator*(double b, double3 a) {
  return make_double3(b * a.x, b * a.y, b * a.z);
}
inline __device__ void operator*=(double3 &a, double b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}

inline __device__ double4 operator*(double4 a, double4 b) {
  return make_double4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline __device__ void operator*=(double4 &a, double4 b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
  a.w *= b.w;
}
inline __device__ double4 operator*(double4 a, double b) {
  return make_double4(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline __device__ double4 operator*(double b, double4 a) {
  return make_double4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline __device__ void operator*=(double4 &a, double b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
  a.w *= b;
}

////////////////////////////////////////////////////////////////////////////////
// divide
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 operator/(double2 a, double2 b) {
  return make_double2(a.x / b.x, a.y / b.y);
}
inline __device__ void operator/=(double2 &a, double2 b) {
  a.x /= b.x;
  a.y /= b.y;
}
inline __device__ double2 operator/(double2 a, double b) {
  return make_double2(a.x / b, a.y / b);
}
inline __device__ void operator/=(double2 &a, double b) {
  a.x /= b;
  a.y /= b;
}
inline __device__ double2 operator/(double b, double2 a) {
  return make_double2(b / a.x, b / a.y);
}

inline __device__ double3 operator/(double3 a, double3 b) {
  return make_double3(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline __device__ void operator/=(double3 &a, double3 b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
}
inline __device__ double3 operator/(double3 a, double b) {
  return make_double3(a.x / b, a.y / b, a.z / b);
}
inline __device__ void operator/=(double3 &a, double b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
}
inline __device__ double3 operator/(double b, double3 a) {
  return make_double3(b / a.x, b / a.y, b / a.z);
}

inline __device__ double4 operator/(double4 a, double4 b) {
  return make_double4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
inline __device__ void operator/=(double4 &a, double4 b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
  a.w /= b.w;
}
inline __device__ double4 operator/(double4 a, double b) {
  return make_double4(a.x / b, a.y / b, a.z / b, a.w / b);
}
inline __device__ void operator/=(double4 &a, double b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
  a.w /= b;
}
inline __device__ double4 operator/(double b, double4 a) {
  return make_double4(b / a.x, b / a.y, b / a.z, b / a.w);
}

////////////////////////////////////////////////////////////////////////////////
// min
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 fminf(double2 a, double2 b) {
  return make_double2(fminf(a.x, b.x), fminf(a.y, b.y));
}
inline __device__ double3 fminf(double3 a, double3 b) {
  return make_double3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}
inline __device__ double4 fminf(double4 a, double4 b) {
  return make_double4(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z),
                      fminf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// max
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 fmaxf(double2 a, double2 b) {
  return make_double2(fmaxf(a.x, b.x), fmaxf(a.y, b.y));
}
inline __device__ double3 fmaxf(double3 a, double3 b) {
  return make_double3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}
inline __device__ double4 fmaxf(double4 a, double4 b) {
  return make_double4(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z),
                      fmaxf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// dot product
////////////////////////////////////////////////////////////////////////////////

inline __device__ double dot(double2 a, double2 b) {
  return a.x * b.x + a.y * b.y;
}
inline __device__ double dot(double3 a, double3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline __device__ double dot(double4 a, double4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

////////////////////////////////////////////////////////////////////////////////
// length
////////////////////////////////////////////////////////////////////////////////

inline __device__ double length(double2 v) { return sqrtf(dot(v, v)); }
inline __device__ double length(double3 v) { return sqrtf(dot(v, v)); }
inline __device__ double length(double4 v) { return sqrtf(dot(v, v)); }

////////////////////////////////////////////////////////////////////////////////
// normalize
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 normalize(double2 v) {
  double invLen = rsqrtf(dot(v, v));
  return v * invLen;
}
inline __device__ double3 normalize(double3 v) {
  double invLen = rsqrtf(dot(v, v));
  return v * invLen;
}
inline __device__ double4 normalize(double4 v) {
  double invLen = rsqrtf(dot(v, v));
  return v * invLen;
}

////////////////////////////////////////////////////////////////////////////////
// floor
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 floorf(double2 v) {
  return make_double2(floorf(v.x), floorf(v.y));
}
inline __device__ double3 floorf(double3 v) {
  return make_double3(floorf(v.x), floorf(v.y), floorf(v.z));
}
inline __device__ double4 floorf(double4 v) {
  return make_double4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// frac - returns the fractional portion of a scalar or each vector component
////////////////////////////////////////////////////////////////////////////////

inline __device__ double fracf(double v) { return v - floorf(v); }
inline __device__ double2 fracf(double2 v) {
  return make_double2(fracf(v.x), fracf(v.y));
}
inline __device__ double3 fracf(double3 v) {
  return make_double3(fracf(v.x), fracf(v.y), fracf(v.z));
}
inline __device__ double4 fracf(double4 v) {
  return make_double4(fracf(v.x), fracf(v.y), fracf(v.z), fracf(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// fmod
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 fmodf(double2 a, double2 b) {
  return make_double2(fmodf(a.x, b.x), fmodf(a.y, b.y));
}
inline __device__ double3 fmodf(double3 a, double3 b) {
  return make_double3(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z));
}
inline __device__ double4 fmodf(double4 a, double4 b) {
  return make_double4(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z),
                      fmodf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// absolute value
////////////////////////////////////////////////////////////////////////////////

inline __device__ double2 fabs(double2 v) {
  return make_double2(fabs(v.x), fabs(v.y));
}
inline __device__ double3 fabs(double3 v) {
  return make_double3(fabs(v.x), fabs(v.y), fabs(v.z));
}
inline __device__ double4 fabs(double4 v) {
  return make_double4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// cross product
////////////////////////////////////////////////////////////////////////////////

inline __device__ double3 cross(double3 a, double3 b) {
  return make_double3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x);
}

#endif  // __CUDACC__

#endif  // CORE_GPU_HELPER_MATH_DOUBLE_H_
