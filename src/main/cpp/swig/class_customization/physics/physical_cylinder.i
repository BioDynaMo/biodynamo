/**
 * This file contains code generation customizations for class PhysicalCylinder.
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

%define %PhysicalCylinder_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalCylinder,
                   ini/cx3d/physics/PhysicalCylinder,
                   cx3d::physics::PhysicalCylinder);
%enddef

%define %PhysicalCylinder_java()
  %java_defined_class_add(cx3d::physics::PhysicalCylinder,
                      PhysicalCylinder,
                      PhysicalCylinder,
                      ini.cx3d.physics.PhysicalCylinder,
                      ini/cx3d/physics/PhysicalCylinder,
                      ;);
%enddef

/**
 * apply customizations
 */
%PhysicalCylinder_cx3d_shared_ptr();
%PhysicalCylinder_java();
%typemap(javaimports) cx3d::physics::PhysicalCylinder "import ini.cx3d.localBiology.CellElement;
import ini.cx3d.physics.PhysicalSphere;"
%stdlist_typemap(std::shared_ptr<cx3d::physics::PhysicalCylinder>,
                 PhysicalCylinder,
                 ini.cx3d.physics.PhysicalCylinder);
