%module(directors="1") simulation

%include "util.i"
%include "config.i"

%{
#include <memory>
#include <stdint.h>

#include "param.h"
#include "color.h"
#include "java_util.h"

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
#include "spatial_organization/spatial_organization_node_movement_listener.h"
#include "spatial_organization/spatial_organization_edge.h"
#include "spatial_organization/simple_triangulation_node_organizer.h"
#include "spatial_organization/open_triangle_organizer.h"

#include "physics/physical_node.h"
#include "physics/substance.h"
#include "physics/intracellular_substance.h"
#include "physics/physical_node.h"
#include "physics/physical_node_movement_listener.h"
#include "physics/physical_object.h"
#include "physics/physical_cylinder.h"
#include "physics/physical_bond.h"
#include "physics/collision_check.h"
#include "physics/physical_sphere.h"
#include "physics/default_force.h"

#include "local_biology/cell_element.h"
#include "local_biology/soma_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/abstract_local_biology_module.h"

#include "synapse/excrescence.h"
#include "synapse/physical_spine.h"
#include "synapse/physical_bouton.h"
#include "synapse/physical_somatic_spine.h"
#include "synapse/biological_spine.h"
#include "synapse/biological_bouton.h"
#include "synapse/biological_somatic_spine.h"
#include "synapse/connection_maker.h"
#include "synapse/test_synapses.h"
#include "synapse/excrescence.h"

#include "cells/cell.h"
#include "cells/cell_module.h"
#include "cells/cell_factory.h"

#include "simulation/scheduler.h"
#include "simulation/ecm.h"

#include "test/dividing_cell_test.h"
#include "test/dividing_module_test.h"
#include "test/intracellular_diffusion_test.h"
#include "test/membrane_contact_test.h"
#include "test/soma_random_walk_module_test.h"
#include "test/neurite_chemo_attraction_test.h"
#include "test/random_branching_module_test.h"
#include "test/simple_synapse_test.h"
#include "test/small_network_test.h"
#include "test/soma_clustering_test.h"
#include "test/figure_5_test.h"
#include "test/x_adhesive_force.h"
#include "test/x_bifurcation_module.h"
#include "test/x_movement_module.h"
#include "test/figure_9_test.h"

using namespace cx3d;
using namespace cx3d::spatial_organization;
using namespace cx3d::physics;
using namespace cx3d::local_biology;
using namespace cx3d::cells;
using namespace cx3d::simulation;
using cx3d::Color;
#include "simulationJAVA_wrap.h"
%}

// import depending modules
%import "cx3d.i"
// %import "physics.i"

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d_simulation);

// typemap definitions, code modifications / additions
%include "big_integer_typemap.i"
%include "primitives.i"
%double_stdarray_array_marshalling(simulation, 2);
%double_stdarray_array_marshalling(simulation, 3);
%double_stdarray_array_marshalling(simulation, 4);
%include "color_typemap.i"
%color(simulation);
%pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder;"

// -----------------------------------------------------------------------------
// class modifications
%include "class_customization/java_util.i"
%include "class_customization/java_util_2.i"

%include "class_customization/rational.i";
%include "class_customization/exact_vector.i";
%include "class_customization/plane_3d.i";
%include "class_customization/triangle_3d.i";
%include "class_customization/edge.i";
%include "class_customization/edge_hash_key.i";
%include "class_customization/triangle_hash_key.i";
%include "class_customization/tetrahedron.i";
%include "class_customization/open_triangle_organizer.i";
%include "class_customization/spatial_organization_node_movement_listener.i"
%include "class_customization/space_node.i"
%include "class_customization/simple_triangulation_node_organizer.i"

%include "class_customization/physics/physical_node.i"
%include "class_customization/physics/substance.i"
%include "class_customization/physics/intracellular_substance.i"
%include "class_customization/physics/physical_node.i"
%include "class_customization/physics/physical_node_movement_listener.i"
%include "class_customization/physics/physical_object.i"
%include "class_customization/physics/physical_cylinder.i"
%include "class_customization/physics/physical_bond.i"
%include "class_customization/physics/physical_sphere.i"
%include "class_customization/physics/inter_object_force.i"
%include "class_customization/physics/default_force.i"

%include "class_customization/local_biology/cell_element.i"
%include "class_customization/local_biology/soma_element.i"
%include "class_customization/local_biology/neurite_element.i"
%include "class_customization/local_biology/local_biology_module.i"
%include "class_customization/local_biology/abstract_local_biology_module.i"

%include "class_customization/synapse/excrescence.i"
%include "class_customization/synapse/physical_spine.i"
%include "class_customization/synapse/physical_bouton.i"
%include "class_customization/synapse/physical_somatic_spine.i"
%include "class_customization/synapse/biological_spine.i"
%include "class_customization/synapse/biological_bouton.i"
%include "class_customization/synapse/biological_somatic_spine.i"
%include "class_customization/synapse/connection_maker.i"
%include "class_customization/synapse/test_synapses.i"

%include "class_customization/cells/cell.i"
%include "class_customization/cells/cell_module.i"
%include "class_customization/cells/cell_factory.i"

%include "class_customization/simulation/ecm.i"
%include "class_customization/simulation/scheduler.i"

%include "class_customization/tests.i"

%ignore cx3d::Param::kYellow;
%ignore cx3d::Param::kYellowSolid;
%ignore cx3d::Param::kViolet;
%ignore cx3d::Param::kVioletSolid;
%ignore cx3d::Param::kRed;
%ignore cx3d::Param::kRedSolid;
%ignore cx3d::Param::kGreen;
%ignore cx3d::Param::kBlue;
%ignore cx3d::Param::kGray;
%ignore cx3d::Param::kGraySolid;


// -----------------------------------------------------------------------------
// add the original header files here
%include "color.h"
%include "param.h"
%include "java_util.h"

%include "spatial_organization/rational.h"
%include "spatial_organization/exact_vector.h"
%include "spatial_organization/plane_3d.h"
%include "spatial_organization/triangle_3d.h"
%include "spatial_organization/spatial_organization_edge.h"
%include "spatial_organization/edge.h"
%include "spatial_organization/edge_hash_key.h"
%include "spatial_organization/triangle_hash_key.h"
%include "spatial_organization/tetrahedron.h"
%include "spatial_organization/space_node.h"
%include "spatial_organization/open_triangle_organizer.h"
%include "spatial_organization/spatial_organization_node_movement_listener.h"
%include "spatial_organization/simple_triangulation_node_organizer.h"

%include "physics/substance.h"
%include "physics/intracellular_substance.h"
%include "physics/physical_node.h"
%include "physics/physical_node_movement_listener.h"
%include "physics/physical_object.h"
%include "physics/physical_cylinder.h"
%include "physics/physical_bond.h"
%include "physics/collision_check.h"
%include "physics/physical_sphere.h"
%include "physics/inter_object_force.h"
%include "physics/default_force.h"

%include "local_biology/local_biology_module.h"
%include "local_biology/abstract_local_biology_module.h"
%include "local_biology/cell_element.h"
%include "local_biology/soma_element.h"
%include "local_biology/neurite_element.h"

%include "synapse/excrescence.h"
%include "synapse/physical_spine.h"
%include "synapse/physical_bouton.h"
%include "synapse/physical_somatic_spine.h"
%include "synapse/biological_spine.h"
%include "synapse/biological_bouton.h"
%include "synapse/biological_somatic_spine.h"
%include "synapse/connection_maker.h"
%include "synapse/test_synapses.h"

%include "cells/cell.h"
%include "cells/cell_module.h"
%include "cells/cell_factory.h"

%include "simulation/scheduler.h"
%include "simulation/ecm.h"

%include "test/dividing_cell_test.h"
%include "test/dividing_module_test.h"
%include "test/intracellular_diffusion_test.h"
%include "test/membrane_contact_test.h"
%include "test/soma_random_walk_module_test.h"
%include "test/neurite_chemo_attraction_test.h"
%include "test/random_branching_module_test.h"
%include "test/simple_synapse_test.h"
%include "test/small_network_test.h"
%include "test/soma_clustering_test.h"
%include "test/figure_5_test.h"
%include "test/x_adhesive_force.h"
%include "test/x_bifurcation_module.h"
%include "test/x_movement_module.h"
%include "test/figure_9_test.h"

// -----------------------------------------------------------------------------
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
