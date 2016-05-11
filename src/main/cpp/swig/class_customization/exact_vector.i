/**
 * This file contains code generation customizations for class ExactVector.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on preprocessor
 * variable: EXACTVECTOR_NATIVE
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it using:
 * %include "class_customization/exact_vector.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_array_typemap.i"

%define %ExactVector_cx3d_shared_ptr()
  %cx3d_shared_ptr(ExactVector,
                   ini/cx3d/spatialOrganization/interfaces/ExactVector,
                   cx3d::spatial_organization::ExactVector);
%enddef

%define %ExactVector_java()
  %java_defined_class(cx3d::spatial_organization::ExactVector,
                      ExactVector,
                      ExactVector,
                      ini.cx3d.spatialOrganization.interfaces.ExactVector,
                      ini/cx3d/spatialOrganization/interfaces/ExactVector);
  %typemap(javainterfaces) cx3d::spatial_organization::ExactVector "ini.cx3d.spatialOrganization.interfaces.ExactVector"
%enddef

%define %ExactVector_native()
  %native_defined_class(cx3d::spatial_organization::ExactVector,
                        ExactVector,
                        ini.cx3d.spatialOrganization.interfaces.ExactVector,
                        ExactVector,
                        public ExactVector(){});
%enddef

%define %ExactVector_typemaps()
  %ExactVector_stdarray_array_marshalling(simulation, 3);
%enddef

%define %ExactVector_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::spatial_organization::ExactVector>,
                              shared_ptr_ExactVector_##SIZE,
                              ini.cx3d.spatialOrganization.interfaces.ExactVector,
                              Lini/cx3d/spatialOrganization/interfaces/ExactVector;,
                              SIZE);
%enddef

/**
 * apply customizations
 */
%ExactVector_cx3d_shared_ptr();
#ifdef EXACTVECTOR_NATIVE
  %ExactVector_native();
#else
  %ExactVector_java();
#endif
%ExactVector_typemaps();
