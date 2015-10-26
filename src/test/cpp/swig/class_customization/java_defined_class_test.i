/**
 * This file contains code generation customizations for class NotPorted and
 * NotPortedTemplated.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it using:
 * %include "class_customization/java_defined_class_test.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_array_typemap.i"
%include "std_list_typemap.i"

%define %NotPorted_cx3d_shared_ptr()
  %cx3d_shared_ptr(NotPorted,
                   ini/cx3d/swig/NotPortedT_int,
                   cx3d::NotPorted);
%enddef

%define %NotPorted_java()
  %java_defined_class(cx3d::NotPorted,
                      NotPorted,
                      NotPorted,
                      ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedJavaImpl,
                      ini/cx3d/swigTests/JavaDefinedClassTest/NotPortedJavaImpl);
%enddef

%define %NotPorted_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                             std::shared_ptr<cx3d::NotPorted>,
                             shared_ptr_NotPorted_##SIZE,
                             ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedJavaImpl,
                             Lini/cx3d/swigTests/JavaDefinedClassTest/NotPortedJavaImpl;, SIZE);
%enddef

%define %NotPorted_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::NotPorted>,
                   NotPorted, ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedJavaImpl);
%enddef

/**
 * apply customizations
 */
%NotPorted_cx3d_shared_ptr();
%NotPorted_java();
%NotPorted_stdarray_array_marshalling(cx3d_test, 2);
%NotPorted_stdlist();

//------------------------------------------------------------------------------
// NotPortedTemplated

%define %NotPortedTemplated_cx3d_shared_ptr()
  %cx3d_shared_ptr(NotPortedTemplatedT_int,
                   ini/cx3d/swig/NotPortedTemplatedT_int,
                   cx3d::NotPortedTemplated<int>);
%enddef

%define %NotPortedTemplated_java()
  %java_defined_class(cx3d::NotPortedTemplated<int>,
                      NotPortedTemplatedT_int,
                      NotPortedTemplated,
                      ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedTemplatedJavaImpl,
                      ini/cx3d/swigTests/JavaDefinedClassTest/NotPortedTemplatedJavaImpl);
%enddef

%define %NotPortedTemplated_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::NotPortedTemplated<int> >,
                              shared_ptr_NotPortedTemplated_int##SIZE,
                              ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedTemplatedJavaImpl,
                              Lini/cx3d/swigTests/JavaDefinedClassTest/NotPortedTemplatedJavaImpl;, SIZE);
%enddef

%define %NotPortedTemplated_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::NotPortedTemplated<int> >,
                   NotPortedTemplatedT_int,
                   ini.cx3d.swigTests.JavaDefinedClassTest.NotPortedTemplatedJavaImpl);
%enddef

/**
 * apply customizations
 */
%NotPortedTemplated_cx3d_shared_ptr();
%NotPortedTemplated_java();
%NotPortedTemplated_stdarray_array_marshalling(cx3d_test, 2);
%NotPortedTemplated_stdlist();
