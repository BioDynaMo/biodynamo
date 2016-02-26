/**
 * This file contains code generation customizations for class SimpleTriangulationNodeOrganizer.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/simple_triangulation_node_organizer.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_array_typemap.i"
%include "std_list_typemap.i"

%define %SimpleTriangulationNodeOrganizer_cx3d_shared_ptr()
  %cx3d_shared_ptr(SimpleTriangulationNodeOrganizerT_PhysicalNode,
                   ini/cx3d/spatialOrganization/interfaces/TriangulationNodeOrganizer,
                   cx3d::spatial_organization::SimpleTriangulationNodeOrganizer<cx3d::physics::PhysicalNode>);
%enddef

%define %SimpleTriangulationNodeOrganizer_java()
  %java_defined_class_add(cx3d::spatial_organization::SimpleTriangulationNodeOrganizer<cx3d::physics::PhysicalNode>,
                          SimpleTriangulationNodeOrganizerT_PhysicalNode,
                          SimpleTriangulationNodeOrganizer,
                          ini.cx3d.spatialOrganization.interfaces.TriangulationNodeOrganizer,
                          ini/cx3d/spatialOrganization/interfaces/TriangulationNodeOrganizer,
                          @Override
                          protected java.util.Iterator<ini.cx3d.spatialOrganization.interfaces.SpaceNode> getNodeIterator(ini.cx3d.spatialOrganization.interfaces.SpaceNode referencePoint) {
                              return null;
                          });
  %typemap(javabase) cx3d::spatial_organization::SimpleTriangulationNodeOrganizer<cx3d::physics::PhysicalNode> "ini.cx3d.spatialOrganization.AbstractTriangulationNodeOrganizer"
%enddef

%define %SimpleTriangulationNodeOrganizer_native()
  %native_defined_class_add(cx3d::spatial_organization::SimpleTriangulationNodeOrganizer<cx3d::physics::PhysicalNode>,
                            SimpleTriangulationNodeOrganizerT_PhysicalNode,
                            ini.cx3d.spatialOrganization.interfaces.TriangulationNodeOrganizer,
                            SimpleTriangulationNodeOrganizer,
                            ;,
                            @Override
                            protected java.util.Iterator<ini.cx3d.spatialOrganization.interfaces.SpaceNode> getNodeIterator(ini.cx3d.spatialOrganization.interfaces.SpaceNode referencePoint) {
                                return null;
                            });
  %typemap(javabase) cx3d::spatial_organization::SimpleTriangulationNodeOrganizer<cx3d::physics::PhysicalNode> "ini.cx3d.spatialOrganization.AbstractTriangulationNodeOrganizer"
%enddef

 /**
  * apply customizations
  */
 %SimpleTriangulationNodeOrganizer_cx3d_shared_ptr();
 #ifdef SIMPLETRIANGULATIONNODEORGANIZER_NATIVE
   %SimpleTriangulationNodeOrganizer_native();
 #else
   %SimpleTriangulationNodeOrganizer_java();
 #endif
 #ifdef SIMPLETRIANGULATIONNODEORGANIZER_DEBUG
   %setJavaDebugSwitch(SimpleTriangulationNodeOrganizer, true);
 #else
   %setJavaDebugSwitch(SimpleTriangulationNodeOrganizer, false);
 #endif
