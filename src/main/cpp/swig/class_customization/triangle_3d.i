/**
 * This file contains code generation customizations for class Tr.iangle3D.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (TRIANGLE3D_NATIVE and TRIANGLE3D_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it using:
 * %include "class_customization/tetrahedron.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

// %define %Triangle3D_ported_type_modification()
//   %ported_type_modification(cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>,
//                             ini.cx3d.spatialOrganization.interfaces.Triangle3D);
// %enddef
//
// %define %Triangle3D_ported_add_equals()
//   %ported_add_equals(cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>,
//                      Triangle3DT_PhysicalNode);
// %enddef

%define %Triangle3D_cx3d_shared_ptr()
  %cx3d_shared_ptr(Triangle3DT_PhysicalNode,
                   ini/cx3d/spatialOrganization/interfaces/Triangle3D,
                   cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>);
%enddef

%define %Triangle3D_java()
  %java_defined_class(cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>,
                      Triangle3DT_PhysicalNode,
                      Triangle3D,
                      ini.cx3d.spatialOrganization.interfaces.Triangle3D,
                      ini/cx3d/spatialOrganization/interfaces/Triangle3D);
  %typemap(javainterfaces) cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.Triangle3D"
%enddef

%define %Triangle3D_native()
  %native_defined_class(cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>,
                        Triangle3DT_PhysicalNode,
                        ini.cx3d.spatialOrganization.interfaces.Triangle3D,
                        Triangle3D,
                        public Triangle3DT_PhysicalNode(){});
%enddef

%define %Triangle3D_typemaps()
  %double_stdarray_2dim_array_marshalling(spatialOrganization, 3, 3);
  // for class Tetrahedron
  %Triangle3D_stdarray_array_marshalling(spatialOrganization, 2);
  %Triangle3D_stdarray_array_marshalling(spatialOrganization, 3);
  %Triangle3D_stdarray_array_marshalling(spatialOrganization, 4);
%enddef

%define %Triangle3D_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode> >,
                              shared_ptr_Triangle3DT_PhysicalNode_##SIZE,
                              ini.cx3d.spatialOrganization.interfaces.Triangle3D,
                              Lini/cx3d/spatialOrganization/interfaces/Triangle3D;, SIZE);
%enddef


/**
 * apply customizations
 */
%Triangle3D_cx3d_shared_ptr();
#ifdef TRIANGLE3D_NATIVE
  %Triangle3D_native();
#else
  %Triangle3D_java();
#endif
#ifdef TRIANGLE3D_DEBUG
  %setJavaDebugSwitch(Triangle3D, true);
#else
  %setJavaDebugSwitch(Triangle3D, false);
#endif
%Triangle3D_typemaps();
