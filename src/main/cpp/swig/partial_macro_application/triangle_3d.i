/**
 * This file contains partial macro applications for a specific type. It means, that it
 * defines a new macro and fixes the value of a number of arguments.
 * As a result it reduces the arity of the underlying macro.
 * Therefore, this macros are easier to use and result in a better readable
 * module file.
 * https://en.wikipedia.org/wiki/Partial_application
 *
 * e.g. given complex_macro(arg1, arg2, arg3, arg4, arg5);
 * set arg1=1, arg2=2, arg4=4, arg5=5:
 * -> partial macro:
 * %define %simplified(arg3)
 *    complex_macro(1, 2, arg3, 4, 5)
 * %enddef
 *
 * Documentation of macros and their arguments can be found in the included
 * files!
 */

%include "ported.i"
%include "cx3d_shared_ptr.i"

%define %Triangle3D_ported_type_modification()
  %ported_type_modification(cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>,
                            ini.cx3d.spatialOrganization.interfaces.Triangle3D<ini.cx3d.physics.PhysicalNode>);
%enddef

%define %Triangle3D_ported_add_equals()
  %ported_add_equals(cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>,
                     Triangle3DT_PhysicalNode);
%enddef

%define %Triangle3D_cx3d_shared_ptr()
  %cx3d_shared_ptr_generics(Triangle3DT_PhysicalNode,
                            ini/cx3d/spatialOrganization/interfaces/Triangle3D,
                            cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>);
%enddef
