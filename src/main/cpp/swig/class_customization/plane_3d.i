/**
 * This file contains code generation customizations for class Plane3D.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it using:
 * %include "class_customization/plane3d.i"
 */

%include "cx3d_shared_ptr.i"

%define %Plane3D_cx3d_shared_ptr()
  %cx3d_shared_ptr(Plane3DT_PhysicalNode,
                   ini/cx3d/spatialOrganization/interfaces/Plane3D,
                   cx3d::spatial_organization::Plane3D<cx3d::PhysicalNode>);
%enddef

/**
 * apply customizations
 */
#ifdef TRIANGLE3D_NATIVE
  %Plane3D_cx3d_shared_ptr();
  %typemap(javainterfaces) cx3d::spatial_organization::Plane3D<cx3d::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.Plane3D"
  %typemap(jstype) cx3d::spatial_organization::Plane3D<cx3d::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.Plane3D"
#endif
