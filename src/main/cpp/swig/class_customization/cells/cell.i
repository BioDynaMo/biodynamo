/**
 * This file contains code generation customizations for class Cell.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (CELL_NATIVE and CELL_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/cells/cell.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %Cell_cx3d_shared_ptr()
  %cx3d_shared_ptr(Cell,
                   ini/cx3d/cells/interfaces/Cell,
                   cx3d::cells::Cell);
%enddef

%define %Cell_java()
  %java_defined_class(cx3d::cells::Cell,
                      Cell,
                      Cell,
                      ini.cx3d.cells.interfaces.Cell,
                      ini/cx3d/cells/interfaces/Cell);
%enddef

%define %Cell_stdlist()
  %stdlist_typemap_cross_module(std::shared_ptr<cx3d::cells::Cell>,
                                Cell,
                                ini.cx3d.cells.interfaces.Cell,
                                ini.cx3d.swig.biology.Cell);
%enddef

%define %Cell_native()
  %native_defined_class(cx3d::cells::Cell,
                      Cell,
                      ini.cx3d.cells.interfaces.Cell,
                      Cell,
                      ;);
%enddef

/**
 * apply customizations
 */
%Cell_cx3d_shared_ptr();
#ifdef CELL_NATIVE
  %Cell_native();
#else
  %Cell_java();
#endif

%Cell_stdlist();

%typemap(javainterfaces) cx3d::cells::Cell "ini.cx3d.cells.interfaces.Cell"
%typemap(javaimports) cx3d::cells::Cell %{
  import ini.cx3d.swig.NativeStringBuilder;
  import ini.cx3d.swig.biology.ECM;
%}
