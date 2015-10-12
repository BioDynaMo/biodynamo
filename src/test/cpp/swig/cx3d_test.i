%module(directors="1") cx3d_test

%{
#include <memory>
#include "sim_state_serializable_test.h"
#include "typemap_big_integer_test.h"
#include "typemap_std_array_test.h"
#include "java_defined_class_test.h"
using namespace cx3d;
#include "cx3d_testJAVA_wrap.h"
%}

// import depending modules
%import "cx3d.i"

// transpartently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_test);

%include "big_integer_typemap.i"

%include "partial_macro_application/double.i"
%double_stdarray_array_marshalling(cx3d_test, 3);
%double_stdarray_2dim_array_marshalling(cx3d_test, 3, 2);

%include "partial_macro_application/java_defined_class_test.i"
%NotPorted_cx3d_shared_ptr();
%NotPorted_jdc_enable();
%NotPorted_jdc_get(getNotPorted);
%NotPorted_jdc_array_extension(2);
%NotPorted_stdarray_array_marshalling(cx3d_test, 2);
%NotPorted_jdc_get_array(2, getNotPortedArray);

%NotPortedTemplated_cx3d_shared_ptr();
%NotPortedTemplated_jdc_enable();
%NotPortedTemplated_jdc_get(getNotPortedTemplated);
%NotPortedTemplated_jdc_array_extension(2);
%NotPortedTemplated_stdarray_array_marshalling(cx3d_test, 2);
%NotPortedTemplated_jdc_get_array(2, getNotPortedTemplatedArray);

// add the original header files here
%include "sim_state_serializable_test.h"
%include "typemap_big_integer_test.h"
%include "typemap_std_array_test.h"
%include "java_defined_class_test.h"

namespace cx3d {
  %template(NotPortedTemplatedT_int) NotPortedTemplated<int>;
  %template(PortedTemplatedT_int) PortedTemplated<int>;
}
