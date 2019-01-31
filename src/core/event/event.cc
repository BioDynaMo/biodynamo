// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "core/event/cell_division_event.h"

namespace bdm {

const EventId CellDivisionEvent::kEventId =
    UniqueEventIdFactory::Get()->NewUniqueEventId();

}  // namespace bdm
