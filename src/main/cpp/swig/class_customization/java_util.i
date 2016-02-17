/**
 * This file contains code generation customizations for class JavaUtil.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/java_util.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %JavaUtil_cx3d_shared_ptr()
  %cx3d_shared_ptr(JavaUtilT_PhysicalNode,
                   ini/cx3d/JavaUtil,
                   cx3d::JavaUtil<cx3d::PhysicalNode>);
%enddef

%define %JavaUtil_java()
  %java_defined_class(cx3d::JavaUtil<cx3d::PhysicalNode>,
                      JavaUtilT_PhysicalNode,
                      JavaUtil,
                      ini.cx3d.JavaUtil,
                      ini/cx3d/JavaUtil);
%enddef

/**
 * apply customizations
 */
%JavaUtil_cx3d_shared_ptr();
%JavaUtil_java();
