/**
 * This file contains code generation customizations for class PhysicalObject.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/physical_object.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %PO_hybrid(FULL_CPP_TYPE, JAVA_PROXY_CLASS_NAME, CLASS_NAME,
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

   public java.util.Hashtable<String, ini.cx3d.physics.interfaces.IntracellularSubstance> getIntracellularSubstances(){
       java.util.Hashtable<String, ini.cx3d.physics.interfaces.IntracellularSubstance> table = new java.util.Hashtable<>();
       for(ini.cx3d.physics.interfaces.IntracellularSubstance s : getIntracellularSubstances1()){
         table.put(s.getId(), s);
       }
       return table;
   }

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

  %pragma(java) modulecode=%{
      public static boolean useNative##CLASS_NAME = USE_NATIVE;
  %}

  %typemap(jstype) FULL_CPP_TYPE "JAVA_TYPE"
%enddef

%define %PhysicalObject_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalObject,
                   ini/cx3d/physics/interfaces/PhysicalObject,
                   cx3d::physics::PhysicalObject);
%enddef

%define %PhysicalObject_hybrid(USE_NATIVE)
  %PO_hybrid(cx3d::physics::PhysicalObject,
              PhysicalObject,
              PhysicalObject,
              ini.cx3d.physics.interfaces.PhysicalObject,
              ini/cx3d/physics/interfaces/PhysicalObject,
              public NativeStringBuilder superSimStateToJson(NativeStringBuilder sb){
                return super.simStateToJson(sb);
              }
              public boolean equals(Object o) {
                  if (o != null && o instanceof PhysicalObject) {
                    return equalTo((PhysicalObject) o);
                  }
                  return false;
              },
              USE_NATIVE);
%enddef

%define %PhysicalObject_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::physics::PhysicalObject>,
                   PhysicalObject,
                   ini.cx3d.physics.interfaces.PhysicalObject);
%enddef

/**
 * apply customizations
 */
%PhysicalObject_cx3d_shared_ptr();
#ifdef PHYSICALOBJECT_NATIVE
  %PhysicalObject_hybrid(true);
#else
  %PhysicalObject_hybrid(false);
#endif
%PhysicalObject_stdlist();
%typemap(javainterfaces) cx3d::physics::PhysicalObject "ini.cx3d.physics.interfaces.PhysicalObject"
%typemap(javaimports) cx3d::physics::PhysicalObject "import ini.cx3d.swig.NativeStringBuilder;"

// class hierarchy modifications
#ifdef PHYSICALOBJECT_NATIVE
    %pragma(java) modulecode=%{
      static public abstract class PhysicalSphereBase extends ini.cx3d.swig.physics.PhysicalSphere {}
      static public abstract class PhysicalCylinderBase extends ini.cx3d.swig.physics.PhysicalCylinder {}
    %}
#else
    %pragma(java) modulecode=%{
      static public abstract class PhysicalSphereBase extends ini.cx3d.physics.PhysicalObject2 {}
      static public abstract class PhysicalCylinderBase extends ini.cx3d.physics.PhysicalObject {}
    %}
#endif
