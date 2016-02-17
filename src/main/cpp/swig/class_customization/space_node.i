/**
 * This file contains code generation customizations for class SpaceNode.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/space_node.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_array_typemap.i"
%include "std_list_typemap.i"

%define %SpaceNode_cx3d_shared_ptr()
  %cx3d_shared_ptr(SpaceNodeT_PhysicalNode,
                   ini/cx3d/spatialOrganization/interfaces/SpaceNode,
                   cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>);
%enddef

%define %SpaceNode_java()
  %java_defined_class_add(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>,
                      SpaceNodeT_PhysicalNode,
                      SpaceNode,
                      ini.cx3d.spatialOrganization.interfaces.SpaceNode,
                      ini/cx3d/spatialOrganization/interfaces/SpaceNode,
                      @Override
                      public Object[] getVerticesOfTheTetrahedronContaining(double[] position) {
                        int[] retNull = new int[1];
                        Object[] retVal = getVerticesOfTheTetrahedronContaining(position, retNull);
                        if(retNull[0] == 1){
                          return null;
                        }
                        return retVal;
                      });
  %typemap(javainterfaces) cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.SpaceNode"
%enddef

%define %SpaceNode_native()
  %native_defined_class_add(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>,
                            SpaceNodeT_PhysicalNode,
                            ini.cx3d.spatialOrganization.interfaces.SpaceNode,
                            SpaceNode,
                            public SpaceNodeT_PhysicalNode(){},
                            @Override
                            public Object[] getVerticesOfTheTetrahedronContaining(double[] position) {
                              int[] retNull = new int[1];
                              Object[] retVal = getVerticesOfTheTetrahedronContaining(position, retNull);
                              if(retNull[0] == 1){
                                return null;
                              }
                              return retVal;
                            });
%enddef

%define %SpaceNode_typemaps()
  %SpaceNode_stdlist();
  %SpaceNode_stdarray_array_marshalling(spatialOrganization, 3);
  %SpaceNode_stdarray_array_marshalling(spatialOrganization, 4);
  %int_stdarray_array_marshalling(spatialOrganization, 1);
  %SpaceNode_type_modification();


%enddef

%define %SpaceNode_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_array_marshalling(SWIG_MODULE,
                              std::shared_ptr<cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode> >,
                              shared_ptr_SpaceNode##SIZE,
                              ini.cx3d.spatialOrganization.interfaces.SpaceNode,
                              Lini/cx3d/spatialOrganization/interfaces/SpaceNode;,
                              SIZE);
%enddef

%define %SpaceNode_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>>,
                   SpaceNode,
                   ini.cx3d.spatialOrganization.interfaces.SpaceNode);
%enddef

%define %SpaceNode_type_modification()
  %typemap(javainterfaces) cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode> "ini.cx3d.spatialOrganization.interfaces.SpaceNode<ini.cx3d.physics.PhysicalNode>,
    ini.cx3d.SimStateSerializable"
  %typemap(javaimports) cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode> "import ini.cx3d.swig.NativeStringBuilder;"
  %pragma(java) jniclassimports="import ini.cx3d.swig.NativeStringBuilder;"
%enddef

 /**
  * apply customizations
  */
 %SpaceNode_cx3d_shared_ptr();
 #ifdef SPACENODE_NATIVE
   %SpaceNode_native();
 #else
   %SpaceNode_java();
 #endif
 #ifdef SPACENODE_DEBUG
   %setJavaDebugSwitch(SpaceNode, true);
 #else
   %setJavaDebugSwitch(SpaceNode, false);
 #endif
 %SpaceNode_typemaps();
