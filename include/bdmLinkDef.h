#ifdef __ROOTCLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace bdm;
#pragma link C++ class bdm::simulation::ECM+;

#pragma link C++ class bdm::physics::PhysicalNode+;
#pragma link C++ class bdm::physics::PhysicalCylinder+;
#pragma link C++ class bdm::physics::PhysicalSphere+;
#pragma link C++ class bdm::physics::Substance+;
#pragma link C++ class bdm::physics::IntracellularSubstance+;
#pragma link C++ class bdm::physics::InterObjectForce+;
#pragma link C++ class bdm::physics::PhysicalBond+;
#pragma link C++ class bdm::physics::PhysicalObject+;

#pragma link C++ class bdm::local_biology::SomaElement+;
#pragma link C++ class bdm::local_biology::NeuriteElement+;
#pragma link C++ class bdm::local_biology::CellElement+;
#pragma link C++ class bdm::local_biology::LocalBiologyModule+;

#pragma link C++ class bdm::cells::Cell+;
#pragma link C++ class bdm::cells::CellModule+;

#pragma link C++ class bdm::synapse::BiologicalBouton+;
#pragma link C++ class bdm::synapse::BiologicalSomaticSpine+;
#pragma link C++ class bdm::synapse::BiologicalSpine+;
#pragma link C++ class bdm::synapse::Excrescence+;

#pragma link C++ class bdm::SimStateSerializable+;
#pragma link C++ class bdm::Color+;

#endif
