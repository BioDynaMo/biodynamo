%module(directors="1") biology

%include "util.i"
%include "config.i"

%{
#include <memory>
#include "local_biology/abstract_local_biology_module.h"
using namespace cx3d::local_biology;
%}

// import depending modules
%import "cx3d.i"

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_biology);

// typemap definitions, code modifications / additions
%include "primitives.i"
%pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder;
import ini.cx3d.swig.physics.CellElement; 
import ini.cx3d.swig.biology.LocalBiologyModule;"

// class modifications
%include "class_customization/local_biology/cell_element.i"
%include "class_customization/local_biology/local_biology_module.i"
%include "class_customization/local_biology/abstract_local_biology_module.i"

// add the original header files here
%include "local_biology/local_biology_module.h"
%include "local_biology/abstract_local_biology_module.h"
