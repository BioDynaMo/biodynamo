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

%include "std_array_typemap.i"
%include "std_list_typemap.i"
%include "java_defined_class.i"
%include "cx3d_shared_ptr.i"

%define %Tetrahedron_cx3d_shared_ptr()
  %cx3d_shared_ptr(TetrahedronT_PhysicalNodeCppType,
                   ini/cx3d/swig/spatialOrganization/TetrahedronT_PhysicalNodeCppType,
                   cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>);
%enddef

%define %Tetrahedron_jdc_enable()
  %jdc_enable_templated(cx3d::spatial_organization, Tetrahedron,
                        <cx3d::PhysicalNode>, TetrahedronT_PhysicalNode);
%enddef

%define %Tetrahedron_jdc_get(METHOD_NAME)
  %jdc_get(cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>, METHOD_NAME,
           ini.cx3d.spatialOrganization.Tetrahedron,
           cx3d_spatial_organization_Tetrahedron_Sl_cx3d_PhysicalNode_Sg__);
%enddef

%define %Tetrahedron_jdc_remove_method_bodies()
  %jdc_remove_method_body(std::shared_ptr<cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>> cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>::getOppositeNode);
%enddef

%define %Tetrahedron_jdc_type_modification()
  %jdc_type_modification(cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>,
                         ini.cx3d.spatialOrganization.Tetrahedron);
%enddef

%define %Tetrahedron_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>>,
                   Tetrahedron, ini.cx3d.spatialOrganization.Tetrahedron);
%enddef
