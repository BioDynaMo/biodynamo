%module(directors="1") physics

%include "util.i"
// administration area

%native(SUBSTANCE);
%native(INTRACELLULARSUBSTANCE);

// end administration area

%{
#include <memory>
#include <stdint.h>

#include "physics/substance.h"
#include "physics/intracellular_substance.h"
#include "physics/physical_node.h"

// using namespace cx3d::physics;
// #include "physicsJAVA_wrap.h"
%}

// import depending modules
%import "cx3d.i"

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_physics);

// typemap definitions, code modifications / additions
%include "primitives.i"
%include "color_typemap.i"

// class modifications
%include "class_customization/physics/substance.i"
%include "class_customization/physics/intracellular_substance.i"
%include "class_customization/physics/physical_node.i"

// add the original header files here
%include "physics/substance.h"
%include "physics/intracellular_substance.h"
%include "physics/physical_node.h"
