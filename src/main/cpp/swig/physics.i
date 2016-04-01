%module(directors="1") physics

%include "util.i"
%include "config.i"

%{
#include <memory>
#include <stdint.h>

#include "color.h"
#include "physics/substance.h"
#include "physics/intracellular_substance.h"
#include "physics/ecm.h"
#include "physics/physical_node.h"
#include "physics/physical_node_movement_listener.h"
#include "physics/physical_object.h"
#include "physics/physical_cylinder.h"
#include "physics/physical_bond.h"
#include "physics/collision_check.h"
#include "physics/physical_sphere.h"
#include "java_util.h"
#include "physics/default_force.h"
#include "synapse/excrescence.h"
#include "local_biology/cell_element.h"
#include "local_biology/soma_element.h"
#include "local_biology/neurite_element.h"
#include "param.h"
using namespace cx3d::physics;
using cx3d::JavaUtil2;
using cx3d::Color;
using cx3d::synapse::Excrescence;
using cx3d::local_biology::CellElement;
%}

// import depending modules
%import "cx3d.i"

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_physics);

// typemap definitions, code modifications / additions
%include "primitives.i"
%include "color_typemap.i"
%double_stdarray_array_marshalling(physics, 2);
%double_stdarray_array_marshalling(physics, 3);
%double_stdarray_array_marshalling(physics, 4);
%pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder; import ini.cx3d.swig.spatialOrganization.SpaceNodeT_PhysicalNode; import ini.cx3d.swig.biology.LocalBiologyModule;"

// class modifications
%include "class_customization/physics/substance.i"
%include "class_customization/physics/intracellular_substance.i"
%include "class_customization/space_node.i"
%include "class_customization/physics/ecm.i"
%include "class_customization/physics/physical_node.i"
%include "class_customization/physics/physical_node_movement_listener.i"
%include "class_customization/physics/physical_object.i"
%include "class_customization/physics/physical_cylinder.i"
%include "class_customization/physics/physical_bond.i"
%include "class_customization/physics/physical_sphere.i"
%include "class_customization/java_util_2.i"
%include "class_customization/physics/inter_object_force.i"
%include "class_customization/physics/default_force.i"
%include "class_customization/synapse/excrescence.i"
%include "class_customization/local_biology/cell_element.i"
%include "class_customization/local_biology/soma_element.i"
%include "class_customization/local_biology/neurite_element.i"
%include "class_customization/local_biology/local_biology_module.i"
%ignore cx3d::Param::kViolet;

// add the original header files here
%include "color.h"
%include "physics/substance.h"
%include "physics/intracellular_substance.h"
%include "physics/ecm.h"
%include "physics/physical_node.h"
%include "physics/physical_node_movement_listener.h"
%include "physics/physical_object.h"
%include "physics/physical_cylinder.h"
%include "physics/physical_bond.h"
%include "physics/collision_check.h"
%include "physics/physical_sphere.h"
%include "java_util.h"
%include "physics/inter_object_force.h"
%include "physics/default_force.h"
%include "synapse/excrescence.h"
%include "local_biology/cell_element.h"
%include "local_biology/soma_element.h"
%include "local_biology/neurite_element.h"
%include "param.h"
