#ifdef __ROOTCLING__
// turns off dictionary generation for all
#pragma link off all class;
#pragma link off all function;
#pragma link off all global;
#pragma link off all typedef;


// should be in libbiodynamo
#pragma link C++ class bdm::NullBiologyModule+;
#pragma link C++ class bdm::BaseBiologyModule+;
#pragma link C++ class bdm::RuntimeVariables+;
#pragma link C++ class bdm::ScalarSimulationObject+;
#pragma link C++ class bdm::GrowDivide+;
#pragma link C++ class bdm::IntegralTypeWrapper<size_t>+;
// end should be in libbiodynamo

// had issues with
#pragma link C++ class std::tuple<bdm::CellExt<bdm::CompileTimeParam<bdm::Soa>,bdm::Capsule<bdm::CellExt>,bdm::SimulationObject_TCTParam_TDerived> >;
#pragma link C++ class std::tuple<bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>,bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> >;
#pragma link C++ class std::vector<bdm::Variant<int,double,char> >;  //VariantTest.IOVector
#pragma link C++ class std::vector<bdm::InlineVector<int,8> >;  //InlineVectorTest.IO

//   ResourceManagerTest.IOAos
#pragma link C++ class std::tuple<bdm::TransactionalVector<bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived> >,bdm::TransactionalVector<bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> > >;
#pragma link C++ class std::tuple<bdm::TransactionalVector<bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived> >,bdm::TransactionalVector<bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> > >: bdm::TransactionalVector<bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> >;
#pragma link C++ class std::tuple<bdm::TransactionalVector<bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived> >,bdm::TransactionalVector<bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> > >: bdm::TransactionalVector<bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived> >;
#pragma link C++ class bdm::TransactionalVector<bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> >;
#pragma link C++ class bdm::TransactionalVector<bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived> >;

// end had issues with

#pragma link C++ class bdm::OneElementArray<array<double,3> >+;
#pragma link C++ class bdm::OneElementArray<double>+;
#pragma link C++ class bdm::OneElementArray<vector<bdm::Variant<bdm::biology_module_op_test_internal::GrowthModule> > >+;
#pragma link C++ class bdm::OneElementArray<unsigned long>+;
#pragma link C++ class bdm::OneElementArray<bdm::InlineVector<int,8> >+;
#pragma link C++ class bdm::OneElementArray<vector<bdm::Variant<bdm::cell_test_internal::GrowthModule,bdm::cell_test_internal::MovementModule> > >+;
#pragma link C++ class bdm::OneElementArray<bool>+;
#pragma link C++ class bdm::OneElementArray<int>+;
#pragma link C++ class bdm::OneElementArray<vector<bdm::Variant<bdm::NullBiologyModule> > >+;
#pragma link C++ class bdm::InlineVector<bdm::SoHandle,4>+;
#pragma link C++ class bdm::InlineVector<array<double,3>,4>+;
#pragma link C++ class bdm::InlineVector<int,8>+;
#pragma link C++ class bdm::DiffusionGrid+;
#pragma link C++ class bdm::ResourceManager<bdm::CompileTimeParam<bdm::Soa> >+;
#pragma link C++ class bdm::ResourceManager<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Soa> >+;
#pragma link C++ class bdm::ResourceManager<bdm::resource_manager_test_internal::CompileTimeParam<bdm::Scalar,bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>,bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> > >+;
#pragma link C++ class bdm::ResourceManager<bdm::resource_manager_test_internal::CompileTimeParam<bdm::Scalar,bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>,bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> > >+;
#pragma link C++ class bdm::ResourceManager<bdm::resource_manager_test_internal::CompileTimeParam<bdm::Soa,bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>,bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> > >+;
#pragma link C++ class bdm::ResourceManager<bdm::resource_manager_test_internal::CompileTimeParam<bdm::Soa,bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>,bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived> > >+;
#pragma link C++ class bdm::SimulationObject<bdm::simulation_object_test_internal::CTParam<bdm::Soa>,bdm::simulation_object_test_internal::TestCapsule<bdm::SimulationObject> >+;
#pragma link C++ class bdm::SimulationObject<bdm::simulation_object_test_internal::CTParam<bdm::Scalar>,bdm::simulation_object_test_internal::TestCapsule<bdm::SimulationObject> >+;
#pragma link C++ class bdm::SimulationObject<bdm::biology_module_op_test_internal::CTParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::biology_module_op_test_internal::CTParam<bdm::Soa>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::cell_test_internal::CTParam<bdm::Scalar>,bdm::cell_test_internal::Capsule<bdm::cell_test_internal::TestCellExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Scalar>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::AExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Scalar>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::BExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Soa>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::BExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Soa>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::AExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt> >+;
#pragma link C++ class bdm::SimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::simulation_object_test_internal::CTParam<bdm::Soa>,bdm::simulation_object_test_internal::TestCapsule<bdm::SimulationObject> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::biology_module_op_test_internal::CTParam<bdm::Soa>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Soa>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::BExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Soa>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::AExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt> >+;
#pragma link C++ class bdm::SoaSimulationObject<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::simulation_object_test_internal::CTParam<bdm::Scalar>,bdm::simulation_object_test_internal::TestCapsule<bdm::SimulationObject> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::biology_module_op_test_internal::CTParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::cell_test_internal::CTParam<bdm::Scalar>,bdm::cell_test_internal::Capsule<bdm::cell_test_internal::TestCellExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::CompileTimeParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Scalar>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::AExt> >+;
#pragma link C++ class bdm::ScalarSimulationObject<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Scalar>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::BExt> >+;
#pragma link C++ class bdm::CellExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::CellExt<bdm::CompileTimeParam<bdm::Soa>,bdm::Capsule<bdm::CellExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::CellExt<bdm::biology_module_op_test_internal::CTParam<bdm::Scalar>,bdm::Capsule<bdm::CellExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::CellExt<bdm::biology_module_op_test_internal::CTParam<bdm::Soa>,bdm::Capsule<bdm::CellExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::CellExt<bdm::cell_test_internal::CTParam<bdm::Scalar>,bdm::cell_test_internal::Capsule<bdm::cell_test_internal::TestCellExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::biology_module_op_test_internal::GrowthModule+;
#pragma link C++ class bdm::biology_module_util_test_internal::RunTestBiologyModule<bdm::biology_module_util_test_internal::TestSimulationObject>+;
#pragma link C++ class bdm::biology_module_util_test_internal::CopyTestBiologyModule+;
#pragma link C++ class bdm::cell_test_internal::GrowthModule+;
#pragma link C++ class bdm::cell_test_internal::MovementModule+;
#pragma link C++ class bdm::cell_test_internal::TestCellExt<bdm::cell_test_internal::CTParam<bdm::Scalar>,bdm::cell_test_internal::Capsule<bdm::cell_test_internal::TestCellExt>,bdm::Cell_TCTParam_TDerived>+;
#pragma link C++ class bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::resource_manager_test_internal::AExt<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Scalar>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::resource_manager_test_internal::BExt<bdm::CompileTimeParam<bdm::Soa>,bdm::resource_manager_test_internal::Capsule<bdm::resource_manager_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::simulation_object_vector_test_internal::AExt<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Scalar>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::simulation_object_vector_test_internal::AExt<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Soa>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::AExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::simulation_object_vector_test_internal::BExt<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Scalar>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived>+;
#pragma link C++ class bdm::simulation_object_vector_test_internal::BExt<bdm::simulation_object_vector_test_internal::CompileTimeParam1<bdm::Soa>,bdm::simulation_object_vector_test_internal::Capsule<bdm::simulation_object_vector_test_internal::BExt>,bdm::SimulationObject_TCTParam_TDerived>+;

#pragma link C++ class bdm::Variant<bdm::NullBiologyModule>-;
#pragma link C++ class bdm::Variant<int,double,char>-;
#pragma link C++ class bdm::Variant<bdm::biology_module_op_test_internal::GrowthModule>-;
#pragma link C++ class bdm::Variant<bdm::cell_test_internal::GrowthModule,bdm::cell_test_internal::MovementModule>-;
#pragma link C++ class bdm::Variant<bdm::biology_module_util_test_internal::RunTestBiologyModule<bdm::biology_module_util_test_internal::TestSimulationObject> >-;


#endif
