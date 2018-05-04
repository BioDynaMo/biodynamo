#ifndef UNIT_SEPARATE_BINARY_SO_POINTER_TEST_H_
#define UNIT_SEPARATE_BINARY_SO_POINTER_TEST_H_

#include <gtest/gtest.h>

#include "compile_time_param.h"
#include "simulation_backup.h"
#include "simulation_object.h"
#include "so_pointer.h"
#include "unit/io_test.h"

namespace bdm {
namespace so_pointer_test_internal {

// SoPointer tests
/// This function is before the decleration of `SoPointerTestClass` to test
/// if the SoPointer implementation can deal with incomplete types
template <typename T, typename TBackend>
void SoPointerTest(T* sim_objects) {
  using SO = typename T::value_type;

  SoPointer<SO, TBackend> null_so_pointer;
  EXPECT_TRUE(null_so_pointer == nullptr);

  SoPointer<SO, TBackend> so_ptr(sim_objects, 0);

  EXPECT_TRUE(so_ptr != nullptr);
  EXPECT_EQ(123u, so_ptr->GetId());

  so_ptr = nullptr;
  EXPECT_TRUE(so_ptr == nullptr);
}

BDM_SIM_OBJECT(SoPointerTestClass, bdm::SimulationObject) {
  BDM_SIM_OBJECT_HEADER(SoPointerTestClassExt, 1, my_so_ptr_, id_);

 public:
  SoPointerTestClassExt() {}
  SoPointerTestClassExt(uint64_t id) { id_[kIdx] = id; }

  uint64_t GetId() const { return id_[kIdx]; }
  void SetId(uint64_t id) { id_[kIdx] = id; }

  MostDerivedSoPtr GetMySoPtr() const { return my_so_ptr_[kIdx]; }
  void SetMySoPtr(MostDerivedSoPtr so_ptr) { my_so_ptr_[kIdx] = so_ptr; }

  vec<MostDerivedSoPtr> my_so_ptr_ = {{}};

 private:
  vec<uint64_t> id_;
};

}  // namespace so_pointer_test_internal

// has to be defined in namespace bdm
template <typename TBackend>
struct CompileTimeParam : public DefaultCompileTimeParam<TBackend> {
  using AtomicTypes =
      VariadicTypedef<so_pointer_test_internal::SoPointerTestClass>;
};

namespace so_pointer_test_internal {

inline void IOTestSoPointerAnyContainerAos() {
  TransactionalVector<SoPointerTestClass> sim_objects;
  sim_objects.push_back(SoPointerTestClass(123));
  sim_objects.push_back(SoPointerTestClass(456));

  SoPointer<SoPointerTestClass, Scalar> soptr(&sim_objects, 1);
  SoPointer<SoPointerTestClass, Scalar>* restored;

  BackupAndRestore(soptr, &restored);

  EXPECT_TRUE(*restored != nullptr);
  EXPECT_EQ(1u, restored->GetElementIdx());
  EXPECT_EQ(456u, (*restored)->GetId());
}

inline void IOTestSoPointerAnyContainerSoa() {
  auto sim_objects = SoPointerTestClass::NewEmptySoa();
  SoPointerTestClass so(123);
  sim_objects.push_back(SoPointerTestClass(123));
  sim_objects.push_back(SoPointerTestClass(456));

  SoPointer<decltype(sim_objects), Soa> soptr(&sim_objects, 1);
  SoPointer<decltype(sim_objects), Soa>* restored;

  BackupAndRestore(soptr, &restored);

  EXPECT_TRUE(*restored != nullptr);
  EXPECT_EQ(1u, restored->GetElementIdx());
  EXPECT_EQ(456u, (*restored)->GetId());
}

inline void IOTestSoPointerRmContainerSoa() {
  Rm()->Clear();
  auto&& so1 = Rm()->New<SoPointerTestClass>(123);
  auto&& so2 = Rm()->New<SoPointerTestClass>(456);

  auto soptr = so1.GetSoPtr();
  EXPECT_EQ(0u, soptr.GetElementIdx());
  so2.SetMySoPtr(soptr);

  auto* rm_before = Rm();

  SimulationBackup backup(IOTest::kRootFile, "");
  backup.Backup(1);

  SimulationBackup restore("", IOTest::kRootFile);
  restore.Restore();

  EXPECT_TRUE(rm_before != Rm());

  auto* restored_sim_objects = Rm()->Get<SoPointerTestClass>();
  EXPECT_EQ(123u, (*restored_sim_objects)[1].GetMySoPtr()->GetId());
  // change id of first element
  (*restored_sim_objects)[0].SetId(987);
  // id should have changed
  EXPECT_EQ(987u, (*restored_sim_objects)[1].GetMySoPtr()->GetId());
}

inline void IOTestSoPointerNullptrAos() {
  SoPointer<SoPointerTestClass, Scalar> null_so_pointer;
  SoPointer<SoPointerTestClass, Scalar>* restored = nullptr;

  BackupAndRestore(null_so_pointer, &restored);

  EXPECT_TRUE(*restored == nullptr);
}

inline void IOTestSoPointerNullptrSoa() {
  SoPointer<SoaSoPointerTestClass, Soa> null_so_pointer;
  SoPointer<SoaSoPointerTestClass, Soa>* restored = nullptr;

  BackupAndRestore(null_so_pointer, &restored);

  EXPECT_TRUE(*restored == nullptr);
}

}  // namespace so_pointer_test_internal

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_SO_POINTER_TEST_H_
