#ifndef BDMLINKDEF_H_
#define BDMLINKDEF_H_

/// The LinkDef header file that contains all the classes which should be
/// enlisted in the ROOT dictionary; classes that want to be used for I/O.
// clang-format off
#ifdef __ROOTCLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace bdm;

#pragma link C++ class bdm::RuntimeVariables+;

#pragma link C++ class bdm::SimulationObject < bdm::Soa > +;
#pragma link C++ class bdm::SimulationObject < bdm::Scalar > +;

#pragma link C++ class bdm::CellExt < bdm::SimulationObject < bdm::Scalar >> +;
#pragma link C++ class bdm::CellExt < bdm::SimulationObject < bdm::Soa >> +;
// #pragma link C++ class bdm::CellExt<bdm::SimulationObject<bdm::SoaRef>>+;

#pragma link C++ class vector < bdm::CellExt < bdm::SimulationObject < bdm::Scalar >>> +;

// todo make invariant of size parameters
#pragma link C++ class bdm::InlineVector < int, 2ul > +;
#pragma link C++ class bdm::InlineVector < int, 3ul > +;
#pragma link C++ class bdm::InlineVector < int, 8ul > +;

#pragma link C++ class bdm::simulation_object_util_test_internal::Neurite+;

#pragma link C++ class bdm::OneElementArray < double > +;
#pragma link C++ class bdm::OneElementArray < std::array < double, 3ul >> +;
#pragma link C++ class bdm::OneElementArray < bdm::InlineVector < int, 8ul >> +;
#pragma link C++ class bdm::OneElementArray < std::vector < bdm::simulation_object_util_test_internal::Neurite, std::allocator < bdm::simulation_object_util_test_internal::Neurite >>> +;

#pragma link C++ class bdm::ScalarSimulationObject +;
#pragma link C++ class bdm::SoaSimulationObject <bdm::Soa > +;
#pragma link C++ class bdm::VectorPlaceholder < bdm::CellExt < bdm::SimulationObject < bdm::Scalar >>> +;

#pragma link C++ class bdm::TransactionalVector < int > +;
#pragma link C++ class bdm::TransactionalVector < bdm::CellExt < bdm::SimulationObject<bdm::Scalar >>> +;
#pragma link C++ class bdm::TransactionalVector < bdm::simulation_object_util_test_internal::NeuronExt < bdm::simulation_object_util_test_internal::CellExt < bdm::SimulationObject < bdm::Scalar >>>> +;

// clang-format on

#endif

#endif  // BDMLINKDEF_H_
