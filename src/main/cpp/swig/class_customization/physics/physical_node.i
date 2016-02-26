/**
 * This file contains code generation customizations for class PhysicalNode.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physical_node.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"
%include "std_array_typemap.i"

%define %PhysicalNode_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalNode,
                   ini/cx3d/physics/interfaces/PhysicalNode,
                   cx3d::physics::PhysicalNode);
%enddef

%define %PhysicalNode_java()
  %java_defined_class(cx3d::physics::PhysicalNode,
                      PhysicalNode,
                      PhysicalNode,
                      ini.cx3d.physics.interfaces.PhysicalNode,
                      ini/cx3d/physics/interfaces/PhysicalNode);
%enddef

%define %PhysicalNode_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %typemap(javaimports) std::array<std::shared_ptr<cx3d::physics::PhysicalNode>, SIZE> %{
    import ini.cx3d.swig.physics.PhysicalNode;
  %}
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::physics::PhysicalNode>,
                              shared_ptr_PhysicalNode_##SIZE,
                              Object,
                              Ljava/lang/Object;, SIZE);
%enddef

%define %PhysicalNode_stdlist()
  %typemap(javaimports) cx3d::ListIteratorCpp<std::shared_ptr<cx3d::physics::PhysicalNode>> %{
    import ini.cx3d.swig.physics.PhysicalNode;
  %}
  %typemap(javaimports) std::list<std::shared_ptr<cx3d::physics::PhysicalNode>> %{
    import ini.cx3d.swig.physics.PhysicalNode;
  %}
  %stdlist_typemap(std::shared_ptr<cx3d::physics::PhysicalNode>,
                   PhysicalNode,
                   ini.cx3d.physics.interfaces.PhysicalNode);
%enddef

/**
 * apply customizations
 */
%PhysicalNode_cx3d_shared_ptr();
%PhysicalNode_java();
// for Tetrahedron:
%PhysicalNode_stdarray_array_marshalling(spatialOrganization, 4);
// for SpaceNode
%PhysicalNode_stdlist();
