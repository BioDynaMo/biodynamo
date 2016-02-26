/**
 * This file contains code generation customizations for class TriangleHashKey
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on the preprocessor
 * variable: TRIANGLEHASHKEY_NATIVE
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it using:
 * %include "class_customization/triangle_hash_key.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %TriangleHashKey_cx3d_shared_ptr()
  %cx3d_shared_ptr(TriangleHashKeyT_PhysicalNode,
                   ini/cx3d/spatialOrganization/interfaces/TriangleHashKey,
                   cx3d::spatial_organization::TriangleHashKey<cx3d::physics::PhysicalNode>);
%enddef

%define %TriangleHashKey_java()
  %java_defined_class(cx3d::spatial_organization::TriangleHashKey<cx3d::physics::PhysicalNode>,
                      TriangleHashKeyT_PhysicalNode,
                      TriangleHashKey,
                      ini.cx3d.spatialOrganization.interfaces.TriangleHashKey,
                      ini/cx3d/spatialOrganization/interfaces/TriangleHashKey);
  %typemap(javainterfaces) cx3d::spatial_organization::TriangleHashKey<cx3d::physics::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.TriangleHashKey"
%enddef

%define %TriangleHashKey_native()
  %native_defined_class(cx3d::spatial_organization::TriangleHashKey<cx3d::physics::PhysicalNode>,
                        TriangleHashKeyT_PhysicalNode,
                        ini.cx3d.spatialOrganization.interfaces.TriangleHashKey,
                        TriangleHashKey,
                        public TriangleHashKeyT_PhysicalNode(){});
%enddef

/**
 * apply customizations
 */
%TriangleHashKey_cx3d_shared_ptr();
#ifdef TRIANGLEHASHKEY_NATIVE
  %TriangleHashKey_native();
#else
  %TriangleHashKey_java();
#endif
