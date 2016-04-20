%module(directors="1") biology

%include "util.i"
%include "config.i"

%{
#include <memory>
#include "color.h"
#include "local_biology/abstract_local_biology_module.h"
#include "local_biology/cell_element.h"
#include "local_biology/soma_element.h"
#include "local_biology/neurite_element.h"
#include "physics/physical_object.h"
#include "physics/physical_sphere.h"
#include "cells/cell.h"
#include "synapse/excrescence.h"
#include "synapse/physical_spine.h"
#include "synapse/physical_bouton.h"
#include "synapse/physical_somatic_spine.h"
#include "synapse/biological_spine.h"
#include "synapse/biological_bouton.h"
#include "synapse/biological_somatic_spine.h"
#include "synapse/connection_maker.h"
#include "synapse/test_synapses.h"
#include "cells/cell_module.h"
using namespace cx3d::local_biology;
using namespace cx3d::cells;
using cx3d::physics::PhysicalObject;
using cx3d::physics::PhysicalSphere;
using cx3d::physics::PhysicalCylinder;
using cx3d::Color;
%}

// import depending modules
%import "cx3d.i"

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_biology);

// typemap definitions, code modifications / additions
%include "primitives.i"
%double_stdarray_array_marshalling(biology, 2);
%double_stdarray_array_marshalling(biology, 3);
%include "color_typemap.i"
%color(biology);
%pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder;
import ini.cx3d.swig.biology.CellElement;
import ini.cx3d.swig.biology.LocalBiologyModule;
import ini.cx3d.swig.physics.PhysicalObject;
import ini.cx3d.swig.physics.PhysicalSphere;
import ini.cx3d.swig.physics.PhysicalCylinder;"

// class modifications
%include "class_customization/local_biology/cell_element.i"
%include "class_customization/local_biology/local_biology_module.i"
%include "class_customization/local_biology/abstract_local_biology_module.i"
%include "class_customization/physics/physical_object.i"
%include "class_customization/physics/physical_sphere.i"
%include "class_customization/physics/physical_cylinder.i"
%include "class_customization/local_biology/soma_element.i"
%include "class_customization/local_biology/neurite_element.i"
%include "class_customization/cells/cell.i"
%include "class_customization/physics/ecm.i"
%include "class_customization/synapse/excrescence.i"
%include "class_customization/synapse/physical_spine.i"
%include "class_customization/synapse/physical_somatic_spine.i"
%include "class_customization/synapse/physical_bouton.i"
%include "class_customization/synapse/biological_spine.i"
%include "class_customization/synapse/biological_bouton.i"
%include "class_customization/synapse/biological_somatic_spine.i"
%include "class_customization/synapse/connection_maker.i"
%include "class_customization/synapse/test_synapses.i"
%include "class_customization/cells/cell_module.i"

// add the original header files here
%include "local_biology/local_biology_module.h"
%include "local_biology/abstract_local_biology_module.h"
%include "local_biology/cell_element.h"
%include "local_biology/soma_element.h"
%include "local_biology/neurite_element.h"
%include "cells/cell.h"
%include "synapse/excrescence.h"
%include "synapse/physical_spine.h"
%include "synapse/physical_bouton.h"
%include "synapse/physical_somatic_spine.h"
%include "synapse/biological_spine.h"
%include "synapse/biological_bouton.h"
%include "synapse/biological_somatic_spine.h"
%include "synapse/connection_maker.h"
%include "synapse/test_synapses.h"
%include "cells/cell_module.h"
%include "color.h"
