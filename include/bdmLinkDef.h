#ifdef __ROOTCLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace bdm;

#pragma extra_include "physics/physical_node.h";

#pragma link C++ class bdm::simulation::ECM+;
#pragma link C++ class bdm::simulation::Scheduler+;

#pragma link C++ class bdm::physics::PhysicalNode+;
#pragma link C++ class bdm::physics::PhysicalCylinder+;
#pragma link C++ class bdm::physics::PhysicalSphere+;
#pragma link C++ class bdm::physics::Substance+;
#pragma link C++ class bdm::physics::IntracellularSubstance+;
#pragma link C++ class bdm::physics::InterObjectForce+;
#pragma link C++ class bdm::physics::PhysicalBond+;
#pragma link C++ class bdm::physics::PhysicalObject+;
#pragma link C++ class bdm::physics::DefaultForce+;

#pragma link C++ class bdm::local_biology::SomaElement+;
#pragma link C++ class bdm::local_biology::NeuriteElement+;
#pragma link C++ class bdm::local_biology::CellElement+;
#pragma link C++ class bdm::local_biology::LocalBiologyModule+;
#pragma link C++ class bdm::local_biology::AbstractLocalBiologyModule+;

#pragma link C++ class bdm::spatial_organization::SpatialOrganizationNode<bdm::physics::PhysicalNode>+;
#pragma link C++ class bdm::spatial_organization::SpaceNode<bdm::physics::PhysicalNode>+;
#pragma link C++ class bdm::spatial_organization::SpatialOrganizationNode+;
#pragma link C++ class bdm::spatial_organization::SpaceNode+;

#pragma link C++ class bdm::cells::Cell+;
#pragma link C++ class bdm::cells::CellModule+;
#pragma link C++ class bdm::cells::AbstractCellModule+;
#pragma link C++ class bdm::cells::SimpleCellCycle+;
#pragma link C++ class bdm::cells::CellFactory+;

#pragma link C++ class bdm::synapse::BiologicalBouton+;
#pragma link C++ class bdm::synapse::PhysicalBouton+;
#pragma link C++ class bdm::synapse::BiologicalSomaticSpine+;
#pragma link C++ class bdm::synapse::PhysicalSomaticSpine+;
#pragma link C++ class bdm::synapse::BiologicalSpine+;
#pragma link C++ class bdm::synapse::PhysicalSpine+;
#pragma link C++ class bdm::synapse::Excrescence+;

#pragma link C++ class bdm::SimStateSerializable+;
#pragma link C++ class bdm::Color+;

#pragma link C++ class bdm::SomaRandomWalkModule+;
#pragma link C++ class bdm::InternalSecretor+;
#pragma link C++ class bdm::GrowthCone+;
#pragma link C++ class bdm::DividingModule+;
#pragma link C++ class bdm::MembraneContact+;
#pragma link C++ class bdm::NeuriteChemoAttraction+;
#pragma link C++ class bdm::RandomBranchingModule+;
#pragma link C++ class bdm::XAdhesiveForce+;
#pragma link C++ class bdm::XBifurcationModule+;
#pragma link C++ class bdm::XMovementModule+;

#endif
