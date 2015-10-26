/**
 * This file contains code generation customizations for class EdgeHashKey.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on the preprocessor
 * variable: EDGEHASHKEY_NATIVE
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it using:
 * %include "class_customization/edge_hash_key.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %EdgeHashKey_cx3d_shared_ptr()
  %cx3d_shared_ptr(EdgeHashKeyT_PhysicalNode,
                   ini/cx3d/spatialOrganization/interfaces/EdgeHashKey,
                   cx3d::spatial_organization::EdgeHashKey<cx3d::PhysicalNode>);
%enddef

%define %EdgeHashKey_java()
  %java_defined_class(cx3d::spatial_organization::EdgeHashKey<cx3d::PhysicalNode>,
                      EdgeHashKeyT_PhysicalNode,
                      EdgeHashKey,
                      ini.cx3d.spatialOrganization.interfaces.EdgeHashKey,
                      ini/cx3d/spatialOrganization/interfaces/EdgeHashKey);
  %typemap(javainterfaces) cx3d::spatial_organization::EdgeHashKey<cx3d::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.EdgeHashKey"
%enddef

%define %EdgeHashKey_native()
  %native_defined_class(cx3d::spatial_organization::EdgeHashKey<cx3d::PhysicalNode>,
                        EdgeHashKeyT_PhysicalNode,
                        ini.cx3d.spatialOrganization.interfaces.EdgeHashKey,
                        EdgeHashKey,
                        public EdgeHashKeyT_PhysicalNode(){});
%enddef

/**
 * apply customizations
 */
%EdgeHashKey_cx3d_shared_ptr();
#ifdef EDGEHASHKEY_NATIVE
  %EdgeHashKey_native();
#else
  %EdgeHashKey_java();
#endif
