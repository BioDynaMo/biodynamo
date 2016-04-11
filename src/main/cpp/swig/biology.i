%module(directors="1") biology

%include "util.i"
%include "config.i"

%{
#include <memory>
#include "local_biology/abstract_local_biology_module.h"
#include "local_biology/cell_element.h"
#include "local_biology/soma_element.h"
#include "local_biology/neurite_element.h"
#include "cells/cell.h"
using namespace cx3d::local_biology;
using cx3d::cells::Cell;
%}

// import depending modules
%import "cx3d.i"

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_biology);

// typemap definitions, code modifications / additions
%include "primitives.i"
%double_stdarray_array_marshalling(biology, 3);
%pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder;
import ini.cx3d.swig.biology.CellElement;
import ini.cx3d.swig.biology.LocalBiologyModule;
import ini.cx3d.swig.physics.PhysicalObject;
import ini.cx3d.swig.physics.PhysicalCylinder;"

// class modifications
%include "class_customization/physics/physical_object.i"
%include "class_customization/physics/physical_cylinder.i"
%include "class_customization/local_biology/cell_element.i"
%include "class_customization/local_biology/soma_element.i"
%include "class_customization/local_biology/neurite_element.i"
%include "class_customization/cells/cell.i"
%include "class_customization/local_biology/local_biology_module.i"
%include "class_customization/local_biology/abstract_local_biology_module.i"
%include "class_customization/physics/ecm.i"

// add the original header files here
%include "local_biology/local_biology_module.h"
%include "local_biology/abstract_local_biology_module.h"
%include "local_biology/cell_element.h"
%include "local_biology/soma_element.h"
%include "local_biology/neurite_element.h"
%include "cells/cell.h"
