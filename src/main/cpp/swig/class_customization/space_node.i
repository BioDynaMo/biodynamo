/**
 * This file contains code generation customizations for class SpaceNode.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/space_node.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_array_typemap.i"

%define %SpaceNode_cx3d_shared_ptr()
  %cx3d_shared_ptr(SpaceNodeT_PhysicalNode,
                   ini/cx3d/spatialOrganization/SpaceNode,
                   cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>);
%enddef

%define %SpaceNode_java()
  %java_defined_class(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>,
                      SpaceNodeT_PhysicalNode,
                      SpaceNode,
                      ini.cx3d.spatialOrganization.SpaceNode,
                      ini/cx3d/spatialOrganization/SpaceNode);
%enddef

%define %SpaceNode_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode> >,
                              shared_ptr_SpaceNodeT_PhysicalNode_##SIZE,
                              ini.cx3d.spatialOrganization.SpaceNode,
                              Lini/cx3d/spatialOrganization/SpaceNode;, SIZE);
%enddef

/**
 * apply customizations
 */
%SpaceNode_cx3d_shared_ptr();
%SpaceNode_java();
%SpaceNode_stdarray_array_marshalling(spatialOrganization, 3);
%SpaceNode_stdarray_array_marshalling(spatialOrganization, 4);
%rename(getUserObjectSwig) cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>::getUserObject;
