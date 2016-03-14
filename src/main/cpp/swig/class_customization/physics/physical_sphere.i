/**
 * This file contains code generation customizations for class PhysicalSphere.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/physical_sphere.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %PhysicalSphere_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalSphere,
                   ini/cx3d/physics/PhysicalSphere,
                   cx3d::physics::PhysicalSphere);
%enddef

%define %PhysicalSphere_java()
  %java_defined_class_add(cx3d::physics::PhysicalSphere,
                      PhysicalSphere,
                      PhysicalSphere,
                      ini.cx3d.physics.PhysicalSphere,
                      ini/cx3d/physics/PhysicalSphere,
                      ;);
%enddef

/**
 * apply customizations
 */
%PhysicalSphere_cx3d_shared_ptr();
%PhysicalSphere_java();
