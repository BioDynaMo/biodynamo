%module(directors="1") spatialOrganization

%include "util.i"
%include "config.i"

%{
#include <memory>
#include <stdint.h>

#include "spatial_organization/rational.h"
#include "spatial_organization/exact_vector.h"
#include "spatial_organization/plane_3d.h"
#include "spatial_organization/triangle_3d.h"
#include "spatial_organization/spatial_organization_edge.h"
#include "spatial_organization/edge.h"
#include "spatial_organization/edge_hash_key.h"
#include "spatial_organization/triangle_hash_key.h"
#include "spatial_organization/tetrahedron.h"
#include "spatial_organization/spatial_organization_node.h"
#include "spatial_organization/space_node.h"
#include "physics/physical_node.h"
#include "spatial_organization/spatial_organization_node_movement_listener.h"
#include "spatial_organization/spatial_organization_edge.h"
#include "java_util.h"
#include "spatial_organization/simple_triangulation_node_organizer.h"
#include "spatial_organization/open_triangle_organizer.h"
using namespace cx3d::spatial_organization;
using cx3d::Color;
#include "spatial_organizationJAVA_wrap.h"
%}

// import depending modules
%import "cx3d.i"
// %import "physics.i"

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_spatialOrganization);

// typemap definitions, code modifications / additions
%include "big_integer_typemap.i"
%include "primitives.i"
%pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder;"

// class modifications
%include "class_customization/rational.i";
%include "class_customization/exact_vector.i";
%include "class_customization/plane_3d.i";
%include "class_customization/triangle_3d.i";
%include "class_customization/edge.i";
%include "class_customization/edge_hash_key.i";
%include "class_customization/triangle_hash_key.i";
%include "class_customization/tetrahedron.i";
%include "class_customization/open_triangle_organizer.i";
%include "class_customization/physics/physical_node.i"
%include "class_customization/spatial_organization_node_movement_listener.i"
%include "class_customization/space_node.i"
%include "class_customization/java_util.i"
%include "class_customization/simple_triangulation_node_organizer.i"

// add the original header files here
%include "spatial_organization/rational.h"
%include "spatial_organization/exact_vector.h"
%include "spatial_organization/plane_3d.h"
%include "spatial_organization/triangle_3d.h"
%include "spatial_organization/spatial_organization_edge.h"
%include "spatial_organization/edge.h"
%include "spatial_organization/edge_hash_key.h"
%include "spatial_organization/triangle_hash_key.h"
%include "spatial_organization/tetrahedron.h"
%include "java_util.h"
%include "spatial_organization/space_node.h"
%include "spatial_organization/open_triangle_organizer.h"
%include "spatial_organization/spatial_organization_node_movement_listener.h"
%include "spatial_organization/simple_triangulation_node_organizer.h"

// generate templates
namespace cx3d{
namespace spatial_organization{
  %template(Plane3DT_PhysicalNode) Plane3D<cx3d::physics::PhysicalNode>;
  %template(Triangle3DT_PhysicalNode) Triangle3D<cx3d::physics::PhysicalNode>;
  %template(EdgeT_PhysicalNode) Edge<cx3d::physics::PhysicalNode>;
  %template(EdgeHashKeyT_PhysicalNode) EdgeHashKey<cx3d::physics::PhysicalNode>;
  %template(TriangleHashKeyT_PhysicalNode) TriangleHashKey<cx3d::physics::PhysicalNode>;
  %template(TetrahedronT_PhysicalNode) Tetrahedron<cx3d::physics::PhysicalNode>;
  %template(SpaceNodeT_PhysicalNode) SpaceNode<cx3d::physics::PhysicalNode>;
  %template(OpenTriangleOrganizerT_PhysicalNode) OpenTriangleOrganizer<cx3d::physics::PhysicalNode>;
  %template(SpatialOrganizationNodeMovementListenerT_PhysicalNode) SpatialOrganizationNodeMovementListener<cx3d::physics::PhysicalNode>;
  %template(SimpleTriangulationNodeOrganizerT_PhysicalNode) SimpleTriangulationNodeOrganizer<cx3d::physics::PhysicalNode>;
}  // namespace spatial_organization
  %template(JavaUtilT_PhysicalNode) JavaUtil<cx3d::physics::PhysicalNode>;
}
