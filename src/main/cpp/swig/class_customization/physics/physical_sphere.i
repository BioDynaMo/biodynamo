/**
 * This file contains code generation customizations for class PhysicalSphere.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/physical_sphere.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %PhysicalSphere_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalSphere,
                   ini/cx3d/physics/interfaces/PhysicalSphere,
                   cx3d::physics::PhysicalSphere);
%enddef

%define %PhysicalSphere_java()
  %java_defined_class_add(cx3d::physics::PhysicalSphere,
                      PhysicalSphere,
                      PhysicalSphere,
                      ini.cx3d.physics.interfaces.PhysicalSphere,
                      ini/cx3d/physics/interfaces/PhysicalSphere,
                      public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                        return super.simStateToJson(sb);
                      });
%enddef

%define %PhysicalSphere_native()
  %native_defined_class(cx3d::physics::PhysicalSphere,
                      PhysicalSphere,
                      ini.cx3d.physics.interfaces.PhysicalSphere,
                      PhysicalSphere,
                      public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                        return super.simStateToJson(sb);
                      });
%enddef

/**
 * apply customizations
 */
%PhysicalSphere_cx3d_shared_ptr();
#ifdef PHYSICALSPHERE_NATIVE
  %PhysicalSphere_native();
#else
  %PhysicalSphere_java();
#endif

#ifdef PHYSICALSPHERE_DEBUG
  %setJavaDebugSwitch(PhysicalSphere, true);
#else
  %setJavaDebugSwitch(PhysicalSphere, false);
#endif

%stdlist_typemap_cross_module(std::shared_ptr<cx3d::physics::PhysicalSphere>,
                              PhysicalSphere,
                              ini.cx3d.physics.interfaces.PhysicalSphere,
                              ini.cx3d.swig.physics.PhysicalSphere);

%typemap(javaimports) cx3d::physics::PhysicalSphere %{
  import ini.cx3d.swig.NativeStringBuilder;
  import ini.cx3d.swig.biology.CellElement;
  import ini.cx3d.swig.biology.SomaElement;
%}
%typemap(javainterfaces) cx3d::physics::PhysicalSphere "ini.cx3d.physics.interfaces.PhysicalSphere"
