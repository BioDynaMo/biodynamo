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
 * %include "class_customization/physics/physical_node.i"
 */

%include "util.i"
%include "primitives.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"
%include "std_array_typemap.i"

%define %PhysicalNode_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalNode,
                   ini/cx3d/physics/interfaces/PhysicalNode,
                   cx3d::physics::PhysicalNode);
%enddef

%define %hybrid_class(FULL_CPP_TYPE, JAVA_PROXY_CLASS_NAME, CLASS_NAME,
                            JAVA_TYPE, JNI_TYPE, ADDITIONAL_CODE, USE_NATIVE)
  // enable cross language polymorphism for this C++ type
  %feature("director") FULL_CPP_TYPE;

  %extend FULL_CPP_TYPE {
   virtual jlong getObjectPtr(const std::shared_ptr<FULL_CPP_TYPE >& shared_ptr){
     jlong result = 0;
     *(FULL_CPP_TYPE **)&result = shared_ptr.get();
     return result;
   }
  }

  %typemap(javacode) FULL_CPP_TYPE %{
   private static java.util.Map<Long, JAVA_TYPE> javaObjectMap = new java.util.HashMap<Long, JAVA_TYPE>();

   /**
    * This method must be called in each constructor of the Java Defined Class.
    * It registers the newly created Java object. Thus it is possible to retrieve
    * it if it is returned from the C++ side, or needed as a parameter in a Director
    * method.
    */
   public static synchronized void registerJavaObject(JAVA_TYPE o) {
     long objPtr = ((JAVA_PROXY_CLASS_NAME) o).getObjectPtr(o);
     javaObjectMap.put(objPtr, o);
   }

   /**
    * Returns the Java object given the C++ object pointer
    */
   public static synchronized JAVA_TYPE getJavaObject(long objPtr) {
     if (objPtr == 0){
       return null;
     }
     else {
       return javaObjectMap.get(objPtr);
     }
   }

   ADDITIONAL_CODE
  %}

  %typemap(javaout) std::shared_ptr< FULL_CPP_TYPE >,
                    std::shared_ptr< FULL_CPP_TYPE > &,
                    std::shared_ptr< FULL_CPP_TYPE > *,
                    std::shared_ptr< FULL_CPP_TYPE > *& {
   long objPtr = $jnicall;
   return JAVA_PROXY_CLASS_NAME.getJavaObject(objPtr);
  }

  %typemap(out) std::shared_ptr< FULL_CPP_TYPE > %{
    *(FULL_CPP_TYPE **)&$result = $1.get();
  %}

  %typemap(out) const std::shared_ptr< FULL_CPP_TYPE >& %{
    *(FULL_CPP_TYPE **)&$result = $1->get();
  %}

  %typemap(javadirectorin) std::shared_ptr< FULL_CPP_TYPE >,
                         std::shared_ptr< FULL_CPP_TYPE > &,
                         std::shared_ptr< FULL_CPP_TYPE > *,
                         std::shared_ptr< FULL_CPP_TYPE > *& #JAVA_PROXY_CLASS_NAME".getJavaObject($jniinput)"

  %typemap(directorin, descriptor="L"#JNI_TYPE";") std::shared_ptr< FULL_CPP_TYPE >,
                                                   const std::shared_ptr< FULL_CPP_TYPE >& %{
    *(FULL_CPP_TYPE **)&j$1 = $1.get();
  %}

  // restore defaults for static create methods which must not be affected
  %typemap(javaout) std::shared_ptr< FULL_CPP_TYPE > create {
   long cPtr = $jnicall;
   return (cPtr == 0) ? null : JAVA_PROXY_CLASS_NAME.swigCreate(cPtr, true);
  }

  %typemap(out) std::shared_ptr< FULL_CPP_TYPE > create %{
    *(std::shared_ptr< FULL_CPP_TYPE > **)&$result = $1 ? new std::shared_ptr< FULL_CPP_TYPE >($1) : 0;
  %}

  %typemap(directorout) std::shared_ptr< FULL_CPP_TYPE > create %{
    *(std::shared_ptr< FULL_CPP_TYPE > **)&$result = $1 ? new std::shared_ptr< FULL_CPP_TYPE >($1) : 0;
  %}

  %pragma(java) modulecode=%{
      public static boolean useNative##CLASS_NAME = USE_NATIVE;
  %}

  %typemap(jstype) FULL_CPP_TYPE "JAVA_TYPE"
%enddef

%define %PhysicalNode_hybrid(USE_NATIVE)
  %hybrid_class(cx3d::physics::PhysicalNode,
                PhysicalNode,
                PhysicalNode,
                ini.cx3d.physics.interfaces.PhysicalNode,
                ini/cx3d/physics/interfaces/PhysicalNode,
                public boolean equals(Object o) {
                    if (o != null && o instanceof PhysicalNode) {
                      return equalTo((PhysicalNode) o);
                    }
                    return false;
                },
                USE_NATIVE);
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
#ifdef PHYSICALNODE_NATIVE
  %PhysicalNode_hybrid(true);
#else
  %PhysicalNode_hybrid(false);
#endif
#ifdef PHYSICALNODE_DEBUG
  %setJavaDebugSwitch(PhysicalNode, true);
#else
  %setJavaDebugSwitch(PhysicalNode, false);
#endif

// set static variable ecm on startup
%pragma(java) jniclasscode=%{
    static {
        ini.cx3d.swig.physics.PhysicalNode.setECM(ini.cx3d.simulations.ECM.getInstance());
    }
%}

// for Tetrahedron:
%PhysicalNode_stdarray_array_marshalling(spatialOrganization, 4);
// for SpaceNode
%PhysicalNode_stdlist();
%typemap(javainterfaces) cx3d::physics::PhysicalNode "ini.cx3d.physics.interfaces.PhysicalNode"
%typemap(javaimports) cx3d::physics::PhysicalNode "import ini.cx3d.swig.NativeStringBuilder; import ini.cx3d.swig.spatialOrganization.SpaceNodeT_PhysicalNode;"
