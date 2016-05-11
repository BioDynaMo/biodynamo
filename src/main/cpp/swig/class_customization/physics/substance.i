/**
 * This file contains code generation customizations for class Substance.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (SUBSTANCE_NATIVE and SUBSTANCE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/substance.i"
 */

%include "util.i"
%include "std_list_typemap.i"
%include "cx3d_shared_ptr.i"

%define %Substance_cx3d_shared_ptr()
  %cx3d_shared_ptr(Substance,
                   ini/cx3d/physics/interfaces/Substance,
                   cx3d::physics::Substance);
%enddef

%define %Substance_java()
  %java_defined_class(cx3d::physics::Substance,
                      Substance,
                      Substance,
                      ini.cx3d.physics.interfaces.Substance,
                      ini/cx3d/physics/interfaces/Substance);
%enddef

%define %Substance_native()
  %native_defined_class(cx3d::physics::Substance,
                            Substance,
                            ini.cx3d.physics.interfaces.Substance,
                            Substance,
                            ;);
%enddef

%define %Substance_stdlist()
  %stdlist_typemap_cross_module(std::shared_ptr<cx3d::physics::Substance>,
                                Substance,
                                ini.cx3d.physics.interfaces.Substance,
                                ini.cx3d.swig.simulation.Substance);
%enddef

%define %Substance_typemaps()
  %Substance_stdlist();
  %typemap(javainterfaces) cx3d::physics::Substance "ini.cx3d.physics.interfaces.Substance"
  %typemap(javaimports) cx3d::physics::Substance "import ini.cx3d.swig.NativeStringBuilder;"
%enddef

 /**
  * apply customizations
  */
 %Substance_cx3d_shared_ptr();
 #ifdef SUBSTANCE_NATIVE
   %Substance_native();
 #else
   %Substance_java();
 #endif
 #ifdef SUBSTANCE_DEBUG
   %setJavaDebugSwitch(Substance, true);
 #else
   %setJavaDebugSwitch(Substance, false);
 #endif
 %Substance_typemaps();
