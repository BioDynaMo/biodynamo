/**
 * This file contains code generation customizations for class OpenTriangleOrganizer.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (OPENTRIANGLEORGNIZER_NATIVE and OPENTRIANGLEORGNIZER_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/open_triangle_organizer.i"
 */

%include "util.i"
%include "std_array_typemap.i"
%include "cx3d_shared_ptr.i"

%define %OpenTriangleOrganizer_cx3d_shared_ptr()
  %cx3d_shared_ptr(OpenTriangleOrganizerT_PhysicalNode,
                   ini/cx3d/spatialOrganization/OpenTriangleOrganizer,
                   cx3d::spatial_organization::OpenTriangleOrganizer<cx3d::PhysicalNode>);
%enddef

%define %OpenTriangleOrganizer_java()
  %java_defined_class(cx3d::spatial_organization::OpenTriangleOrganizer<cx3d::PhysicalNode>,
                      OpenTriangleOrganizerT_PhysicalNode,
                      OpenTriangleOrganizer,
                      ini.cx3d.spatialOrganization.OpenTriangleOrganizer,
                      ini/cx3d/spatialOrganization/OpenTriangleOrganizer);
%enddef

/**
 * apply customizations
 */
%OpenTriangleOrganizer_cx3d_shared_ptr();
%OpenTriangleOrganizer_java();
