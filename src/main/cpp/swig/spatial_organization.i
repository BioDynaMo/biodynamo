%module(directors="1") spatialOrganization

%{
#include <memory>
#include <stdint.h>

#include "spatial_organization/rational.h"
#include "spatial_organization/exact_vector.h"
#include "physical_node.h"
#include "spatial_organization/spatial_organization_node.h"
#include "spatial_organization/space_node.h"
#include "spatial_organization/tetrahedron.h"
#include "spatial_organization/plane_3d.h"
#include "spatial_organization/triangle_3d.h"
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
%include "partial_macro_application/double.i"

// modifications for class Rational
%apply long long { int64_t };
%include "partial_macro_application/rational.i";
%Rational_ported_type_modification();
%Rational_cx3d_shared_ptr();

// modifications for class ExactVector
%include "partial_macro_application/exact_vector.i";
%ExactVector_ported_type_modification();
%ExactVector_cx3d_shared_ptr();
%double_stdarray_array_marshalling(spatialOrganization, 3);
%Rational_stdarray_array_marshalling(spatialOrganization, 3);
%ExactVector_stdarray_array_marshalling(spatialOrganization, 3);

// modifications for class SpaceNode
%include "partial_macro_application/space_node.i";
%SpaceNode_cx3d_shared_ptr();
%SpaceNode_jdc_enable();
%SpaceNode_jdc_array_extension(3);
%SpaceNode_stdarray_array_marshalling(spatialOrganization, 3);
%SpaceNode_jdc_get_array(3, getNodes);
%SpaceNode_jdc_array_extension(4);
%SpaceNode_stdarray_array_marshalling(spatialOrganization, 4);
%SpaceNode_jdc_get_array(4, getAdjacentNodes);
%SpaceNode_jdc_type_modification();

// modifications for class Tetrahedron
%include "partial_macro_application/tetrahedron.i";
%Tetrahedron_cx3d_shared_ptr();
%Tetrahedron_jdc_enable();
%Tetrahedron_jdc_get(getOppositeTetrahedron);
%Tetrahedron_jdc_remove_method_bodies();
%Tetrahedron_jdc_type_modification();

// modifications for Plane3D
%include "partial_macro_application/plane_3d.i";
%Plane3D_ported_type_modification();
%Plane3D_cx3d_shared_ptr();

// modifications for class Triangle3D
%include "partial_macro_application/triangle_3d.i";
%Triangle3D_ported_type_modification();
%Triangle3D_ported_add_equals();
%Triangle3D_cx3d_shared_ptr();
%double_stdarray_2dim_array_marshalling(spatialOrganization, 3, 3);

// add the original header files here
%include "spatial_organization/rational.h"
%include "spatial_organization/exact_vector.h"
%include "physical_node.h"
%include "spatial_organization/spatial_organization_node.h"
%include "spatial_organization/space_node.h"
%include "spatial_organization/tetrahedron.h"
%include "spatial_organization/plane_3d.h"
%include "spatial_organization/triangle_3d.h"

// generate templates
namespace cx3d{
namespace spatial_organization{
  %template(SpaceNodeT_PhysicalNode) SpaceNode<cx3d::PhysicalNode>;
  %template(TetrahedronT_PhysicalNode) Tetrahedron<cx3d::PhysicalNode>;
  %template(Plane3DT_PhysicalNode) Plane3D<cx3d::PhysicalNode>;
  %template(Triangle3DT_PhysicalNode) Triangle3D<cx3d::PhysicalNode>;
}
}
