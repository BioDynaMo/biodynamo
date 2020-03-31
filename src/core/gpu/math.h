// double3 operators

inline __device__ double3 make_double3(const double* a, uint32_t idx) {
  idx *= 3;
  return make_double3(a[idx], a[idx + 1], a[idx + 2]);
}

inline __device__ double3 operator-(double3 a) {
  return make_double3(-a.x, -a.y, -a.z);
}

inline __device__ double3 operator+(double3 a, double3 b) {
  return make_double3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline __device__ double3 operator-(double3 a, double3 b) {
  return make_double3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline __device__ double3 operator*(double3 a, double3 b) {
  return make_double3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline __device__ double3 operator/(double3 a, double3 b) {
  return make_double3(a.x / b.x, a.y / b.y, a.z / b.z);
}

inline __device__ void operator+=(double3& a, double3 b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
}

inline __device__ void operator-=(double3& a, double3 b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}

inline __device__ void operator*=(double3& a, double3 b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
}

inline __device__ void operator/=(double3& a, double3 b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
}

inline __device__ double3 operator*(double3 a, double b) {
  return make_double3(a.x * b, a.y * b, a.z * b);
}

inline __device__ double3 operator*(double a, double3 b) {
  return make_double3(a * b.x, a * b.y, a * b.z);
}

inline __device__ double3 operator/(double3 a, double b) {
  double scale = 1.0 / b;
  return a * scale;
}

inline __device__ void operator*=(double3& a, double b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}

inline __device__ double dot(double3 a, double3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline __device__ double3 cross(double3 a, double3 b) {
  return make_double3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x);
}

inline __device__ double3 Normalize(double3 a) {
  return a * rsqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline __device__ double Norm(double3 a) {
  return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}
