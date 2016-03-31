/**
 * This file contains code generation customizations for class PhysicalCylinder.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/physical_object.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %PhysicalCylinder_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalCylinder,
                   ini/cx3d/physics/interfaces/PhysicalCylinder,
                   cx3d::physics::PhysicalCylinder);
%enddef

%define %PhysicalCylinder_java()
  %java_defined_class_add(cx3d::physics::PhysicalCylinder,
                      PhysicalCylinder,
                      PhysicalCylinder,
                      ini.cx3d.physics.interfaces.PhysicalCylinder,
                      ini/cx3d/physics/interfaces/PhysicalCylinder,
                      public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                        return super.simStateToJson(sb);
                      });
%enddef

%define %PhysicalCylinder_native()
  %native_defined_class(cx3d::physics::PhysicalCylinder,
                      PhysicalCylinder,
                      ini.cx3d.physics.interfaces.PhysicalCylinder,
                      PhysicalCylinder,
                      public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                        return super.simStateToJson(sb);
                      });
%enddef

%define %PhysicalCylinder_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::physics::PhysicalCylinder>,
                              shared_ptr_PhysicalCylinder_##SIZE,
                              ini.cx3d.physics.interfaces.PhysicalCylinder,
                              Lini/cx3d/physics/interfaces/PhysicalCylinder;,
                              SIZE);
%enddef

/**
 * apply customizations
 */
%PhysicalCylinder_cx3d_shared_ptr();
#ifdef PHYSICALCYLINDER_NATIVE
  %PhysicalCylinder_native();
#else
  %PhysicalCylinder_java();
#endif

#ifdef PHYSICALCYLINDER_DEBUG
  %setJavaDebugSwitch(PhysicalCylinder, true);
#else
  %setJavaDebugSwitch(PhysicalCylinder, false);
#endif

%typemap(javaimports) cx3d::physics::PhysicalCylinder "import ini.cx3d.swig.NativeStringBuilder;"
%typemap(javainterfaces) cx3d::physics::PhysicalCylinder "ini.cx3d.physics.interfaces.PhysicalCylinder"
%stdlist_typemap(std::shared_ptr<cx3d::physics::PhysicalCylinder>,
                 PhysicalCylinder,
                 ini.cx3d.physics.interfaces.PhysicalCylinder);
%PhysicalCylinder_stdarray_array_marshalling(physics, 2);
