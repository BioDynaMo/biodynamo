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

inline real fminf(real a, real b) { return a < b ? a : b; }

inline real fmaxf(real a, real b) { return a > b ? a : b; }

inline __device__ real rsqrtf(real x) { return 1.0f / sqrtf(x); }

////////////////////////////////////////////////////////////////////////////////
// constructors
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 make_real2(real s) { return make_real2(s, s); }
inline __device__ real2 make_real2(real3 a) {
  return make_real2(a.x, a.y);
}
inline __device__ real2 make_real2(int2 a) {
  return make_real2(real(a.x), real(a.y));
}
inline __device__ real2 make_real2(uint2 a) {
  return make_real2(real(a.x), real(a.y));
}

inline __device__ int2 make_int2(real2 a) {
  return make_int2(int(a.x), int(a.y));
}

inline __device__ real3 make_real3(real s) {
  return make_real3(s, s, s);
}
inline __device__ real3 make_real3(real2 a) {
  return make_real3(a.x, a.y, 0.0f);
}
inline __device__ real3 make_real3(real2 a, real s) {
  return make_real3(a.x, a.y, s);
}
inline __device__ real3 make_real3(real4 a) {
  return make_real3(a.x, a.y, a.z);
}
inline __device__ real3 make_real3(int3 a) {
  return make_real3(real(a.x), real(a.y), real(a.z));
}
inline __device__ real3 make_real3(uint3 a) {
  return make_real3(real(a.x), real(a.y), real(a.z));
}

inline __device__ int3 make_int3(real3 a) {
  return make_int3(int(a.x), int(a.y), int(a.z));
}

inline __device__ real4 make_real4(real s) {
  return make_real4(s, s, s, s);
}
inline __device__ real4 make_real4(real3 a) {
  return make_real4(a.x, a.y, a.z, 0.0f);
}
inline __device__ real4 make_real4(real3 a, real w) {
  return make_real4(a.x, a.y, a.z, w);
}
inline __device__ real4 make_real4(int4 a) {
  return make_real4(real(a.x), real(a.y), real(a.z), real(a.w));
}
inline __device__ real4 make_real4(uint4 a) {
  return make_real4(real(a.x), real(a.y), real(a.z), real(a.w));
}

inline __device__ int4 make_int4(real4 a) {
  return make_int4(int(a.x), int(a.y), int(a.z), int(a.w));
}

////////////////////////////////////////////////////////////////////////////////
// negate
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 operator-(real2 &a) {
  return make_real2(-a.x, -a.y);
}
inline __device__ real3 operator-(real3 &a) {
  return make_real3(-a.x, -a.y, -a.z);
}
inline __device__ real4 operator-(real4 &a) {
  return make_real4(-a.x, -a.y, -a.z, -a.w);
}

////////////////////////////////////////////////////////////////////////////////
// addition
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 operator+(real2 a, real2 b) {
  return make_real2(a.x + b.x, a.y + b.y);
}
inline __device__ void operator+=(real2 &a, real2 b) {
  a.x += b.x;
  a.y += b.y;
}
inline __device__ real2 operator+(real2 a, real b) {
  return make_real2(a.x + b, a.y + b);
}
inline __device__ real2 operator+(real b, real2 a) {
  return make_real2(a.x + b, a.y + b);
}
inline __device__ void operator+=(real2 &a, real b) {
  a.x += b;
  a.y += b;
}

inline __device__ real3 operator+(real3 a, real3 b) {
  return make_real3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline __device__ void operator+=(real3 &a, real3 b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
}
inline __device__ real3 operator+(real3 a, real b) {
  return make_real3(a.x + b, a.y + b, a.z + b);
}
inline __device__ void operator+=(real3 &a, real b) {
  a.x += b;
  a.y += b;
  a.z += b;
}

inline __device__ real3 operator+(real b, real3 a) {
  return make_real3(a.x + b, a.y + b, a.z + b);
}

inline __device__ real4 operator+(real4 a, real4 b) {
  return make_real4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline __device__ void operator+=(real4 &a, real4 b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  a.w += b.w;
}
inline __device__ real4 operator+(real4 a, real b) {
  return make_real4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline __device__ real4 operator+(real b, real4 a) {
  return make_real4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline __device__ void operator+=(real4 &a, real b) {
  a.x += b;
  a.y += b;
  a.z += b;
  a.w += b;
}

////////////////////////////////////////////////////////////////////////////////
// subtract
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 operator-(real2 a, real2 b) {
  return make_real2(a.x - b.x, a.y - b.y);
}
inline __device__ void operator-=(real2 &a, real2 b) {
  a.x -= b.x;
  a.y -= b.y;
}
inline __device__ real2 operator-(real2 a, real b) {
  return make_real2(a.x - b, a.y - b);
}
inline __device__ real2 operator-(real b, real2 a) {
  return make_real2(b - a.x, b - a.y);
}
inline __device__ void operator-=(real2 &a, real b) {
  a.x -= b;
  a.y -= b;
}

inline __device__ real3 operator-(real3 a, real3 b) {
  return make_real3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline __device__ void operator-=(real3 &a, real3 b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}
inline __device__ real3 operator-(real3 a, real b) {
  return make_real3(a.x - b, a.y - b, a.z - b);
}
inline __device__ real3 operator-(real b, real3 a) {
  return make_real3(b - a.x, b - a.y, b - a.z);
}
inline __device__ void operator-=(real3 &a, real b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
}

inline __device__ real4 operator-(real4 a, real4 b) {
  return make_real4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline __device__ void operator-=(real4 &a, real4 b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  a.w -= b.w;
}
inline __device__ real4 operator-(real4 a, real b) {
  return make_real4(a.x - b, a.y - b, a.z - b, a.w - b);
}
inline __device__ void operator-=(real4 &a, real b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
  a.w -= b;
}

////////////////////////////////////////////////////////////////////////////////
// multiply
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 operator*(real2 a, real2 b) {
  return make_real2(a.x * b.x, a.y * b.y);
}
inline __device__ void operator*=(real2 &a, real2 b) {
  a.x *= b.x;
  a.y *= b.y;
}
inline __device__ real2 operator*(real2 a, real b) {
  return make_real2(a.x * b, a.y * b);
}
inline __device__ real2 operator*(real b, real2 a) {
  return make_real2(b * a.x, b * a.y);
}
inline __device__ void operator*=(real2 &a, real b) {
  a.x *= b;
  a.y *= b;
}

inline __device__ real3 operator*(real3 a, real3 b) {
  return make_real3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline __device__ void operator*=(real3 &a, real3 b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}
inline __device__ real3 operator*(real3 a, real b) {
  return make_real3(a.x * b, a.y * b, a.z * b);
}
inline __device__ real3 operator*(real b, real3 a) {
  return make_real3(b * a.x, b * a.y, b * a.z);
}
inline __device__ void operator*=(real3 &a, real b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}

inline __device__ real4 operator*(real4 a, real4 b) {
  return make_real4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline __device__ void operator*=(real4 &a, real4 b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
  a.w *= b.w;
}
inline __device__ real4 operator*(real4 a, real b) {
  return make_real4(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline __device__ real4 operator*(real b, real4 a) {
  return make_real4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline __device__ void operator*=(real4 &a, real b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
  a.w *= b;
}

////////////////////////////////////////////////////////////////////////////////
// divide
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 operator/(real2 a, real2 b) {
  return make_real2(a.x / b.x, a.y / b.y);
}
inline __device__ void operator/=(real2 &a, real2 b) {
  a.x /= b.x;
  a.y /= b.y;
}
inline __device__ real2 operator/(real2 a, real b) {
  return make_real2(a.x / b, a.y / b);
}
inline __device__ void operator/=(real2 &a, real b) {
  a.x /= b;
  a.y /= b;
}
inline __device__ real2 operator/(real b, real2 a) {
  return make_real2(b / a.x, b / a.y);
}

inline __device__ real3 operator/(real3 a, real3 b) {
  return make_real3(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline __device__ void operator/=(real3 &a, real3 b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
}
inline __device__ real3 operator/(real3 a, real b) {
  return make_real3(a.x / b, a.y / b, a.z / b);
}
inline __device__ void operator/=(real3 &a, real b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
}
inline __device__ real3 operator/(real b, real3 a) {
  return make_real3(b / a.x, b / a.y, b / a.z);
}

inline __device__ real4 operator/(real4 a, real4 b) {
  return make_real4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
inline __device__ void operator/=(real4 &a, real4 b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
  a.w /= b.w;
}
inline __device__ real4 operator/(real4 a, real b) {
  return make_real4(a.x / b, a.y / b, a.z / b, a.w / b);
}
inline __device__ void operator/=(real4 &a, real b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
  a.w /= b;
}
inline __device__ real4 operator/(real b, real4 a) {
  return make_real4(b / a.x, b / a.y, b / a.z, b / a.w);
}

////////////////////////////////////////////////////////////////////////////////
// min
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 fminf(real2 a, real2 b) {
  return make_real2(fminf(a.x, b.x), fminf(a.y, b.y));
}
inline __device__ real3 fminf(real3 a, real3 b) {
  return make_real3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}
inline __device__ real4 fminf(real4 a, real4 b) {
  return make_real4(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z),
                      fminf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// max
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 fmaxf(real2 a, real2 b) {
  return make_real2(fmaxf(a.x, b.x), fmaxf(a.y, b.y));
}
inline __device__ real3 fmaxf(real3 a, real3 b) {
  return make_real3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}
inline __device__ real4 fmaxf(real4 a, real4 b) {
  return make_real4(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z),
                      fmaxf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// dot product
////////////////////////////////////////////////////////////////////////////////

inline __device__ real dot(real2 a, real2 b) {
  return a.x * b.x + a.y * b.y;
}
inline __device__ real dot(real3 a, real3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline __device__ real dot(real4 a, real4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

////////////////////////////////////////////////////////////////////////////////
// length
////////////////////////////////////////////////////////////////////////////////

inline __device__ real length(real2 v) { return sqrtf(dot(v, v)); }
inline __device__ real length(real3 v) { return sqrtf(dot(v, v)); }
inline __device__ real length(real4 v) { return sqrtf(dot(v, v)); }

////////////////////////////////////////////////////////////////////////////////
// normalize
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 normalize(real2 v) {
  real invLen = rsqrtf(dot(v, v));
  return v * invLen;
}
inline __device__ real3 normalize(real3 v) {
  real invLen = rsqrtf(dot(v, v));
  return v * invLen;
}
inline __device__ real4 normalize(real4 v) {
  real invLen = rsqrtf(dot(v, v));
  return v * invLen;
}

////////////////////////////////////////////////////////////////////////////////
// floor
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 floorf(real2 v) {
  return make_real2(floorf(v.x), floorf(v.y));
}
inline __device__ real3 floorf(real3 v) {
  return make_real3(floorf(v.x), floorf(v.y), floorf(v.z));
}
inline __device__ real4 floorf(real4 v) {
  return make_real4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// frac - returns the fractional portion of a scalar or each vector component
////////////////////////////////////////////////////////////////////////////////

inline __device__ real fracf(real v) { return v - floorf(v); }
inline __device__ real2 fracf(real2 v) {
  return make_real2(fracf(v.x), fracf(v.y));
}
inline __device__ real3 fracf(real3 v) {
  return make_real3(fracf(v.x), fracf(v.y), fracf(v.z));
}
inline __device__ real4 fracf(real4 v) {
  return make_real4(fracf(v.x), fracf(v.y), fracf(v.z), fracf(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// fmod
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 fmodf(real2 a, real2 b) {
  return make_real2(fmodf(a.x, b.x), fmodf(a.y, b.y));
}
inline __device__ real3 fmodf(real3 a, real3 b) {
  return make_real3(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z));
}
inline __device__ real4 fmodf(real4 a, real4 b) {
  return make_real4(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z),
                      fmodf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// absolute value
////////////////////////////////////////////////////////////////////////////////

inline __device__ real2 fabs(real2 v) {
  return make_real2(fabs(v.x), fabs(v.y));
}
inline __device__ real3 fabs(real3 v) {
  return make_real3(fabs(v.x), fabs(v.y), fabs(v.z));
}
inline __device__ real4 fabs(real4 v) {
  return make_real4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// cross product
////////////////////////////////////////////////////////////////////////////////

inline __device__ real3 cross(real3 a, real3 b) {
  return make_real3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x);
}

#endif  // __CUDACC__

#endif  // CORE_GPU_HELPER_MATH_DOUBLE_H_
