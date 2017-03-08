/// Benchmarks SOA and AOSOA memory layout in different scenarios
/// For more information see help message in function `main`
#include <assert.h>

#include <cmath>
#include <iostream>

#include "timing.h"

#define Read 5
#define ReadWrite 4
#define ReadWriteAlternating 3

// validate compile time parameter `MODE`
#if defined(MODE) && \
    (MODE != Read && MODE != ReadWrite && MODE != ReadWriteAlternating)
#error MODE must be one of: Read, ReadWrite, ReadWriteAlternating
#endif

/// attributes stored as SOA memory layout
struct Soa {
  double* a;
  double* b;
  double* c;
  double* d;
  double* e;
  double* f;
  double* g;
  double* h;
  double* i;
  double* j;
  size_t length;

  explicit Soa(size_t length)
      : a(new double[length]),
        b(new double[length]),
        c(new double[length]),
        d(new double[length]),
        e(new double[length]),
        f(new double[length]),
        g(new double[length]),
        h(new double[length]),
        i(new double[length]),
        j(new double[length]),
        length(length) {
    for (size_t idx = 0; idx < length; idx++) {
      a[idx] = 3.14;
      b[idx] = 3.14;
      c[idx] = 3.14;
      d[idx] = 3.14;
      e[idx] = 3.14;
      f[idx] = 3.14;
      g[idx] = 3.14;
      h[idx] = 3.14;
      i[idx] = 3.14;
      j[idx] = 3.14;
    }
  }

  ~Soa() {
    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;
    delete[] e;
    delete[] f;
    delete[] g;
    delete[] h;
    delete[] i;
    delete[] j;
  }
};

/// Attributes stored in a fixed size vector form
/// becomes AOSOA when an array of `Aosoa` is created
/// e.g. `Aosoa<4>* data = new Aosoa<4>[1024];`
/// `Aosoa<4> data;` would be a SOA memory layout
template <size_t N>
struct Aosoa {
  double a[N];
  double b[N];
  double c[N];
  double d[N];
  double e[N];
  double f[N];
  double g[N];
  double h[N];
  double i[N];
  double j[N];

  Aosoa() {
    for (size_t idx = 0; idx < N; idx++) {
      a[idx] = 3.14;
      b[idx] = 3.14;
      c[idx] = 3.14;
      d[idx] = 3.14;
      e[idx] = 3.14;
      f[idx] = 3.14;
      g[idx] = 3.14;
      h[idx] = 3.14;
      i[idx] = 3.14;
      j[idx] = 3.14;
    }
  }
};

double SoaAccessOneAttribute(Soa* data) {
  volatile double sum = 0;
  for (size_t i = 0; i < data->length; i++) {
    sum += data->e[i];
#if defined(MODE) && (MODE == ReadWrite || MODE == ReadWriteAlternating)
    data->e[i] = sum;
#endif
  }
  return sum;
}

template <size_t N>
double AosoaAccessOneAttribute(Aosoa<N>* data, size_t vectors) {
  volatile double sum = 0;
  for (size_t i = 0; i < vectors; i++) {
    for (size_t j = 0; j < N; j++) {
      sum += data[i].e[j];
#if defined(MODE) && (MODE == ReadWrite || MODE == ReadWriteAlternating)
      data[i].e[j] = sum;
#endif
    }
  }
  return sum;
}

double SoaAccessAllAttributes(Soa* data) {
  volatile double sum = 0;
  for (size_t i = 0; i < data->length; i++) {
#if !defined(MODE) || MODE == Read
    sum += data->a[i];
    sum += data->b[i];
    sum += data->c[i];
    sum += data->d[i];
    sum += data->e[i];
    sum += data->f[i];
    sum += data->g[i];
    sum += data->h[i];
    sum += data->i[i];
    sum += data->j[i];

#elif defined(MODE) && MODE == ReadWrite
    sum += data->a[i];
    sum += data->b[i];
    sum += data->c[i];
    sum += data->d[i];
    sum += data->e[i];
    sum += data->f[i];
    sum += data->g[i];
    sum += data->h[i];
    sum += data->i[i];
    sum += data->j[i];
    data->a[i] = sum;
    data->b[i] = sum;
    data->c[i] = sum;
    data->d[i] = sum;
    data->e[i] = sum;
    data->f[i] = sum;
    data->g[i] = sum;
    data->h[i] = sum;
    data->i[i] = sum;
    data->j[i] = sum;

#elif defined(MODE) && MODE == ReadWriteAlternating
    sum += data->a[i];
    data->a[i] = sum;
    sum += data->b[i];
    data->b[i] = sum;
    sum += data->c[i];
    data->c[i] = sum;
    sum += data->d[i];
    data->d[i] = sum;
    sum += data->e[i];
    data->e[i] = sum;
    sum += data->f[i];
    data->f[i] = sum;
    sum += data->g[i];
    data->g[i] = sum;
    sum += data->h[i];
    data->h[i] = sum;
    sum += data->i[i];
    data->i[i] = sum;
    sum += data->j[i];
    data->j[i] = sum;
#endif
  }

  return sum;
}

template <size_t N>
double AosoaAccessAllAttributes(Aosoa<N>* data, size_t vectors) {
  volatile double sum = 0;
  for (size_t idx = 0; idx < vectors; idx++) {
    auto& current = data[idx];
    for (size_t j = 0; j < N; j++) {
#if !defined(MODE) || MODE == Read
      sum += current.a[j];
      sum += current.b[j];
      sum += current.c[j];
      sum += current.d[j];
      sum += current.f[j];
      sum += current.e[j];
      sum += current.g[j];
      sum += current.h[j];
      sum += current.i[j];
      sum += current.j[j];

#elif defined(MODE) && MODE == ReadWrite
      sum += current.a[j];
      sum += current.b[j];
      sum += current.c[j];
      sum += current.d[j];
      sum += current.f[j];
      sum += current.e[j];
      sum += current.g[j];
      sum += current.h[j];
      sum += current.i[j];
      sum += current.j[j];
      current.a[j] = sum;
      current.b[j] = sum;
      current.c[j] = sum;
      current.d[j] = sum;
      current.e[j] = sum;
      current.f[j] = sum;
      current.g[j] = sum;
      current.h[j] = sum;
      current.i[j] = sum;
      current.j[j] = sum;

#elif defined(MODE) && MODE == ReadWriteAlternating
      sum += current.a[j];
      current.a[j] = sum;
      sum += current.b[j];
      current.b[j] = sum;
      sum += current.c[j];
      current.c[j] = sum;
      sum += current.d[j];
      current.d[j] = sum;
      sum += current.f[j];
      current.f[j] = sum;
      sum += current.e[j];
      current.e[j] = sum;
      sum += current.g[j];
      current.g[j] = sum;
      sum += current.h[j];
      current.h[j] = sum;
      sum += current.i[j];
      current.i[j] = sum;
      sum += current.j[j];
      current.j[j] = sum;
#endif
    }
  }
  return sum;
}

void FlushCache() {
  const size_t bytes = 100 * 1024 * 1024;  // 100 MB
  const size_t repetitions = 8;
  char* data = new char[bytes];
  for (size_t i = 0; i < repetitions; i++) {  // repetitions
    for (size_t j = 0; j < bytes; j++) {
      data[j] = i + j;
    }
  }
  delete[] data;
}

void BenchmarkSoa(size_t elements, size_t repititions) {
  {
    Soa data(elements);
    FlushCache();
    {
      bdm::Timing timing("SOA 1 attribute:        ");
      volatile double sum = 0;
      for (size_t i = 0; i < repititions; i++) {
        sum += SoaAccessOneAttribute(&data);
      }
#if !defined(MODE) || MODE == Read
      assert(std::abs(sum / 1.05361e+09 - 1) < 1e-5);
#else
      assert(std::abs(sum / 1.56555e+69 - 1) < 1e-5);
#endif
    }
  }
  {
    Soa data(elements);
    FlushCache();
    {
      bdm::Timing timing("SOA all attributes:     ");
      volatile double sum = 0;
      for (size_t i = 0; i < repititions; i++) {
        sum += SoaAccessAllAttributes(&data);
      }
#if !defined(MODE) || MODE == Read
      assert(std::abs(sum / 1.05361e+10 - 1) < 1e-5);
#else
      assert(std::abs(sum / 1.56555e+79 - 1) < 1e-5);
#endif
    }
  }
}

void BenchmarkAosoa(size_t elements, size_t repititions) {
  const size_t vectors = elements / 4;
  Aosoa<4>* data = new Aosoa<4>[vectors];

  FlushCache();
  {
    bdm::Timing timing("AOSOA 1 attribute:      ");
    volatile double sum = 0;
    for (size_t i = 0; i < repititions; i++) {
      sum += AosoaAccessOneAttribute(data, vectors);
    }
#if !defined(MODE) || MODE == Read
    assert(std::abs(sum / 1.05361e+09 - 1) < 1e-5);
#else
    assert(std::abs(sum / 1.56555e+69 - 1) < 1e-5);
#endif
  }

  delete[] data;
  data = new Aosoa<4>[vectors];

  FlushCache();
  {
    bdm::Timing timing("AOSOA all attributes:   ");
    volatile double sum = 0;
    for (size_t i = 0; i < repititions; i++) {
      sum += AosoaAccessAllAttributes(data, vectors);
    }
#if !defined(MODE) || MODE == Read
    assert(std::abs(sum / 1.05361e+10 - 1) < 1e-5);
#else
    assert(std::abs(sum / 1.56555e+79 - 1) < 1e-5);
#endif
  }

  delete[] data;
}

int main(int args, char** argv) {
  if (args == 2 &&
      (std::string(argv[1]) == "help" || std::string(argv[1]) == "--help")) {
    // clang-format off
    std::cout << "SYNOPSIS\n"
              << "  ./soa_aosoa help | --help\n"
              << "\nDESCRIPTION\n"
              << "  Benchmarks two different memory layouts in different settings\n"
              << "  SOA (structure of arrays) and AOSOA (array of structure of arrays)\n"
              << "  Outputs SOA and AOSOA runtime for each scenario\n"
              << "  Two main scenarios:\n"
              << "    * Function accesses ONE data member\n"
              << "    * Function accesses ALL data members of the struct\n"
              << "  There are three variations which can be controlled with the compile time parameter -DMODE=...: \n"
              << "    * Read: data is read and processed (default configuration)\n"
              << "    * ReadWrite: data is read, processed, and written back at the end of an iteration\n"
              << "    * ReadWriteAlternating: reads and writes are alternating between data members in an iteration\n"
              << "  For the \"one data member\" scenario ReadWriteAlternating == ReadWrite\n"
              << "  This binary has been compiled with the following mode: "
#if !defined(MODE) || MODE == Read
              << "Read\n"
#elif defined(MODE) && MODE == ReadWrite
              << "ReadWrite\n"
#elif defined(MODE) && MODE == ReadWriteAlternating
              << "ReadWriteAlternating\n"
#endif
              << "\nREQUIREMENTS\n"
              << "  2.7GB of free memory\n"
              << "\nOPTIONS\n"
              << "  help | --help\n"
              << "    Explains usage of this binary and its command line options\n"
              << std::endl;
    // clang-format on
    return 0;
  }

  std::cout << "Compiled in "
#if !defined(MODE) || MODE == Read
            << "Read mode\n\n";
#elif defined(MODE) && MODE == ReadWrite
            << "ReadWrite mode\n\n";
#elif defined(MODE) && MODE == ReadWriteAlternating
            << "ReadWriteAlternating mode\n\n";
#endif

  const size_t elements = 16777216 * 2;
  const size_t repititions = 10;

  BenchmarkSoa(elements, repititions);
  BenchmarkAosoa(elements, repititions);

  return 0;
}
