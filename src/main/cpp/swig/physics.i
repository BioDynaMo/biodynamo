%module(directors="1") physics

%include "util.i"
%include "config.i"

%{
#include <memory>
#include <stdint.h>

#include "physics/substance.h"
#include "physics/intracellular_substance.h"
#include "physics/ecm.h"
#include "physics/physical_node.h"
#include "physics/physical_node_movement_listener.h"
#include "physics/physical_object.h"
#include "physics/physical_cylinder.h"
#include "physics/physical_bond.h"
using namespace cx3d::physics;
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
%pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder; import ini.cx3d.swig.spatialOrganization.SpaceNodeT_PhysicalNode;"

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

// add the original header files here
%include "physics/substance.h"
%include "physics/intracellular_substance.h"
%include "physics/ecm.h"
%include "physics/physical_node.h"
%include "physics/physical_node_movement_listener.h"
%include "physics/physical_object.h"
%include "physics/physical_cylinder.h"
%include "physics/physical_bond.h"
