/**
 * This file contains code generation customizations for class InterObjectForce.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (INTEROBJECTFORCE_NATIVE and INTEROBJECTFORCE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/inter_object_force.i"
 */

%include "util.i"
%include "std_list_typemap.i"
%include "cx3d_shared_ptr.i"

%define %InterObjectForce_cx3d_shared_ptr()
  %cx3d_shared_ptr(InterObjectForce,
                   ini/cx3d/physics/interfaces/InterObjectForce,
                   cx3d::physics::InterObjectForce);
%enddef

%define %InterObjectForce_java()
  %java_defined_class(cx3d::physics::InterObjectForce,
                      InterObjectForce,
                      InterObjectForce,
                      ini.cx3d.physics.InterObjectForce,
                      ini/cx3d/physics/InterObjectForce);
%enddef

%define %InterObjectForce_native()
  %native_defined_class(cx3d::physics::InterObjectForce,
                            InterObjectForce,
                            ini.cx3d.physics.InterObjectForce,
                            InterObjectForce,
                            ;);
%enddef

%define %InterObjectForce_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::physics::InterObjectForce>,
                   InterObjectForce,
                   ini.cx3d.physics.InterObjectForce);
%enddef

%define %InterObjectForce_typemaps()
  %InterObjectForce_stdlist();
  %typemap(javainterfaces) cx3d::physics::InterObjectForce "ini.cx3d.physics.InterObjectForce"
  %typemap(javaimports) cx3d::physics::InterObjectForce "import ini.cx3d.swig.NativeStringBuilder;"
%enddef

 /**
  * apply customizations
  */
 %InterObjectForce_cx3d_shared_ptr();
 #ifdef INTEROBJECTFORCE_NATIVE
   %InterObjectForce_native();
 #else
   %InterObjectForce_java();
 #endif
 #ifdef INTEROBJECTFORCE_DEBUG
   %setJavaDebugSwitch(InterObjectForce, true);
 #else
   %setJavaDebugSwitch(InterObjectForce, false);
 #endif
 %InterObjectForce_typemaps();
