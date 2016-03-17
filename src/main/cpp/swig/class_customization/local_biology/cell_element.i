/**
 * This file contains code generation customizations for class CellElement.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (CELLELEMENT_NATIVE and CELLELEMENT_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/local_biology/cell_element.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %CellElement_cx3d_shared_ptr()
  %cx3d_shared_ptr(CellElement,
                   ini/cx3d/localBiology/CellElement,
                   cx3d::local_biology::CellElement);
%enddef

%define %CellElement_java()
  %java_defined_class(cx3d::local_biology::CellElement,
                      CellElement,
                      CellElement,
                      ini.cx3d.localBiology.CellElement,
                      ini/cx3d/localBiology/CellElement);
%enddef

%define %CellElement_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::local_biology::CellElement>,
                   CellElement,
                   ini.cx3d.localBiology.CellElement);
%enddef

/**
 * apply customizations
 */
%CellElement_cx3d_shared_ptr();
%CellElement_java();
%CellElement_stdlist();
%typemap(javaimports) cx3d::local_biology::CellElement "import ini.cx3d.swig.NativeStringBuilder;"
