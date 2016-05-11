/**
 * This file contains code generation customizations for class PhysicalBond.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/physical_object.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %PhysicalBond_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalBond,
                   ini/cx3d/physics/interfaces/PhysicalBond,
                   cx3d::physics::PhysicalBond);
%enddef

%define %PhysicalBond_java()
  %java_defined_class(cx3d::physics::PhysicalBond,
                      PhysicalBond,
                      PhysicalBond,
                      ini.cx3d.physics.interfaces.PhysicalBond,
                      ini/cx3d/physics/interfaces/PhysicalBond);
%enddef

%define %PhysicalBond_native()
  %native_defined_class(cx3d::physics::PhysicalBond,
                      PhysicalBond,
                      ini.cx3d.physics.interfaces.PhysicalBond,
                      PhysicalBond,
                      ;);
%enddef

/**
 * apply customizations
 */
%PhysicalBond_cx3d_shared_ptr();
#ifdef PHYSICALBOND_NATIVE
  %PhysicalBond_native();
#else
  %PhysicalBond_java();
#endif
#ifdef PHYSICALBOND_DEBUG
  %setJavaDebugSwitch(PhysicalBond, true);
#else
  %setJavaDebugSwitch(PhysicalBond, false);
#endif
%typemap(javaimports) cx3d::physics::PhysicalBond "import ini.cx3d.swig.NativeStringBuilder;"
%typemap(javainterfaces) cx3d::physics::PhysicalBond "ini.cx3d.physics.interfaces.PhysicalBond"
%stdlist_typemap_cross_module(std::shared_ptr<cx3d::physics::PhysicalBond>,
                              PhysicalBond,
                              ini.cx3d.physics.interfaces.PhysicalBond,
                              ini.cx3d.swig.simulation.PhysicalBond);
