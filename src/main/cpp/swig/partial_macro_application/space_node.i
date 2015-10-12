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
%include "java_defined_class.i"
%include "cx3d_shared_ptr.i"

%define %SpaceNode_cx3d_shared_ptr()
  %cx3d_shared_ptr_generics(SpaceNodeT_PhysicalNodeCppType,
                            ini/cx3d/spatialOrganization/SpaceNode,
                            cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>);
%enddef

%define %SpaceNode_jdc_enable()
  %jdc_enable_templated(cx3d::spatial_organization, SpaceNode,
                        <cx3d::PhysicalNode>, SpaceNodeT_PhysicalNode);
%enddef

%define %SpaceNode_jdc_get(METHOD_NAME)
  %jdc_get(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>, METHOD_NAME,
           ini.cx3d.spatialOrganization.SpaceNode<ini.cx3d.physics.PhysicalNode>,
           cx3d_spatial_organization_SpaceNode_Sl_cx3d_PhysicalNode_Sg__);
%enddef

%define %SpaceNode_jdc_array_extension(SIZE)
  %jdc_array_extension_templated(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>, getJava_impl,
                                 ini.cx3d.spatialOrganization.SpaceNode, ini.cx3d.physics.PhysicalNode,
                                 shared_ptr_SpaceNodeT_PhysicalNode_##SIZE, cx3d_spatial_organization_SpaceNode_Sl_cx3d_PhysicalNode_Sg__);
%enddef

%define %SpaceNode_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode> >,
                              shared_ptr_SpaceNodeT_PhysicalNode_##SIZE,
                              ini.cx3d.swig.spatialOrganization.SpaceNodeT_PhysicalNodeCppType,
                              Lini/cx3d/spatialOrganization/SpaceNode;, SIZE);
%enddef

%define %SpaceNode_jdc_get_array(SIZE, METHOD_NAME)
  %jdc_get_array(std::shared_ptr<cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>>, SIZE, METHOD_NAME,
                 ini.cx3d.spatialOrganization.SpaceNode,
                 shared_ptr_SpaceNodeT_PhysicalNode_##SIZE);
%enddef

%define %SpaceNode_jdc_type_modification()
  %jdc_type_modification(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>,
                         ini.cx3d.spatialOrganization.SpaceNode);
%enddef
