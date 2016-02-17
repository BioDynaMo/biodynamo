/**
 * This file contains code generation customizations for class SpatialOrganizationNodeMovementListener.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/spatial_organization_node_movement_listener.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"
%include "std_array_typemap.i"

%define %SpatialOrganizationNodeMovementListener_cx3d_shared_ptr()
  %cx3d_shared_ptr(SpatialOrganizationNodeMovementListenerT_PhysicalNode,
                   ini/cx3d/spatialOrganization/SpatialOrganizationNodeMovementListener,
                   cx3d::spatial_organization::SpatialOrganizationNodeMovementListener<cx3d::PhysicalNode>);
%enddef

%define %SpatialOrganizationNodeMovementListener_java()
  %java_defined_class(cx3d::spatial_organization::SpatialOrganizationNodeMovementListener<cx3d::PhysicalNode>,
                      SpatialOrganizationNodeMovementListenerT_PhysicalNode,
                      SpatialOrganizationNodeMovementListener,
                      ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener,
                      ini/cx3d/spatialOrganization/SpatialOrganizationNodeMovementListener);
  %typemap(javainterfaces) cx3d::spatial_organization::SpatialOrganizationNodeMovementListener<cx3d::PhysicalNode> "ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener"
%enddef

%define %SpatialOrganizationNodeMovementListener_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::spatial_organization::SpatialOrganizationNodeMovementListener<cx3d::PhysicalNode>>,
                   SpatialOrganizationNodeMovementListener,
                   ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener);
%enddef

/**
 * apply customizations
 */
%SpatialOrganizationNodeMovementListener_cx3d_shared_ptr();
%SpatialOrganizationNodeMovementListener_java();
%SpatialOrganizationNodeMovementListener_stdlist();
