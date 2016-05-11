%include "cx3d_shared_ptr.i"

%cx3d_shared_ptr(DividingModule,
                 ini/cx3d/swig/simulation/DividingModule,
                 cx3d::DividingModule);
%typemap(javaimports) cx3d::DividingModule "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(InternalSecretor,
                ini/cx3d/swig/simulation/InternalSecretor,
                cx3d::InternalSecretor);
%typemap(javaimports) cx3d::InternalSecretor "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(GrowthCone,
                 ini/cx3d/swig/simulation/GrowthCone,
                 cx3d::GrowthCone);
%typemap(javaimports) cx3d::GrowthCone "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(SomaRandomWalkModule,
                 ini/cx3d/swig/simulation/SomaRandomWalkModule,
                 cx3d::SomaRandomWalkModule);
%typemap(javaimports) cx3d::SomaRandomWalkModule "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(MembraneContact,
                 ini/cx3d/swig/simulation/MembraneContact,
                 cx3d::MembraneContact);
%typemap(javaimports) cx3d::MembraneContact "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(NeuriteChemoAttraction,
                 ini/cx3d/swig/simulation/NeuriteChemoAttraction,
                 cx3d::NeuriteChemoAttraction);
%typemap(javaimports) cx3d::NeuriteChemoAttraction "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(RandomBranchingModule,
                 ini/cx3d/swig/simulation/RandomBranchingModule,
                 cx3d::RandomBranchingModule);
%typemap(javaimports) cx3d::RandomBranchingModule "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(SomaClustering,
                 ini/cx3d/swig/simulation/SomaClustering,
                 cx3d::SomaClustering);
%typemap(javaimports) cx3d::SomaClustering "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(XAdhesiveForce,
                 ini/cx3d/swig/simulation/XAdhesiveForce,
                 cx3d::XAdhesiveForce);
%typemap(javaimports) cx3d::XAdhesiveForce "import ini.cx3d.swig.NativeStringBuilder;"


%define %XAdhesiveForce_cx3d_shared_ptr()
  %cx3d_shared_ptr(XAdhesiveForce,
                   ini/cx3d/physics/InterObjectForce,
                   cx3d::XAdhesiveForce);
%enddef

%define %XAdhesiveForce_native()
  %native_defined_class(cx3d::XAdhesiveForce,
                            XAdhesiveForce,
                            ini.cx3d.physics.InterObjectForce,
                            XAdhesiveForce,
                            ;);
%enddef

%define %XAdhesiveForce_typemaps()
  %typemap(javainterfaces) cx3d::XAdhesiveForce "ini.cx3d.physics.InterObjectForce"
  %typemap(javaimports) cx3d::XAdhesiveForce %{
    import ini.cx3d.swig.NativeStringBuilder;
%}
%enddef

%XAdhesiveForce_cx3d_shared_ptr();
%XAdhesiveForce_native();
%XAdhesiveForce_typemaps();

%cx3d_shared_ptr(XBifurcationModule,
                 ini/cx3d/swig/simulation/XBifurcationModule,
                 cx3d::XBifurcationModule);
%typemap(javaimports) cx3d::XBifurcationModule "import ini.cx3d.swig.NativeStringBuilder;"

%cx3d_shared_ptr(XMovementModule,
                 ini/cx3d/swig/simulation/XMovementModule,
                 cx3d::XMovementModule);
%typemap(javaimports) cx3d::XMovementModule "import ini.cx3d.swig.NativeStringBuilder;"
