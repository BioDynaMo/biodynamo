/**
 * This file contains code generation customizations for class NeuriteElement.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (NEURITEELEMENT_NATIVE and NEURITEELEMENT_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/local_biology/neurite_element.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"
%include "std_array_typemap.i"

%define %NeuriteElement_cx3d_shared_ptr()
  %cx3d_shared_ptr(NeuriteElement,
                   ini/cx3d/localBiology/interfaces/NeuriteElement,
                   cx3d::local_biology::NeuriteElement);
%enddef

%define %NeuriteElement_java()
  %java_defined_class_add(cx3d::local_biology::NeuriteElement,
                          NeuriteElement,
                          NeuriteElement,
                          ini.cx3d.localBiology.interfaces.NeuriteElement,
                          ini/cx3d/localBiology/interfaces/NeuriteElement,
                          public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                            return super.simStateToJson(sb);
                          });
%enddef

%define %NeuriteElement_native()
  %native_defined_class(cx3d::local_biology::NeuriteElement,
                      NeuriteElement,
                      ini.cx3d.localBiology.interfaces.NeuriteElement,
                      NeuriteElement,
                      public NativeStringBuilder superSuperSimStateToJson(NativeStringBuilder sb) {
                        return super.simStateToJson(sb);
                      });
%enddef

%define %NeuriteElement_stdlist()
  %typemap(jstype)  std::list<std::shared_ptr<cx3d::local_biology::NeuriteElement>>& "java.util.AbstractSequentialList<ini.cx3d.localBiology.interfaces.NeuriteElement>"
  %stdlist_typemap_cross_module(std::shared_ptr<cx3d::local_biology::NeuriteElement>,
                                NeuriteElement,
                                ini.cx3d.localBiology.interfaces.NeuriteElement,
                                ini.cx3d.swig.biology.NeuriteElement);
%enddef

%define %NeuriteElement_array(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling_cross_module(SWIG_MODULE,
                                           std::shared_ptr<cx3d::local_biology::NeuriteElement>,
                                           shared_ptr_NeuriteElement_##SIZE,
                                           ini.cx3d.localBiology.interfaces.NeuriteElement,
                                           Lini/cx3d/localBiology/interfaces/NeuriteElement;,
                                           SIZE,
                                           ini.cx3d.swig.biology.NeuriteElement);
%enddef

/**
 * apply customizations
 */
%NeuriteElement_cx3d_shared_ptr();
#ifdef NEURITEELEMENT_NATIVE
  %NeuriteElement_native();
#else
  %NeuriteElement_java();
#endif
%NeuriteElement_stdlist();
%typemap(javaimports) cx3d::local_biology::NeuriteElement %{
  import ini.cx3d.swig.NativeStringBuilder;
  import ini.cx3d.swig.biology.LocalBiologyModule;
  import ini.cx3d.swig.physics.PhysicalObject;
  import ini.cx3d.swig.physics.PhysicalCylinder;
%}
%typemap(javainterfaces) cx3d::local_biology::NeuriteElement "ini.cx3d.localBiology.interfaces.NeuriteElement"
%NeuriteElement_array(biology, 2);
