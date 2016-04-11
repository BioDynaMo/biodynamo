/**
 * This file contains code generation customizations for class SomaElement.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (SOMAELEMENT_NATIVE and SOMAELEMENT_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/local_biology/soma_element.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %SomaElement_cx3d_shared_ptr()
  %cx3d_shared_ptr(SomaElement,
                   ini/cx3d/localBiology/interfaces/SomaElement,
                   cx3d::local_biology::SomaElement);
%enddef

%define %SomaElement_java()
  %java_defined_class_add(cx3d::local_biology::SomaElement,
                      SomaElement,
                      SomaElement,
                      ini.cx3d.localBiology.interfaces.SomaElement,
                      ini/cx3d/localBiology/interfaces/SomaElement,
                      public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                        return super.simStateToJson(sb);
                      });
%enddef

%define %SomaElement_stdlist()
  %stdlist_typemap_cross_module(std::shared_ptr<cx3d::local_biology::SomaElement>,
                                SomaElement,
                                ini.cx3d.localBiology.interfaces.SomaElement,
                                ini.cx3d.swig.biology.SomaElement);
%enddef

%define %SomaElement_native()
  %native_defined_class(cx3d::local_biology::SomaElement,
                      SomaElement,
                      ini.cx3d.localBiology.interfaces.SomaElement,
                      SomaElement,
                      public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                        return super.simStateToJson(sb);
                      });
%enddef

/**
 * apply customizations
 */
%SomaElement_cx3d_shared_ptr();

#ifdef SOMAELEMENT_NATIVE
  %SomaElement_native();
#else
  %SomaElement_java();
#endif

%SomaElement_stdlist();
%typemap(javaimports) cx3d::local_biology::SomaElement %{
  import ini.cx3d.swig.NativeStringBuilder;
  import ini.cx3d.swig.physics.PhysicalObject;
  import ini.cx3d.swig.physics.PhysicalSphere;
%}
%typemap(javainterfaces) cx3d::local_biology::SomaElement "ini.cx3d.localBiology.interfaces.SomaElement"
