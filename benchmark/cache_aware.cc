/// Benchmarks SOA and AOSOA memory layout in different scenarios
/// For more information see help message in function `main`
#include <assert.h>

#include <cmath>
#include <iostream>
#include <string>
#include <sstream>

#include "timing.h"


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

void BenchmarkNormal(const size_t elements, const size_t repititions) {
  auto data = new size_t[elements];
  for(size_t i = 0; i < elements; i++) {
    data[i] = i;
  }
  FlushCache();
  {
    bdm::Timing timing("normal:        ");
    // volatile register int sum = 0;
    for (size_t i = 0; i < repititions; i++) {
      for(size_t j = 0; j < elements; j++) {
        // data[j]++;
        // data[j] = std::exp(data[j]);
        data[j] /= 2;
        // data[j] = i + j;
        // data[j] = data[(j - 1 > 0 ? j - 1 : 0)] + data[(j + 1 < elements ? j + 1 : elements - 1)];
        // sum += data[j];
      }
    }
    // std::cout << sum << std::endl;
  }
  delete[] data;
}

void BenchmarkCacheAware(const size_t elements, const size_t repititions, const size_t batch_size) {
  auto* data = new size_t[elements];
  for(size_t i = 0; i < elements; i++) {
    data[i] = i;
  }
  FlushCache();
  {
    bdm::Timing timing("cachea:        ");
    // volatile int sum = 0;
    asm("# start cachea");
    for (size_t k = 0; k < elements / batch_size; k++) {
      // for(size_t j = k * batch_size; j < (k+1) * batch_size; j++) {
      //   __builtin_prefetch((const void*)(&data[j]),0,0);
      // }
      const size_t from = k * batch_size;
      const size_t to = from + batch_size;
      for (size_t i = 0; i < repititions; i++) {
        for(size_t j = from; j < to; j++) {
          // data[j]++;
          // data[j] = std::exp(data[j]);
          data[j] /= 2;
          // data[j] = i + j;
          // data[j] = data[(j - 1 > 0 ? j - 1 : 0)] + data[(j + 1 < elements ? j + 1 : elements - 1)];
          // sum += data[j];
        }
      }
    }
    asm("# stop cachea");
    // std::cout << sum << std::endl;
  }
  delete[] data;
}

int main(int args, char** argv) {
  size_t data_mbyte = 4096;
  size_t repititions = 3;
  size_t batch_size_kbyte = 2048;

  if(args == 4) {
    std::istringstream(std::string(argv[1])) >> data_mbyte;
    std::istringstream(std::string(argv[2])) >> repititions;
    std::istringstream(std::string(argv[3])) >> batch_size_kbyte;
  }

  const size_t elements = data_mbyte * 1024 * 1024 / sizeof(size_t);
  const size_t batch_size = batch_size_kbyte * 1024 / sizeof(size_t);

  std::cout << "data_mbyte: " << data_mbyte << std::endl
            << "repititions: " << repititions << std::endl
            << "batch_size_kbyte: " << batch_size_kbyte << std::endl;
  BenchmarkNormal(elements, repititions);
  BenchmarkCacheAware(elements, repititions, batch_size);

  return 0;
}

// perf stat -e LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,L1-dcache-loads,L1-dcache-load-misses ./cache_aware
// perf stat -e instructions -e r01A2 ./cache_aware
// rXXYY XX umask  YY event code
// r01A2 Resource_Stall page467 http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.pdf
// r00c0 instructions retired = -e instructions
// r012c UNC_QMC_NORMAL_READS.ANY
