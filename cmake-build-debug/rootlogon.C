{

  gROOT->ProcessLine(".include /usr/include");
  gROOT->ProcessLine(".include /home/uriel/.bdm/third_party/paraview/include/paraview-5.6");
  gROOT->ProcessLine(".include /home/uriel/.bdm/third_party/paraview/include");
  gROOT->ProcessLine(".include /home/testuser/bdm-build-third-party/paraview-build/install/include");
  gROOT->ProcessLine(".include /home/testuser/bdm-build-third-party/paraview-build/install/include/python2.7");
  gROOT->ProcessLine(".include /usr/lib/openmpi/include/openmpi/opal/mca/event/libevent2021/libevent");
  gROOT->ProcessLine(".include /usr/lib/openmpi/include/openmpi/opal/mca/event/libevent2021/libevent/include");
  gROOT->ProcessLine(".include /usr/lib/openmpi/include");
  gROOT->ProcessLine(".include /usr/lib/openmpi/include/openmpi");
  gROOT->ProcessLine(".include /home/uriel/.bdm/third_party/root/include");
  gROOT->ProcessLine(".include /home/uriel/Github/biodynamo/src");
  gROOT->ProcessLine(".include /home/uriel/Github/biodynamo/test");
  gROOT->ProcessLine(".include /home/uriel/Github/biodynamo/third_party");
  gROOT->ProcessLine(".include /home/uriel/Github/biodynamo/cmake-build-debug/extracted-third-party-libs");
  gROOT->ProcessLine(".include /home/uriel/Github/biodynamo/cmake-build-debug/version");
  gROOT->ProcessLine(".include /home/uriel/Github/biodynamo/build/omp");
  gROOT->ProcessLine(".L libbiodynamo.so");
}