#ifndef BDMLINKDEF_H_
#define BDMLINKDEF_H_

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
// #pragma link C++ class bdm::CellExt<bdm::SimulationObject<bdm::SoaRef>>-;

#pragma link C++ class vector < bdm::CellExt < bdm::SimulationObject < \
    bdm::Scalar >>> +;

#pragma link C++ class bdm::OneElementArray < double > +;
#pragma link C++ class bdm::OneElementArray < std::array < double, 3ul >> +;
#pragma link C++ class bdm::OneElementArray < bdm::InlineVector < int, 8ul >> +;

// todo make invariant of size parameters
#pragma link C++ class bdm::InlineVector < int, 2ul > +;
#pragma link C++ class bdm::InlineVector < int, 3ul > +;
#pragma link C++ class bdm::InlineVector < int, 8ul > +;

#endif

#endif  // BDMLINKDEF_H_
