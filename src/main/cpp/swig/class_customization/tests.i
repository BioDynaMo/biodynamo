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
