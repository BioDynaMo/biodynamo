{
   gInterpreter->AddIncludePath("../include");
   gInterpreter->AddIncludePath(".");
   gInterpreter->AddIncludePath("../build/gtest/src/gtest/include");
   gSystem->Load("../build/libbiodynamo.dylib");
   printf("\nWelcome to the biodynamo test directory...\n");
}
