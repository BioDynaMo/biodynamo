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
