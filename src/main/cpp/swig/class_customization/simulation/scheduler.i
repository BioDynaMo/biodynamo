/**
 * This file contains code generation customizations for class Scheduler.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (SCHEDULER_NATIVE and SCHEDULER_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/simulation/scheduler.i"
 */

%include "util.i"
%include "std_list_typemap.i"
%include "cx3d_shared_ptr.i"

%define %Scheduler_cx3d_shared_ptr()
  %cx3d_shared_ptr(Scheduler,
                   ini/cx3d/swig/biology/Scheduler,
                   cx3d::simulation::Scheduler);
%enddef

%define %Scheduler_typemaps()
  %typemap(javaimports) cx3d::simulation::Scheduler "import ini.cx3d.swig.biology.ECM;"
%enddef

 /**
  * apply customizations
  */
 %Scheduler_cx3d_shared_ptr();
 #ifdef SCHEDULER_NATIVE
 %pragma(java) modulecode=%{
     public static boolean useNativeScheduler = true;
 %}
#else
 %pragma(java) modulecode=%{
     public static boolean useNativeScheduler = false;
 %}
#endif
 %Scheduler_typemaps();
