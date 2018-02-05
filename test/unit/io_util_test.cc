#include "unit/io_util_test.h"

namespace bdm {

TEST(IOTest, InvalidRead) { RunInvalidReadTest(); }

TEST(IOTest, RuntimeVars) {
  RuntimeVariables this_machine;

  SysInfo_t si = this_machine.GetSystemInfo();
  si.fOS = "Non-Existing_OS";
  RuntimeVariables different_machine;
  different_machine.SetSystemInfo(si);

  if (this_machine == different_machine) {
    FAIL();
  }

  RuntimeVariables this_machine_copy;
  if (this_machine != this_machine_copy) {
    FAIL();
  }

  WritePersistentObject(ROOTFILE, "RuntimeVars", this_machine, "RECREATE");
  RuntimeVariables* this_machine_r = nullptr;
  GetPersistentObject(ROOTFILE, "RuntimeVars", this_machine_r);

  if (this_machine != *this_machine_r) {
    FAIL();
  }

  remove(ROOTFILE);
}

}  // namespace bdm
