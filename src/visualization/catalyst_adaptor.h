#ifndef DEMO_BDMADAPTOR_H_
#define DEMO_BDMADAPTOR_H_

#include "cell.h"

using bdm::Cell;
using bdm::Soa;

namespace bdm_adaptor {
void Initialize(const std::string& script);

void Finalize();

void CoProcess(Cell<Soa>* cells, double time, size_t time_step,
               bool last_time_step);
}

#endif  // DEMO_BDMADAPTOR_H_
