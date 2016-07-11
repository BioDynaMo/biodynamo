{
   gROOT->ProcessLine(".I ../include");
   gROOT->ProcessLine(".I .");
   gROOT->ProcessLine(".I ../build/gtest/src/gtest/include");
   gSystem->Load("../build/libgmp.10.dylib");
   gSystem->Load("../build/libbiodynamo.dylib");
   printf("\nWelcome to the biodynamo test directory...\n");
}
