/**
 * This file contains a collection of macros to modify code generation to
 * facilitate an iterative porting approach
 */

/**
 * Macro that turns on native implementation for the specified class.
 * Must be called at the top after %module and before %{ #includes ..%}
 * Current limitation: if objects of class X are created on the C++ side and
 * the java version of class X should be used (by omitting: %native(X)) all
 * classes that construct class X must also be declared non native.
 *
 * @param CLASS_NAME_CAPS capitalized class name
 *
 * usage example:
 * %native(TETRAHEDRON);
 */
%define %native(CLASS_NAME_CAPS)
 %{
 #define CLASS_NAME_CAPS##_NATIVE
 %}
 #define CLASS_NAME_CAPS##_NATIVE
%enddef

/**
 * Macro that turns on debuggin output for the specified class.
 * Works for native and Java implementation
 * Must be called at the top after %module and before %{ #includes ..%}
 *
 * @param CLASS_NAME_CAPS capitalized class name
 *
 * usage example:
 * %debug(TETRAHEDRON);
 */
%define %debug(CLASS_NAME_CAPS)
 %{
 #define CLASS_NAME_CAPS##_DEBUG
 %}
 #define CLASS_NAME_CAPS##_DEBUG
%enddef

/**
 * This macro modifies code generation to allow usage of the Java defined
 * implementation by enabling cross language polymorphism and the possibility
 * to return the java object from the native implementation.
 * Does not specify an interface which the generated Java Proxy class should
 * implement. If this is needed it has to be done seperately using:
 * %typemap(javainterfaces) FULL_CPP_TYPE "JAVA_INTERFACE_NAME"
 *
 * @param FULL_CPP_TYPE including namespace, class name and optional template type
 * @param JAVA_PROXY_CLASS_NAME name of the generated proxy class
 *        without templates it is equal to the class name, with it depends
 *        on the id specified in %template(...) FULL_CPP_TYPE
 * @param FULL_JAVA_INTERFACE_NAME includes package specifier and interface name
 * @param CLASS_NAME name of the Java defined class
 * @param JAVA_TYPE identifier of the java type including the package id
 *        could be the type of the Java Defined Class itself, or the common
 *        interface if it has already been introduced.
 * @param JNI_TYPE JNI specifier for the type specified in JAVA_TYPE
 *        @see: http://journals.ecs.soton.ac.uk/java/tutorial/native1.1/implementing/method.html
 *
 * usage example:
 * %java_defined_class(cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>,
 *             TetrahedronT_PhysicalNode,
 *             Tetrahedron,
 *             ini.cx3d.spatialOrganization.interfaces.Tetrahedron,
 *             ini/cx3d/spatialOrganization/interfaces/Tetrahedron);
 */
%define %java_defined_class(FULL_CPP_TYPE, JAVA_PROXY_CLASS_NAME, CLASS_NAME,
                            JAVA_TYPE, JNI_TYPE)
  // enable cross language polymorphism for this C++ type
  %feature("director", assumeoverride=1) FULL_CPP_TYPE;

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
      public static boolean useNative##CLASS_NAME = false;
  %}

  %typemap(jstype) FULL_CPP_TYPE "JAVA_TYPE"
%enddef

/**
 * This macro modifies code generation to allow usage of the native implementation
 * It modifies types adds an equals method to the generated Java Proxy class
 * and adds a boolean variable that tells the Java code to use the native implementation
 * Furthermore, it adds an empty method definition for registerJavaObject. This is necessary
 * to avoid code changes when switching between Java and Native implementation.
 * It also adds an empty default constructor if none is specified in the C++ class.
 * This is also to avoid code changes.
 * If shared pointers are used, this macro must be called after cx3d_shared_ptr!
 * Precondition for equals method: C++ class has a declared function with the following signature:
 * <code>bool equalTo(const CPP_TYPE& other);</code>
 *
 * @param FULL_CPP_TYPE including namespace, class name and optional template type
 * @param JAVA_PROXY_CLASS_NAME name of the generated proxy class
 *        without templates it is equal to the class name, with it depends
 *        on the id specified in %template(...) FULL_CPP_TYPE
 * @param FULL_JAVA_INTERFACE_NAME includes package specifier and interface name
 * @param DEFAULT_CONSTRUCOR if C++ class does not specify a public default
 *        constructor: public JAVA_PROXY_CLASS_NAME(){}
 *        empty otherwise
 *
 * usage example
 * if Tetrahedron specifies a public default constructor:
 * %native_defined_class(cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>,
 *                       TetrahedronT_PhysicalNode,
 *                       ini.cx3d.spatialOrganization.interfaces.Tetrahedron,
 *                       Tetrahedron, );
 *
 * if it does not:
 * %native_defined_class(cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>,
 *                       TetrahedronT_PhysicalNode,
 *                       ini.cx3d.spatialOrganization.interfaces.Tetrahedron,
 *                       Tetrahedron,
 *                       public TetrahedronT_PhysicalNode(){});
 */
%define %native_defined_class(FULL_CPP_TYPE, JAVA_PROXY_CLASS_NAME,
                              FULL_JAVA_INTERFACE_NAME, CLASS_NAME,
                              DEFAULT_CONSTRUCOR)

  %typemap(javainterfaces) FULL_CPP_TYPE "FULL_JAVA_INTERFACE_NAME"
  %typemap(jstype) FULL_CPP_TYPE "FULL_JAVA_INTERFACE_NAME"

  %typemap(javacode) FULL_CPP_TYPE %{

    DEFAULT_CONSTRUCOR

    public static synchronized void registerJavaObject(JAVA_PROXY_CLASS_NAME o) {
    }

    public boolean equals(Object o) {
      if (o != null && o instanceof JAVA_PROXY_CLASS_NAME) {
        return equalTo((JAVA_PROXY_CLASS_NAME) o);
      }
      return false;
    }
  %}

  %pragma(java) modulecode=%{
      public static boolean useNative##CLASS_NAME = true;
  %}
%enddef

/**
 * This macro inserts a static variable into the module class that
 * specifies if debuggin output should be generated on the Java side.
 * Intended usage is in combination with the %debug macro.
 *
 * @param REPLACED_JAVA_CLASS_NAME Class name of he replaces Java class
 * @param VALUE true or false
 *
 * usage example:
 * #ifdef TETRAHEDRON_DEBUG
 *   %setJavaDebugSwitch(Tetrahedron, true);
 * #else
 *   %setJavaDebugSwitch(Tetrahedron, false);
 * #endif
 */
%define %setJavaDebugSwitch(REPLACED_JAVA_CLASS_NAME, VALUE)
    %pragma(java) modulecode=%{
        public static boolean debug##REPLACED_JAVA_CLASS_NAME = VALUE;
    %}
%enddef

/**
 * This macro adds an equals method to the generated Java Proxy Class.
 * precondition: C++ class has a declared function with the following signature:
 * <code>bool equalTo(const CPP_TYPE& other);</code>
 *
 * @param FULL_CPP_TYPE specifies the C++ type including namespace and optional
 *        template type
 * @param JAVA_CLASS_NAME name of the generated Java proxy class (without
 *        package name and generic modifier)
 *
 * usage example:
 * %ported_add_equals(cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>,
 *                    Triangle3DT_PhysicalNode);
 */
%define %add_equals(FULL_CPP_TYPE, JAVA_CLASS_NAME)
    %typemap(javacode) FULL_CPP_TYPE %{
      public boolean equals(Object o) {
        if (o != null && o instanceof JAVA_CLASS_NAME) {
          return equalTo((JAVA_CLASS_NAME) o);
        }
        return false;
      }
    %}
%enddef
