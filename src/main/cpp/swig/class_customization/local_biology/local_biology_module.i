/**
 * This file contains code generation customizations for class LocalBiologyModule.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (LOCALBIOLOGYMODULE_NATIVE and LOCALBIOLOGYMODULE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/local_biology/local_biology_module.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %LocalBiologyModule_cx3d_shared_ptr()
  %cx3d_shared_ptr(LocalBiologyModule,
                   ini/cx3d/localBiology/LocalBiologyModule,
                   cx3d::local_biology::LocalBiologyModule);
%enddef

%define %LocalBiologyModule_java()
  %java_defined_class_add(cx3d::local_biology::LocalBiologyModule,
                          LocalBiologyModule,
                          LocalBiologyModule,
                          ini.cx3d.localBiology.LocalBiologyModule,
                          ini/cx3d/localBiology/LocalBiologyModule,
                          public void run() {

                          }
                          public ini.cx3d.localBiology.interfaces.CellElement getCellElement() {
                              return null;
                          }
                          public void setCellElement(ini.cx3d.localBiology.interfaces.CellElement cellElement) {

                          }
                          public boolean isCopiedWhenNeuriteBranches() {
                            return false;
                          }
                          public boolean isCopiedWhenSomaDivides() {
                              return false;
                          }
                          public boolean isCopiedWhenNeuriteExtendsFromSoma() {
                              return false;
                          }
                          public boolean isDeletedAfterNeuriteHasBifurcated() {
                              return false;
                          }
                          public NativeStringBuilder simStateToJson(NativeStringBuilder sb) {
                              return null;
                          });
%enddef

%define %LocalBiologyModule_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::local_biology::LocalBiologyModule>,
                   LocalBiologyModule,
                   ini.cx3d.localBiology.LocalBiologyModule);
%enddef

/**
 * apply customizations
 */
%LocalBiologyModule_cx3d_shared_ptr();
%LocalBiologyModule_java();
%LocalBiologyModule_stdlist();
%typemap(javaimports) cx3d::local_biology::LocalBiologyModule "import ini.cx3d.swig.NativeStringBuilder;"
%typemap(javainterfaces) cx3d::local_biology::LocalBiologyModule "ini.cx3d.localBiology.LocalBiologyModule"
