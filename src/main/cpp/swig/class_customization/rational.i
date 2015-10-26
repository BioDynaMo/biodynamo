/**
 * This file contains code generation customizations for class Rational.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on preprocessor
 * variable: RATIONAL_NATIVE
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it using:
 * %include "class_customization/tetrahedron.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_array_typemap.i"

%define %Rational_ported_type_modification()
  %ported_type_modification(cx3d::spatial_organization::Rational,
                            ini.cx3d.spatialOrganization.interfaces.Rational);
%enddef

%define %Rational_cx3d_shared_ptr()
  %cx3d_shared_ptr(Rational,
                   ini/cx3d/spatialOrganization/interfaces/Rational,
                   cx3d::spatial_organization::Rational);
%enddef

%define %Rational_java()
  %java_defined_class(cx3d::spatial_organization::Rational,
                      Rational,
                      Rational,
                      ini.cx3d.spatialOrganization.interfaces.Rational,
                      ini/cx3d/spatialOrganization/interfaces/Rational);
  %typemap(javainterfaces) cx3d::spatial_organization::Rational "ini.cx3d.spatialOrganization.interfaces.Rational"
%enddef

%define %Rational_native()
  %native_defined_class(cx3d::spatial_organization::Rational,
                        Rational,
                        ini.cx3d.spatialOrganization.interfaces.Rational,
                        Rational,
                        public Rational(){});
%enddef

%define %Rational_typemaps()
 // for class ExactVector
 %Rational_stdarray_array_marshalling(spatialOrganization, 3);
%enddef

%define %Rational_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::spatial_organization::Rational>,
                              shared_ptr_Rational_##SIZE,
                              ini.cx3d.spatialOrganization.interfaces.Rational,
                              Lini/cx3d/spatialOrganization/interfaces/Rational;,
                              SIZE);
%enddef

/**
 * apply customizations
 */
%Rational_cx3d_shared_ptr();
#ifdef RATIONAL_NATIVE
  %Rational_native();
#else
  %Rational_java();
#endif
%Rational_typemaps();
