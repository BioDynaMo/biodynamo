%module(directors="1") spatialOrganization

%include "util.i"
// administration area

%native(RATIONAL);
%native(EXACTVECTOR);
%native(TRIANGLE3D);
%native(EDGE);
%native(TETRAHEDRON);

// end administration area

%{
#include <memory>
#include <stdint.h>

#include "spatial_organization/rational.h"
#include "spatial_organization/exact_vector.h"
#include "physical_node.h"
#include "spatial_organization/spatial_organization_node.h"
#include "spatial_organization/space_node.h"
#include "spatial_organization/open_triangle_organizer.h"
#include "spatial_organization/tetrahedron.h"
#include "spatial_organization/plane_3d.h"
#include "spatial_organization/triangle_3d.h"
#include "spatial_organization/spatial_organization_edge.h"
#include "spatial_organization/edge.h"
#include "spatial_organization/edge_hash_key.h"
#include "spatial_organization/triangle_hash_key.h"
using namespace cx3d::spatial_organization;
#include "spatial_organizationJAVA_wrap.h"
%}

// import depending modules
%import "cx3d.i"

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_spatialOrganization);

// typemap definitions, code modifications / additions
%include "big_integer_typemap.i"
%include "primitives.i"

// modifications for class Rational
%apply long long { int64_t };
%include "class_customization/rational.i";

// modifications for class ExactVector
%include "class_customization/exact_vector.i";

// modifications for class Edge
%include "class_customization/edge.i";

// modifications for class SpaceNode
%include "class_customization/space_node.i";

// modifications for class TriangleHashKey
%include "class_customization/open_triangle_organizer.i";

// modifications for class Tetrahedron
%include "class_customization/tetrahedron.i";

// modifications for Plane3D
%include "class_customization/plane_3d.i";

// modifications for class Triangle3D
%include "class_customization/triangle_3d.i";

// modifications for class PhysicalNode
%include "class_customization/physical_node.i"

// modifications for class EdgeHashKey
%include "class_customization/edge_hash_key.i";
%EdgeHashKey_cx3d_shared_ptr();
%EdgeHashKey_ported_add_equals();
%EdgeHashKey_ported_type_modification();

// modifications for class TriangleHashKey
%include "class_customization/triangle_hash_key.i";
%TriangleHashKey_cx3d_shared_ptr();
%TriangleHashKey_ported_add_equals();
%TriangleHashKey_ported_type_modification();


// add the original header files here
%include "spatial_organization/rational.h"
%include "spatial_organization/exact_vector.h"
%include "physical_node.h"
%include "spatial_organization/spatial_organization_node.h"
%include "spatial_organization/open_triangle_organizer.h"
%include "spatial_organization/tetrahedron.h"
%include "spatial_organization/plane_3d.h"
%include "spatial_organization/triangle_3d.h"
%include "spatial_organization/spatial_organization_edge.h"
%include "spatial_organization/edge.h"
%include "spatial_organization/edge_hash_key.h"
%include "spatial_organization/triangle_hash_key.h"
%include "spatial_organization/space_node.h"


// generate templates
namespace cx3d{
namespace spatial_organization{
  %template(OpenTriangleOrganizerT_PhysicalNode) OpenTriangleOrganizer<cx3d::PhysicalNode>;
  %template(TetrahedronT_PhysicalNode) Tetrahedron<cx3d::PhysicalNode>;
  %template(Plane3DT_PhysicalNode) Plane3D<cx3d::PhysicalNode>;
  %template(Triangle3DT_PhysicalNode) Triangle3D<cx3d::PhysicalNode>;
  %template(EdgeT_PhysicalNode) Edge<cx3d::PhysicalNode>;
  %template(EdgeHashKeyT_PhysicalNode) EdgeHashKey<cx3d::PhysicalNode>;
  %template(TriangleHashKeyT_PhysicalNode) TriangleHashKey<cx3d::PhysicalNode>;
  %template(SpaceNodeT_PhysicalNode) SpaceNode<cx3d::PhysicalNode>;
}
}
