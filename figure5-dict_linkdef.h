// some C++ header definition
#ifdef __ROOTCLING__
// turns off dictionary generation for all
#pragma link off all class;
#pragma link off all function;
#pragma link off all global;
#pragma link off all typedef;

#pragma link C++ class bdm::OneElementArray<array<double,3> >+;
#pragma link C++ class bdm::OneElementArray<double>+;
#pragma link C++ class bdm::OneElementArray<vector<bdm::Variant<bdm::NullBiologyModule> > >+;
#pragma link C++ class bdm::OneElementArray<unsigned long>+;
#pragma link C++ class bdm::OneElementArray<bool>+;
#pragma link C++ class bdm::OneElementArray<bdm::neuroscience::NeuronNeuriteAdapter<bdm::SoPointer<bdm::neuroscience::NeuronExt<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt>,bdm::Cell_TCTParam_TDerived>,bdm::Soa>,bdm::SoPointer<bdm::neuroscience::NeuriteExt<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt>,bdm::SimulationObject_TCTParam_TDerived>,bdm::Soa> > >+;
#pragma link C++ class bdm::OneElementArray<bdm::SoPointer<bdm::neuroscience::NeuriteExt<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt>,bdm::SimulationObject_TCTParam_TDerived>,bdm::Soa> >+;
#pragma link C++ class bdm::OneElementArray<int>+;
#pragma link C++ class bdm::OneElementArray<vector<bdm::SoPointer<bdm::neuroscience::NeuriteExt<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt>,bdm::SimulationObject_TCTParam_TDerived>,bdm::Soa> > >+;
#pragma link C++ class bdm::OneElementArray<std::unordered_map<unsigned int,array<double,3> > >+;
#pragma link C++ class bdm::OneElementArray<bdm::SoPointer<bdm::neuroscience::NeuronExt<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt>,bdm::Cell_TCTParam_TDerived>,bdm::Soa> >+;
#pragma link C++ class bdm::InlineVector<bdm::SoHandle,4>+;
#pragma link C++ class bdm::InlineVector<array<double,3>,4>+;
#pragma link C++ class bdm::DiffusionGrid+;
#pragma link C++ class bdm::ResourceManager<bdm::CompileTimeParam<bdm::Soa> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt> >+;
#pragma link C++ class bdm::CellExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::CellExt<bdm::CompileTimeParam<bdm::Soa>,bdm::Capsule<bdm::CellExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::CellExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::CellExt<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::neuroscience::NeuriteExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::neuroscience::NeuriteExt<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuriteExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::neuroscience::NeuronExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt>,bdm::Cell_TCTParam_TDerived>+;
#pragma link C++ class bdm::neuroscience::NeuronExt<bdm::CompileTimeParam<bdm::Soa>,bdm::neuroscience::Capsule<bdm::neuroscience::NeuronExt>,bdm::Cell_TCTParam_TDerived>+;

#pragma link C++ class bdm::Variant<bdm::NullBiologyModule>-;

#endif
