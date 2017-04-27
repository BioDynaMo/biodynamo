#ifndef BDMLINKDEF_H_
#define BDMLINKDEF_H_

#ifdef __ROOTCLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace bdm;

#pragma link C++ class bdm::Cell<bdm::VcBackend>+;
#pragma link C++ class bdm::Cell<bdm::ScalarBackend>+;

#pragma link C++ class bdm::Object<bdm::VcBackend>+;
#pragma link C++ class bdm::Object<bdm::ScalarBackend>+;

#pragma link C++ class bdm::daosoa<bdm::Cell, bdm::VcBackend>+;
#pragma link C++ class bdm::daosoa<bdm::Cell, bdm::ScalarBackend>+;
#pragma link C++ class bdm::daosoa<bdm::Object, bdm::VcBackend>+;
#pragma link C++ class bdm::daosoa<bdm::Object, bdm::ScalarBackend>+;

// todo make invariant of size parameters
#pragma link C++ class bdm::InlineVector<int, 2ul>+;
#pragma link C++ class bdm::InlineVector<int, 3ul>+;
#pragma link C++ class bdm::InlineVector<int, 8ul>+;

#pragma link C++ class Vc::AlignedBase<16ul>+;
#pragma link C++ class Vc::AlignedBase<32ul>+;

#endif

#endif  // BDMLINKDEF_H_
