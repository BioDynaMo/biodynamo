/**
 * This file contains code generation customizations for class OpenTriangleOrganizer.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (OPENTRIANGLEORGANIZER_NATIVE and OPENTRIANGLEORGANIZER_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/open_triangle_organizer.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %OpenTriangleOrganizer_cx3d_shared_ptr()
  %cx3d_shared_ptr(OpenTriangleOrganizerT_PhysicalNode,
                   ini/cx3d/spatialOrganization/interfaces/OpenTriangleOrganizer,
                   cx3d::spatial_organization::OpenTriangleOrganizer<cx3d::physics::PhysicalNode>);
%enddef

%define %OpenTriangleOrganizer_java()
  %java_defined_class(cx3d::spatial_organization::OpenTriangleOrganizer<cx3d::physics::PhysicalNode>,
                      OpenTriangleOrganizerT_PhysicalNode,
                      OpenTriangleOrganizer,
                      ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer,
                      ini/cx3d/spatialOrganization/interfaces/OpenTriangleOrganizer);
%enddef

%define %OpenTriangleOrganizer_native()
  %native_defined_class(cx3d::spatial_organization::OpenTriangleOrganizer<cx3d::physics::PhysicalNode>,
                            OpenTriangleOrganizerT_PhysicalNode,
                            ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer,
                            OpenTriangleOrganizer,
                            public OpenTriangleOrganizerT_PhysicalNode(){});
%enddef

%define %OpenTriangleOrganizer_typemaps()
  %typemap(javainterfaces) cx3d::spatial_organization::OpenTriangleOrganizer<cx3d::physics::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer"
%enddef

 /**
  * apply customizations
  */
 %OpenTriangleOrganizer_cx3d_shared_ptr();
 #ifdef OPENTRIANGLEORGANIZER_NATIVE
   %OpenTriangleOrganizer_native();
 #else
   %OpenTriangleOrganizer_java();
 #endif
 #ifdef OPENTRIANGLEORGANIZER_DEBUG
   %setJavaDebugSwitch(OpenTriangleOrganizer, true);
 #else
   %setJavaDebugSwitch(OpenTriangleOrganizer, false);
 #endif
 %OpenTriangleOrganizer_typemaps();
