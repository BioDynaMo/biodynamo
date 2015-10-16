/**
 * This file contains partial macro applications for a specific type. It means, that it
 * defines a new macro and fixes the value of a number of arguments.
 * As a result it reduces the arity of the underlying macro.
 * Therefore, this macros are easier to use and result in a better readable
 * module file.
 * https://en.wikipedia.org/wiki/Partial_application
 *
 * e.g. given complex_macro(arg1, arg2, arg3, arg4, arg5);
 * set arg1=1, arg2=2, arg4=4, arg5=5:
 * -> partial macro:
 * %define %simplified(arg3)
 *    complex_macro(1, 2, arg3, 4, 5)
 * %enddef
 *
 * Documentation of macros and their arguments can be found in the included
 * files!
 */

%include "std_array_typemap.i"
%include "java_defined_class.i"
%include "cx3d_shared_ptr.i"

%define %NotPorted_cx3d_shared_ptr()
  %cx3d_shared_ptr(NotPortedCppType,
                   ini/cx3d/swig/NotPortedT_intCppType,
                   cx3d::NotPorted);
%enddef

%define %NotPorted_jdc_enable()
  %jdc_enable(cx3d, NotPorted);
%enddef

%define %NotPorted_jdc_get(METHOD_NAME)
  %jdc_get(cx3d::NotPorted, METHOD_NAME,
      ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedJavaImpl,
      cx3d_NotPorted_);
%enddef

%define %NotPorted_jdc_array_extension(SIZE)
  %jdc_array_extension(cx3d::NotPorted, getJava_impl,
                       ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedJavaImpl,
                       shared_ptr_NotPorted_##SIZE,
                       cx3d_NotPorted_);
%enddef

%define %NotPorted_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                             std::shared_ptr<cx3d::NotPorted>,
                             shared_ptr_NotPorted_##SIZE,
                             ini.cx3d.swig.NotPortedCppType,
                             Lini/cx3d/swig/NotPortedCppType;, SIZE);
%enddef

%define %NotPorted_jdc_get_array(SIZE, METHOD_NAME)
  %jdc_get_array(std::shared_ptr<cx3d::NotPorted>, SIZE, METHOD_NAME,
                 ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedJavaImpl,
                 shared_ptr_NotPorted_##SIZE);
%enddef

%define %NotPortedTemplated_cx3d_shared_ptr()
  %cx3d_shared_ptr(NotPortedTemplatedT_intCppType,
                   ini/cx3d/swig/NotPortedTemplatedT_intCppType,
                   cx3d::NotPortedTemplated<int>);
%enddef

%define %NotPortedTemplated_jdc_enable()
  %jdc_enable_templated(cx3d, NotPortedTemplated, <int>, NotPortedTemplatedT_int);
%enddef

%define %NotPortedTemplated_jdc_get(METHOD_NAME)
  %jdc_get(cx3d::NotPortedTemplated<int>, METHOD_NAME,
           ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedTemplatedJavaImpl<Integer>,
           cx3d_NotPortedTemplated_Sl_int_Sg__);
%enddef

%define %NotPortedTemplated_jdc_array_extension(SIZE)
  %jdc_array_extension_templated(cx3d::NotPortedTemplated<int>, getJava_impl,
                                 ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedTemplatedJavaImpl, Integer,
                                 shared_ptr_NotPortedTemplated_##SIZE, cx3d_NotPortedTemplated_Sl_int_Sg__);
%enddef

%define %NotPortedTemplated_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::NotPortedTemplated<int> >,
                              shared_ptr_NotPortedTemplated_##SIZE,
                              ini.cx3d.swig.NotPortedTemplatedT_intCppType,
                              Lini/cx3d/swig/NotPortedTemplatedT_intCppType;, SIZE);
%enddef

%define %NotPortedTemplated_jdc_get_array(SIZE, METHOD_NAME)
  %jdc_get_array(std::shared_ptr<cx3d::NotPortedTemplated<int>>, SIZE, METHOD_NAME,
                                 ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedTemplatedJavaImpl<Integer>,
                                 shared_ptr_NotPortedTemplated_##SIZE);
%enddef
