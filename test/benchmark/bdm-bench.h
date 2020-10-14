#ifndef BDM_BENCH_H
#define BDM_BENCH_H

#include <benchmark.h>

void BM_BDM_SOMA(benchmark::State& state);
void BM_BDM_TUMOR(benchmark::State& state);

#endif