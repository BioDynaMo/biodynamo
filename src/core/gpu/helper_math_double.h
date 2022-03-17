// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

inline real_t fminf(real_t a, real_t b) { return a < b ? a : b; }

inline real_t fmaxf(real_t a, real_t b) { return a > b ? a : b; }

inline __device__ real_t rsqrtf(real_t x) { return 1.0f / sqrtf(x); }

////////////////////////////////////////////////////////////////////////////////
// constructors
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 make_real_t2(real_t s) { return make_real_t2(s, s); }
inline __device__ real_t2 make_real_t2(real_t3 a) {
  return make_real_t2(a.x, a.y);
}
inline __device__ real_t2 make_real_t2(int2 a) {
  return make_real_t2(real_t(a.x), real_t(a.y));
}
inline __device__ real_t2 make_real_t2(uint2 a) {
  return make_real_t2(real_t(a.x), real_t(a.y));
}

inline __device__ int2 make_int2(real_t2 a) {
  return make_int2(int(a.x), int(a.y));
}

inline __device__ real_t3 make_real_t3(real_t s) {
  return make_real_t3(s, s, s);
}
inline __device__ real_t3 make_real_t3(real_t2 a) {
  return make_real_t3(a.x, a.y, 0.0f);
}
inline __device__ real_t3 make_real_t3(real_t2 a, real_t s) {
  return make_real_t3(a.x, a.y, s);
}
inline __device__ real_t3 make_real_t3(real_t4 a) {
  return make_real_t3(a.x, a.y, a.z);
}
inline __device__ real_t3 make_real_t3(int3 a) {
  return make_real_t3(real_t(a.x), real_t(a.y), real_t(a.z));
}
inline __device__ real_t3 make_real_t3(uint3 a) {
  return make_real_t3(real_t(a.x), real_t(a.y), real_t(a.z));
}

inline __device__ int3 make_int3(real_t3 a) {
  return make_int3(int(a.x), int(a.y), int(a.z));
}

inline __device__ real_t4 make_real_t4(real_t s) {
  return make_real_t4(s, s, s, s);
}
inline __device__ real_t4 make_real_t4(real_t3 a) {
  return make_real_t4(a.x, a.y, a.z, 0.0f);
}
inline __device__ real_t4 make_real_t4(real_t3 a, real_t w) {
  return make_real_t4(a.x, a.y, a.z, w);
}
inline __device__ real_t4 make_real_t4(int4 a) {
  return make_real_t4(real_t(a.x), real_t(a.y), real_t(a.z), real_t(a.w));
}
inline __device__ real_t4 make_real_t4(uint4 a) {
  return make_real_t4(real_t(a.x), real_t(a.y), real_t(a.z), real_t(a.w));
}

inline __device__ int4 make_int4(real_t4 a) {
  return make_int4(int(a.x), int(a.y), int(a.z), int(a.w));
}

////////////////////////////////////////////////////////////////////////////////
// negate
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 operator-(real_t2 &a) {
  return make_real_t2(-a.x, -a.y);
}
inline __device__ real_t3 operator-(real_t3 &a) {
  return make_real_t3(-a.x, -a.y, -a.z);
}
inline __device__ real_t4 operator-(real_t4 &a) {
  return make_real_t4(-a.x, -a.y, -a.z, -a.w);
}

////////////////////////////////////////////////////////////////////////////////
// addition
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 operator+(real_t2 a, real_t2 b) {
  return make_real_t2(a.x + b.x, a.y + b.y);
}
inline __device__ void operator+=(real_t2 &a, real_t2 b) {
  a.x += b.x;
  a.y += b.y;
}
inline __device__ real_t2 operator+(real_t2 a, real_t b) {
  return make_real_t2(a.x + b, a.y + b);
}
inline __device__ real_t2 operator+(real_t b, real_t2 a) {
  return make_real_t2(a.x + b, a.y + b);
}
inline __device__ void operator+=(real_t2 &a, real_t b) {
  a.x += b;
  a.y += b;
}

inline __device__ real_t3 operator+(real_t3 a, real_t3 b) {
  return make_real_t3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline __device__ void operator+=(real_t3 &a, real_t3 b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
}
inline __device__ real_t3 operator+(real_t3 a, real_t b) {
  return make_real_t3(a.x + b, a.y + b, a.z + b);
}
inline __device__ void operator+=(real_t3 &a, real_t b) {
  a.x += b;
  a.y += b;
  a.z += b;
}

inline __device__ real_t3 operator+(real_t b, real_t3 a) {
  return make_real_t3(a.x + b, a.y + b, a.z + b);
}

inline __device__ real_t4 operator+(real_t4 a, real_t4 b) {
  return make_real_t4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline __device__ void operator+=(real_t4 &a, real_t4 b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  a.w += b.w;
}
inline __device__ real_t4 operator+(real_t4 a, real_t b) {
  return make_real_t4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline __device__ real_t4 operator+(real_t b, real_t4 a) {
  return make_real_t4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline __device__ void operator+=(real_t4 &a, real_t b) {
  a.x += b;
  a.y += b;
  a.z += b;
  a.w += b;
}

////////////////////////////////////////////////////////////////////////////////
// subtract
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 operator-(real_t2 a, real_t2 b) {
  return make_real_t2(a.x - b.x, a.y - b.y);
}
inline __device__ void operator-=(real_t2 &a, real_t2 b) {
  a.x -= b.x;
  a.y -= b.y;
}
inline __device__ real_t2 operator-(real_t2 a, real_t b) {
  return make_real_t2(a.x - b, a.y - b);
}
inline __device__ real_t2 operator-(real_t b, real_t2 a) {
  return make_real_t2(b - a.x, b - a.y);
}
inline __device__ void operator-=(real_t2 &a, real_t b) {
  a.x -= b;
  a.y -= b;
}

inline __device__ real_t3 operator-(real_t3 a, real_t3 b) {
  return make_real_t3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline __device__ void operator-=(real_t3 &a, real_t3 b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}
inline __device__ real_t3 operator-(real_t3 a, real_t b) {
  return make_real_t3(a.x - b, a.y - b, a.z - b);
}
inline __device__ real_t3 operator-(real_t b, real_t3 a) {
  return make_real_t3(b - a.x, b - a.y, b - a.z);
}
inline __device__ void operator-=(real_t3 &a, real_t b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
}

inline __device__ real_t4 operator-(real_t4 a, real_t4 b) {
  return make_real_t4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline __device__ void operator-=(real_t4 &a, real_t4 b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  a.w -= b.w;
}
inline __device__ real_t4 operator-(real_t4 a, real_t b) {
  return make_real_t4(a.x - b, a.y - b, a.z - b, a.w - b);
}
inline __device__ void operator-=(real_t4 &a, real_t b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
  a.w -= b;
}

////////////////////////////////////////////////////////////////////////////////
// multiply
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 operator*(real_t2 a, real_t2 b) {
  return make_real_t2(a.x * b.x, a.y * b.y);
}
inline __device__ void operator*=(real_t2 &a, real_t2 b) {
  a.x *= b.x;
  a.y *= b.y;
}
inline __device__ real_t2 operator*(real_t2 a, real_t b) {
  return make_real_t2(a.x * b, a.y * b);
}
inline __device__ real_t2 operator*(real_t b, real_t2 a) {
  return make_real_t2(b * a.x, b * a.y);
}
inline __device__ void operator*=(real_t2 &a, real_t b) {
  a.x *= b;
  a.y *= b;
}

inline __device__ real_t3 operator*(real_t3 a, real_t3 b) {
  return make_real_t3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline __device__ void operator*=(real_t3 &a, real_t3 b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}
inline __device__ real_t3 operator*(real_t3 a, real_t b) {
  return make_real_t3(a.x * b, a.y * b, a.z * b);
}
inline __device__ real_t3 operator*(real_t b, real_t3 a) {
  return make_real_t3(b * a.x, b * a.y, b * a.z);
}
inline __device__ void operator*=(real_t3 &a, real_t b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}

inline __device__ real_t4 operator*(real_t4 a, real_t4 b) {
  return make_real_t4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline __device__ void operator*=(real_t4 &a, real_t4 b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
  a.w *= b.w;
}
inline __device__ real_t4 operator*(real_t4 a, real_t b) {
  return make_real_t4(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline __device__ real_t4 operator*(real_t b, real_t4 a) {
  return make_real_t4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline __device__ void operator*=(real_t4 &a, real_t b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
  a.w *= b;
}

////////////////////////////////////////////////////////////////////////////////
// divide
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 operator/(real_t2 a, real_t2 b) {
  return make_real_t2(a.x / b.x, a.y / b.y);
}
inline __device__ void operator/=(real_t2 &a, real_t2 b) {
  a.x /= b.x;
  a.y /= b.y;
}
inline __device__ real_t2 operator/(real_t2 a, real_t b) {
  return make_real_t2(a.x / b, a.y / b);
}
inline __device__ void operator/=(real_t2 &a, real_t b) {
  a.x /= b;
  a.y /= b;
}
inline __device__ real_t2 operator/(real_t b, real_t2 a) {
  return make_real_t2(b / a.x, b / a.y);
}

inline __device__ real_t3 operator/(real_t3 a, real_t3 b) {
  return make_real_t3(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline __device__ void operator/=(real_t3 &a, real_t3 b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
}
inline __device__ real_t3 operator/(real_t3 a, real_t b) {
  return make_real_t3(a.x / b, a.y / b, a.z / b);
}
inline __device__ void operator/=(real_t3 &a, real_t b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
}
inline __device__ real_t3 operator/(real_t b, real_t3 a) {
  return make_real_t3(b / a.x, b / a.y, b / a.z);
}

inline __device__ real_t4 operator/(real_t4 a, real_t4 b) {
  return make_real_t4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
inline __device__ void operator/=(real_t4 &a, real_t4 b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
  a.w /= b.w;
}
inline __device__ real_t4 operator/(real_t4 a, real_t b) {
  return make_real_t4(a.x / b, a.y / b, a.z / b, a.w / b);
}
inline __device__ void operator/=(real_t4 &a, real_t b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
  a.w /= b;
}
inline __device__ real_t4 operator/(real_t b, real_t4 a) {
  return make_real_t4(b / a.x, b / a.y, b / a.z, b / a.w);
}

////////////////////////////////////////////////////////////////////////////////
// min
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 fminf(real_t2 a, real_t2 b) {
  return make_real_t2(fminf(a.x, b.x), fminf(a.y, b.y));
}
inline __device__ real_t3 fminf(real_t3 a, real_t3 b) {
  return make_real_t3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}
inline __device__ real_t4 fminf(real_t4 a, real_t4 b) {
  return make_real_t4(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z),
                      fminf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// max
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 fmaxf(real_t2 a, real_t2 b) {
  return make_real_t2(fmaxf(a.x, b.x), fmaxf(a.y, b.y));
}
inline __device__ real_t3 fmaxf(real_t3 a, real_t3 b) {
  return make_real_t3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}
inline __device__ real_t4 fmaxf(real_t4 a, real_t4 b) {
  return make_real_t4(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z),
                      fmaxf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// dot product
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t dot(real_t2 a, real_t2 b) {
  return a.x * b.x + a.y * b.y;
}
inline __device__ real_t dot(real_t3 a, real_t3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline __device__ real_t dot(real_t4 a, real_t4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

////////////////////////////////////////////////////////////////////////////////
// length
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t length(real_t2 v) { return sqrtf(dot(v, v)); }
inline __device__ real_t length(real_t3 v) { return sqrtf(dot(v, v)); }
inline __device__ real_t length(real_t4 v) { return sqrtf(dot(v, v)); }

////////////////////////////////////////////////////////////////////////////////
// normalize
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 normalize(real_t2 v) {
  real_t invLen = rsqrtf(dot(v, v));
  return v * invLen;
}
inline __device__ real_t3 normalize(real_t3 v) {
  real_t invLen = rsqrtf(dot(v, v));
  return v * invLen;
}
inline __device__ real_t4 normalize(real_t4 v) {
  real_t invLen = rsqrtf(dot(v, v));
  return v * invLen;
}

////////////////////////////////////////////////////////////////////////////////
// floor
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 floorf(real_t2 v) {
  return make_real_t2(floorf(v.x), floorf(v.y));
}
inline __device__ real_t3 floorf(real_t3 v) {
  return make_real_t3(floorf(v.x), floorf(v.y), floorf(v.z));
}
inline __device__ real_t4 floorf(real_t4 v) {
  return make_real_t4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// frac - returns the fractional portion of a scalar or each vector component
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t fracf(real_t v) { return v - floorf(v); }
inline __device__ real_t2 fracf(real_t2 v) {
  return make_real_t2(fracf(v.x), fracf(v.y));
}
inline __device__ real_t3 fracf(real_t3 v) {
  return make_real_t3(fracf(v.x), fracf(v.y), fracf(v.z));
}
inline __device__ real_t4 fracf(real_t4 v) {
  return make_real_t4(fracf(v.x), fracf(v.y), fracf(v.z), fracf(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// fmod
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 fmodf(real_t2 a, real_t2 b) {
  return make_real_t2(fmodf(a.x, b.x), fmodf(a.y, b.y));
}
inline __device__ real_t3 fmodf(real_t3 a, real_t3 b) {
  return make_real_t3(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z));
}
inline __device__ real_t4 fmodf(real_t4 a, real_t4 b) {
  return make_real_t4(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z),
                      fmodf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// absolute value
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t2 fabs(real_t2 v) {
  return make_real_t2(fabs(v.x), fabs(v.y));
}
inline __device__ real_t3 fabs(real_t3 v) {
  return make_real_t3(fabs(v.x), fabs(v.y), fabs(v.z));
}
inline __device__ real_t4 fabs(real_t4 v) {
  return make_real_t4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// cross product
////////////////////////////////////////////////////////////////////////////////

inline __device__ real_t3 cross(real_t3 a, real_t3 b) {
  return make_real_t3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x);
}

#endif  // __CUDACC__

#endif  // CORE_GPU_HELPER_MATH_DOUBLE_H_
