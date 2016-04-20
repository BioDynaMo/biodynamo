/**
 * SWIG modules that use the class simply include it:
 * %include "class_customization/cells/cell.i"
 */

#ifdef CELLFACTORY_NATIVE
  %pragma(java) modulecode=%{
      public static boolean useNativeCellFactory = true;
  %}
#else
  %pragma(java) modulecode=%{
      public static boolean useNativeCellFactory = false;
  %}
#endif
%typemap(javaimports) cx3d::cells::CellFactory %{
  import ini.cx3d.swig.physics.ECM;
%}
