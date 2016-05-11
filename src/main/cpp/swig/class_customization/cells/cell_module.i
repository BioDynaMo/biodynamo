/**
 * This file contains code generation customizations for class CellModule.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (CELLMODULE_NATIVE and CELLMODULE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/cells/cell_module.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %CellModule_cx3d_shared_ptr()
  %cx3d_shared_ptr(CellModule,
                   ini/cx3d/cells/CellModule,
                   cx3d::cells::CellModule);
%enddef

%define %CellModule_java()
  %java_defined_class_add(cx3d::cells::CellModule,
                          CellModule,
                          CellModule,
                          ini.cx3d.cells.CellModule,
                          ini/cx3d/cells/CellModule,
                          ;);
%enddef

%define %CellModule_stdlist()
  %stdlist_typemap_cross_module(std::shared_ptr<cx3d::cells::CellModule>,
                                CellModule,
                                ini.cx3d.cells.CellModule,
                                ini.cx3d.swig.simulation.CellModule);
%enddef

/**
 * apply customizations
 */
%CellModule_cx3d_shared_ptr();
%CellModule_java();
%CellModule_stdlist();
%typemap(javaimports) cx3d::cells::CellModule "import ini.cx3d.swig.NativeStringBuilder; import ini.cx3d.swig.simulation.CellElement;"
%typemap(javainterfaces) cx3d::cells::CellModule "ini.cx3d.cells.CellModule"
