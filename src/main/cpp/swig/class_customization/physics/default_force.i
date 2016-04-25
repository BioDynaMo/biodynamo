/**
 * This file contains code generation customizations for class DefaultForce.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (DEFAULTFORCE_NATIVE and DEFAULTFORCE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/default_force.i"
 */

%include "util.i"
%include "std_list_typemap.i"
%include "cx3d_shared_ptr.i"

%define %DefaultForce_cx3d_shared_ptr()
  %cx3d_shared_ptr(DefaultForce,
                   ini/cx3d/physics/interfaces/DefaultForce,
                   cx3d::physics::DefaultForce);
%enddef

%define %DefaultForce_java()
  %java_defined_class(cx3d::physics::DefaultForce,
                      DefaultForce,
                      DefaultForce,
                      ini.cx3d.physics.InterObjectForce,
                      ini/cx3d/physics/interfaces/DefaultForce);
%enddef

%define %DefaultForce_native()
  %native_defined_class(cx3d::physics::DefaultForce,
                            DefaultForce,
                            ini.cx3d.physics.InterObjectForce,
                            DefaultForce,
                            ;);
%enddef

%define %DefaultForce_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::physics::DefaultForce>,
                   DefaultForce,
                   ini.cx3d.physics.InterObjectForce);
%enddef

%define %DefaultForce_typemaps()
  %DefaultForce_stdlist();
  %typemap(javainterfaces) cx3d::physics::DefaultForce "ini.cx3d.physics.InterObjectForce"
  %typemap(javaimports) cx3d::physics::DefaultForce %{
    import ini.cx3d.swig.NativeStringBuilder;
    import ini.cx3d.swig.biology.JavaUtil2;

%}
%enddef

 /**
  * apply customizations
  */
 %DefaultForce_cx3d_shared_ptr();
 #ifdef DEFAULTFORCE_NATIVE
   %DefaultForce_native();
 #else
   %DefaultForce_java();
 #endif
 #ifdef DEFAULTFORCE_DEBUG
   %setJavaDebugSwitch(DefaultForce, true);
 #else
   %setJavaDebugSwitch(DefaultForce, false);
 #endif
 %DefaultForce_typemaps();
