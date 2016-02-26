/**
 * This file contains code generation customizations for class IntracellularIntracellularIntracellularSubstance.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (INTRACELLULARSUBSTANCE_NATIVE and INTRACELLULARSUBSTANCE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/intracellular_substance.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %IntracellularSubstance_cx3d_shared_ptr()
  %cx3d_shared_ptr(IntracellularSubstance,
                   ini/cx3d/physics/interfaces/IntracellularSubstance,
                   cx3d::physics::IntracellularSubstance);
%enddef

%define %IntracellularSubstance_java()
  %java_defined_class_add(cx3d::physics::IntracellularSubstance,
                          IntracellularSubstance,
                          IntracellularSubstance,
                          ini.cx3d.physics.interfaces.IntracellularSubstance,
                          ini/cx3d/physics/interfaces/IntracellularSubstance,
                          protected IntracellularSubstance(IntracellularSubstance templateIntracellularSubstance) {
                              super(templateIntracellularSubstance);
                          }
                          protected IntracellularSubstance(String id, double diffusion_constant, double degradation_constant, boolean foo){
                              super(id, diffusion_constant, degradation_constant);
                          }
                          public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                              return super.simStateToJson(sb);
                          });
   %typemap(javaimports) cx3d::physics::IntracellularSubstance "import ini.cx3d.swig.NativeStringBuilder; import ini.cx3d.physics.Substance;"
%enddef

%define %IntracellularSubstance_native()
  %native_defined_class(cx3d::physics::IntracellularSubstance,
                        IntracellularSubstance,
                        ini.cx3d.physics.interfaces.IntracellularSubstance,
                        IntracellularSubstance,
                        protected IntracellularSubstance(IntracellularSubstance templateIntracellularSubstance) {
                            throw new UnsupportedOperationException();
                        }
                        protected IntracellularSubstance(String id, double diffusion_constant, double degradation_constant, boolean foo){
                            throw new UnsupportedOperationException();
                        }
                        public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                            throw new UnsupportedOperationException();
                        }
                        public java.util.concurrent.locks.ReadWriteLock getRwLock() {
                      		throw new UnsupportedOperationException();
                      	}
                        protected double degradationConstant = 0.01;
                        protected double concentration = 0.01;);
  %typemap(javaimports) cx3d::physics::IntracellularSubstance "import ini.cx3d.swig.NativeStringBuilder;"
%enddef

%define %IntracellularSubstance_typemaps()
  %typemap(javainterfaces) cx3d::physics::IntracellularSubstance "ini.cx3d.physics.interfaces.IntracellularSubstance"
  %pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder;"
%enddef

 /**
  * apply customizations
  */
 %IntracellularSubstance_cx3d_shared_ptr();
 #ifdef INTRACELLULARSUBSTANCE_NATIVE
   %IntracellularSubstance_native();
 #else
   %IntracellularSubstance_java();
 #endif
 #ifdef INTRACELLULARSUBSTANCE_DEBUG
   %setJavaDebugSwitch(IntracellularSubstance, true);
 #else
   %setJavaDebugSwitch(IntracellularSubstance, false);
 #endif
 %IntracellularSubstance_typemaps();
