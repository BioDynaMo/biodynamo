/**
 * This file contains code generation customizations for class JavaUtil2.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/java_util_2.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %JavaUtil2_cx3d_shared_ptr()
  %cx3d_shared_ptr(JavaUtil2,
                   ini/cx3d/JavaUtil2,
                   cx3d::JavaUtil2);
%enddef

%define %JavaUtil2_java()
  %java_defined_class(cx3d::JavaUtil2,
                      JavaUtil2,
                      JavaUtil2,
                      ini.cx3d.JavaUtil2,
                      ini/cx3d/JavaUtil2);
%enddef

/**
 * apply customizations
 */
%JavaUtil2_cx3d_shared_ptr();
%JavaUtil2_java();

%typemap(javaimports) cx3d::JavaUtil2 %{
  import ini.cx3d.swig.physics.PhysicalBond;
  import ini.cx3d.swig.physics.PhysicalCylinder;
  import ini.cx3d.swig.physics.PhysicalObject;
  import ini.cx3d.swig.physics.PhysicalSphere;
%}
