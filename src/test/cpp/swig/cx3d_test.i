%module(directors="1") cx3d_test

%{
#include <memory>
#include "sim_state_serializable_test.h"
#include "typemap_big_integer_test.h"
#include "typemap_std_array_test.h"
#include "java_defined_class_test.h"
#include "debug_approach_test.h"
using namespace cx3d;
#include "cx3d_testJAVA_wrap.h"
%}

// import depending modules
%import "cx3d.i"

// transpartently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_test);

%include "big_integer_typemap.i"
%include "std_list_typemap.i"
%include "primitives.i"

%double_stdarray_array_marshalling(cx3d_test, 3);
%double_stdarray_2dim_array_marshalling(cx3d_test, 3, 2);

%include "class_customization/java_defined_class_test.i"

%stdlist_typemap(int, Integer, Integer);

// modifications for debug_approach_test
%include "util.i"
%cx3d_shared_ptr(ClassToBeDebugged,
                 ini/cx3d/swig/ClassToBeDebugged,
                 cx3d::ClassToBeDebugged);
%cx3d_shared_ptr(ClassToBeDebuggedDebug,
                 ini/cx3d/swig/ClassToBeDebuggedDebug,
                 cx3d::ClassToBeDebuggedDebug);
%int_stdarray_array_marshalling(cx3d_test, 2);
%add_equals(cx3d::ClassToBeDebugged, ClassToBeDebugged);

// add the original header files here
%include "sim_state_serializable_test.h"
%include "typemap_big_integer_test.h"
%include "typemap_std_array_test.h"
%include "java_defined_class_test.h"
%include "debug_approach_test.h"

namespace cx3d {
  %template(NotPortedTemplatedT_int) NotPortedTemplated<int>;
  %template(PortedTemplatedT_int) PortedTemplated<int>;
}
