/**
 * This file contains code generation customizations for class AbstractLocalBiologyModule.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (ABSTRACTLOCALBIOLOGYMODULE_NATIVE and ABSTRACTLOCALBIOLOGYMODULE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/local_biology/abstract_local_biology_module.i"
 */

%include "util.i"
%include "std_list_typemap.i"
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

%define %AbstractLocalBiologyModule_cx3d_shared_ptr()
  %cx3d_shared_ptr(AbstractLocalBiologyModule,
                   ini/cx3d/localBiology/LocalBiologyModule,
                   cx3d::local_biology::AbstractLocalBiologyModule);
%enddef

%define %AbstractLocalBiologyModule_hybrid(USE_NATIVE)
  %PO_hybrid(cx3d::local_biology::AbstractLocalBiologyModule,
             AbstractLocalBiologyModule,
             AbstractLocalBiologyModule,
             ini.cx3d.localBiology.LocalBiologyModule,
             ini/cx3d/localBiology/LocalBiologyModule,
              public boolean equals(Object o) {
                  if (o != null && o instanceof AbstractLocalBiologyModule) {
                    return equalTo((AbstractLocalBiologyModule) o);
                  }
                  return false;
              },
              USE_NATIVE);
%enddef

%define %AbstractLocalBiologyModule_typemaps()
  %typemap(javainterfaces) cx3d::local_biology::AbstractLocalBiologyModule "ini.cx3d.localBiology.LocalBiologyModule"
  %typemap(javaimports) cx3d::local_biology::AbstractLocalBiologyModule %{
    import ini.cx3d.swig.NativeStringBuilder;
    import ini.cx3d.swig.biology.LocalBiologyModule;
    import ini.cx3d.swig.physics.CellElement;
  %}
%enddef

/**
 * apply customizations
 */
%AbstractLocalBiologyModule_cx3d_shared_ptr();
#ifdef ABSTRACTLOCALBIOLOGYMODULE_NATIVE
  %AbstractLocalBiologyModule_hybrid(true);
#else
  %AbstractLocalBiologyModule_hybrid(false);
#endif

// class hierarchy modifications
#ifdef ABSTRACTLOCALBIOLOGYMODULE_NATIVE
   %pragma(java) modulecode=%{
     static public abstract class AbstractLocalBiologyModuleBase extends ini.cx3d.swig.biology.AbstractLocalBiologyModule {}
   %}
#else
   %pragma(java) modulecode=%{
     static public abstract class AbstractLocalBiologyModuleBase extends ini.cx3d.localBiology.AbstractLocalBiologyModule {}
   %}
#endif

#ifdef ABSTRACTLOCALBIOLOGYMODULE_DEBUG
  %setJavaDebugSwitch(AbstractLocalBiologyModule, true);
#else
  %setJavaDebugSwitch(AbstractLocalBiologyModule, false);
#endif
%AbstractLocalBiologyModule_typemaps();
