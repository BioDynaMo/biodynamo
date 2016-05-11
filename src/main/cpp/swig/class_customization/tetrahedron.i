/**
 * This file contains code generation customizations for class Tetrahedron.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (TETRAHEDRON_NATIVE and TETRAHEDRON_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it using:
 * %include "class_customization/tetrahedron.i"
 */

%include "util.i"
%include "std_array_typemap.i"
%include "std_list_typemap.i"

%define %Tetrahedron_cx3d_shared_ptr()
  %cx3d_shared_ptr(TetrahedronT_PhysicalNode,
                   ini/cx3d/spatialOrganization/interfaces/Tetrahedron,
                   cx3d::spatial_organization::Tetrahedron<cx3d::physics::PhysicalNode>);
%enddef

%define %Tetrahedron_java()
  %java_defined_class(cx3d::spatial_organization::Tetrahedron<cx3d::physics::PhysicalNode>,
                      TetrahedronT_PhysicalNode,
                      Tetrahedron,
                      ini.cx3d.spatialOrganization.interfaces.Tetrahedron,
                      ini/cx3d/spatialOrganization/interfaces/Tetrahedron);
  %typemap(javainterfaces) cx3d::spatial_organization::Tetrahedron<cx3d::physics::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.Tetrahedron"
%enddef

%define %Tetrahedron_native()
  %native_defined_class(cx3d::spatial_organization::Tetrahedron<cx3d::physics::PhysicalNode>,
                        TetrahedronT_PhysicalNode,
                        ini.cx3d.spatialOrganization.interfaces.Tetrahedron,
                        Tetrahedron,
                        public TetrahedronT_PhysicalNode(){});
%enddef

%define %Tetrahedron_typemaps()
  %Tetrahedron_stdlist();
  %Tetrahedron_stdarray_array_marshalling(simulation, 2);
  %Tetrahedron_stdarray_array_marshalling(simulation, 3);
  %Tetrahedron_type_modification();
  %int_stdarray_array_marshalling(simulation, 4);
%enddef

%define %Tetrahedron_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::spatial_organization::Tetrahedron<cx3d::physics::PhysicalNode> >,
                              shared_ptr_Tetrahedron##SIZE,
                              ini.cx3d.spatialOrganization.interfaces.Tetrahedron,
                              Lini/cx3d/spatialOrganization/interfaces/Tetrahedron;,
                              SIZE);
%enddef

%define %Tetrahedron_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::spatial_organization::Tetrahedron<cx3d::physics::PhysicalNode>>,
                   Tetrahedron,
                   ini.cx3d.spatialOrganization.interfaces.Tetrahedron);
%enddef

/**
 * apply customizations
 */
%Tetrahedron_cx3d_shared_ptr();
#ifdef TETRAHEDRON_NATIVE
  %Tetrahedron_native();
#else
  %Tetrahedron_java();
#endif
#ifdef TETRAHEDRON_DEBUG
  %setJavaDebugSwitch(Tetrahedron, true);
#else
  %setJavaDebugSwitch(Tetrahedron, false);
#endif
%Tetrahedron_typemaps();
