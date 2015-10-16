/**
 * This file contains macros modifying code generation for classes that are used
 * in native code, but whose implementation is still in Java
 * =Java Defined Class (jdc)
 */

/**
 * This macro forms the basis for Java defined classes by enabling cross
 * language polymorphism and the possibility to return the java object from
 * the native implementation
 * There is a different macro for templates
 *
 * @param CPP_NS namespace of the C++ type
 * @param CPP_TYPE name of the C++ class
 *
 * usage example:
 * %jdc_enable(cx3d::spatial_organization, CppClassName)
 */
%define %jdc_enable(CPP_NS, CPP_TYPE)
  // %java_implementation_templated(CPP_NS, CPP_TYPE, ##EMPTY_PARAM_HACK, CPP_TYPE);
  // enable cross language polymorphism for this cpp type
  %feature("director", assumeoverride=1) CPP_NS::CPP_TYPE;

  // append "CppType" suffix to the generated Java proxy class
  %rename(CPP_TYPE## CppType) CPP_NS::CPP_TYPE;

  %extend CPP_NS::CPP_TYPE {
    virtual jobject swigDirectorReturnJavaObject(JNIEnv *jenv){
       if(self == nullptr){
         return 0;
       }
       SwigDirector_##CPP_TYPE##CppType *director = dynamic_cast<SwigDirector_##CPP_TYPE##CppType *>(self);
       if (director) {
          return director->swig_get_self(jenv);
       }
       SWIG_JavaThrowException(jenv, SWIG_JavaIllegalArgumentException, "dynamic cast to SwigDirector_"#CPP_TYPE"CppType failed!");
       return 0;
    }
  }
%enddef

/**
 * Templated version of %jdc_enable(..) that takes two more arguments
 *
 * @param CPP_NS namespace of the C++ type
 * @param CPP_TYPE name of the C++ class
 * @param TEMPLATE_TYPE type of the template including brackets
 * @param JAVA_PROXY_CLASS_NAME name of the generated Java proxy class
 *        defined using the %template macro
 *
 * usage example:
 * %jdc_enable_templated(cx3d::spatial_organization, Tetrahedron,
 *                       <cx3d::PhysicalNode>, TetrahedronT_PhysicalNode);
 */
%define %jdc_enable_templated(CPP_NS, CPP_TYPE, TEMPLATE_TYPE, JAVA_PROXY_CLASS_NAME)

  // enable cross language polymorphism for this cpp type
  %feature("director", assumeoverride=1) CPP_NS::CPP_TYPE##TEMPLATE_TYPE;

  // append "CppType" suffix to the generated Java proxy class
  %rename(JAVA_PROXY_CLASS_NAME## CppType) CPP_NS::CPP_TYPE##TEMPLATE_TYPE;

  %extend CPP_NS::CPP_TYPE##TEMPLATE_TYPE {
    virtual jobject swigDirectorReturnJavaObject(JNIEnv *jenv){
       if(self == nullptr){
         return 0;
       }
       SwigDirector_##JAVA_PROXY_CLASS_NAME##CppType *director = dynamic_cast<SwigDirector_##JAVA_PROXY_CLASS_NAME##CppType *>(self);
       if (director) {
          return director->swig_get_self(jenv);
       }
       SWIG_JavaThrowException(jenv, SWIG_JavaIllegalArgumentException, "dynamic cast to SwigDirector_"#JAVA_PROXY_CLASS_NAME"CppType failed!");
       return 0;
    }
  }
%enddef

/**
 * This macro makes it possible that a call to APortedClass.getJdcObject
 * returns the jdc object instead of the C++ representation.
 *
 * @param FULL_CPP_TYPE namespace::CppClassName<full template type>
 * @param METHOD_NAME only name of the method - e.g. getId
 * @param JAVA_TYPE the type of the object that will be returned
 * @param SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX in jdc_enable the class gets extended
 *        by another function swigDirectorReturnJavaObject. The name in the generated
 *        C function has a prefix depending on the C++ class
 *        e.g. if C++ class is: cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>
 *        then prefix: cx3d_spatial_organization_SpaceNode_Sl_cx3d_PhysicalNode_Sg__
 *        if in doubt have a look at the generated cxx module file and search for
 *        swigDirectorReturnJavaObject
 *
 * usage example:
 * %jdc_get(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>, getId,
 *          ini.cx3d.spatialOrganization.SpaceNode<ini.cx3d.physics.PhysicalNode>,
 *          cx3d_spatial_organization_SpaceNode_Sl_cx3d_PhysicalNode_Sg__);
 */
%define %jdc_get(FULL_CPP_TYPE, METHOD_NAME, JAVA_TYPE, SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX)
  %typemap (jni)    std::shared_ptr<FULL_CPP_TYPE > METHOD_NAME "jobject"
  %typemap (jtype)  std::shared_ptr<FULL_CPP_TYPE > METHOD_NAME "JAVA_TYPE"
  %typemap (jstype) std::shared_ptr<FULL_CPP_TYPE > METHOD_NAME "JAVA_TYPE"
  %typemap (javaout) std::shared_ptr<FULL_CPP_TYPE > METHOD_NAME {
    return $jnicall;
  }
  %typemap (out) std::shared_ptr<FULL_CPP_TYPE > METHOD_NAME {
    jresult = SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX##swigDirectorReturnJavaObject(result.get(), jenv);
  }
%enddef

/**
 * Extends the functionality of std::arrays to return the Java object instead of
 * the C++ representation. This macro must be called before any stdarray_array_marshalling
 * Caution: This implementation is not thread safe! No other thread should be
 * to enter array.get between array.setReturnJavaObj to true/false
 *
 * @param FULL_CPP_TYPE namespace::CppClassName<full template type>
 * @param METHOD_NAME only name of the method - e.g. getId
 * @param JAVA_TYPE the type of the object that will be returned
 * @param TEMPLATE_SUFFIX determines the name of the used Array Java class.
 *        It is appended to ArrayT_
 *        Use same as for stdarray_array_marshalling - have a look at this macro
 *        for a more detailed explaination
 * @param SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX in jdc_enable the class gets extended
 *        by another function swigDirectorReturnJavaObject. The name in the generated
 *        C function has a prefix depending on the C++ class
 *        e.g. if C++ class is: cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>
 *        then prefix: cx3d_spatial_organization_SpaceNode_Sl_cx3d_PhysicalNode_Sg__
 *        if in doubt have a look at the generated cxx module file and search for
 *        swigDirectorReturnJavaObject
 *
 * usage example:
 * %jdc_array_extension_templated(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>,
 *                                getJava_impl,
 *                                ini.cx3d.spatialOrganization.SpaceNode,
 *                                shared_ptr_SpaceNodeT_PhysicalNode_3,
 *                                cx3d_spatial_organization_SpaceNode_Sl_cx3d_PhysicalNode_Sg__);
 */
%define %jdc_array_extension(FULL_CPP_TYPE, METHOD_NAME, JAVA_TYPE, TEMPLATE_SUFFIX, SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX)
  %jdc_get(FULL_CPP_TYPE, METHOD_NAME, JAVA_TYPE, SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX);

  %pragma(java) modulecode=%{
    static JAVA_TYPE[] unwrapJavaArrayInArrayT_##TEMPLATE_SUFFIX(long cPtr, boolean cMemoryOwn) {
      ArrayT_##TEMPLATE_SUFFIX array = new ArrayT_##TEMPLATE_SUFFIX(cPtr, cMemoryOwn);
      try {
        array.setReturnJavaObj(true);
        JAVA_TYPE[] arr = new JAVA_TYPE[array.size()];
        for(int i = 0; i < array.size(); i++) {
          arr[i] = (JAVA_TYPE) array.get(i);
        }
        return arr;
      } catch (Exception e) {
        throw e;
      }finally {
        array.setReturnJavaObj(false);
      }
    }
  %}

  // remove body of function std::array<...>::get_impl
  // it will not be used for java defined classes and the default implementation
  // will output the wrong type leading to a Java compilation error
  %typemap(javaout) const FULL_CPP_TYPE& get_impl {
      throw new UnsupportedOperationException("This method must be implemented in a subclass");
  }
%enddef

/**
 * Equivalent of %jdc_array_extension for templates.
 *
 * @param FULL_CPP_TYPE namespace::CppClassName<full template type>
 * @param METHOD_NAME only name of the method - e.g. getId
 * @param JAVA_TYPE the type of the object that will be returned
 * @param JAVA_TYPE_WO_GENERICS java type without generic specification
 * @param JAVA_GENERIC_TYPE
 * @param TEMPLATE_SUFFIX determines the name of the used Array Java class.
 *        It is appended to ArrayT_
 *        Use same as for stdarray_array_marshalling - have a look at this macro
 *        for a more detailed explaination
 * @param SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX in jdc_enable the class gets extended
 *        by another function swigDirectorReturnJavaObject. The name in the generated
 *        C function has a prefix depending on the C++ class
 *        e.g. if C++ class is: cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>
 *        then prefix: cx3d_spatial_organization_SpaceNode_Sl_cx3d_PhysicalNode_Sg__
 *        if in doubt have a look at the generated cxx module file and search for
 *        swigDirectorReturnJavaObject
 *
 * usage example:
 * %jdc_array_extension_templated(cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>,
 *                                getJava_impl,
 *                                ini.cx3d.spatialOrganization.SpaceNode,
 *                                ini.cx3d.physics.PhysicalNode,
 *                                shared_ptr_SpaceNodeT_PhysicalNode_3,
 *                                cx3d_spatial_organization_SpaceNode_Sl_cx3d_PhysicalNode_Sg__);
 */
%define %jdc_array_extension_templated(FULL_CPP_TYPE, METHOD_NAME, JAVA_TYPE_WO_GENERICS, JAVA_GENERIC_TYPE, TEMPLATE_SUFFIX, SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX)
  %jdc_get(FULL_CPP_TYPE, METHOD_NAME, JAVA_TYPE_WO_GENERICS, SWIG_DIRECTOR_RETURN_JAVA_OBJ_PREFIX)

  %pragma(java) modulecode=%{
    static JAVA_TYPE_WO_GENERICS<JAVA_GENERIC_TYPE>[] unwrapJavaArrayInArrayT_##TEMPLATE_SUFFIX(long cPtr, boolean cMemoryOwn) {
      ArrayT_##TEMPLATE_SUFFIX array = new ArrayT_##TEMPLATE_SUFFIX(cPtr, cMemoryOwn);
      try {
        array.setReturnJavaObj(true);
        JAVA_TYPE_WO_GENERICS<JAVA_GENERIC_TYPE>[] arr = new JAVA_TYPE_WO_GENERICS[array.size()];
        for(int i = 0; i < array.size(); i++) {
          arr[i] = (JAVA_TYPE_WO_GENERICS<JAVA_GENERIC_TYPE>) array.get(i);
        }
        return arr;
      } catch (Exception e) {
        throw e;
      }finally {
        array.setReturnJavaObj(false);
      }
    }
  %}

  // remove body of function std::array<...>::get_impl
  // it will not be used for java defined classes and the default implementation
  // will output the wrong type leading to a Java compilation error
  %typemap(javaout) const FULL_CPP_TYPE& get_impl {
      throw new UnsupportedOperationException("This method must be implemented in a subclass");
  }
%enddef

/**
 * Equivalent of %jdc_get for array types
 *
 * @param FULL_CPP_TYPE namespace::CppClassName<full template type>
 * @param SIZE size of the std::array
 * @param METHOD_NAME only name of the method - e.g. getId
 * @param JAVA_TYPE the type of the object that will be returned
 * @param TEMPLATE_SUFFIX determines the name of the used Array Java class.
 *        It is appended to ArrayT_
 *        Use same as for stdarray_array_marshalling - have a look at this macro
 *        for a more detailed explaination
 *
 * usage example:
 * %jdc_get_array(std::shared_ptr<cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>>,
 *                3, getNodes,
 *                ini.cx3d.spatialOrganization.SpaceNode,
 *                shared_ptr_SpaceNodeT_PhysicalNode_3);
 */
%define %jdc_get_array(FULL_CPP_TYPE, SIZE, METHOD_NAME, JAVA_TYPE, TEMPLATE_SUFFIX)
  %typemap(jstype) std::array<FULL_CPP_TYPE, SIZE> METHOD_NAME #JAVA_TYPE"[]"
  %typemap(javaout) std::array<FULL_CPP_TYPE, SIZE> METHOD_NAME "{ return $module.unwrapJavaArrayInArrayT_"#TEMPLATE_SUFFIX"($jnicall, $owner);}"
%enddef

/**
 * Removes the method body for a given method
 * The generated methods inside the Java Proxy Class will never be called for
 * jdc as they are implemented in Java.
 * Sometimes conflicting types would lead to Java compile errors.
 * Therefore it is best to remove the implementation.
 *
 * @param METHOD_SIGNATURE complete method signature return_type namespace::Classname::methodname
 *
 * usage example:
 * %jdc_remove_method_body(std::shared_ptr<cx3d::spatial_organization::SpaceNode<cx3d::PhysicalNode>> cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>::getOppositeNode);
 */
%define %jdc_remove_method_body(METHOD_SIGNATURE)
  %typemap(javaout) METHOD_SIGNATURE {
      throw new UnsupportedOperationException("This method must be implemented in a subclass");
  }
%enddef

//should be called as last modification for a given type to avoid overriding this rule
/**
 * Change the type from the Java Proxy Class to the Java Class that holds the
 * implementation (=jdc)
 *
 * @param FULL_CPP_TYPE namespace::CppClassName<full template type>
 * @param JAVA_RAW_TYPE java type including package but without generics
 *
 * usage example:
 * %jdc_type_modification(cx3d::spatial_organization::Tetrahedron<cx3d::PhysicalNode>, ini.cx3d.spatialOrganization.Tetrahedron);
 */
%define %jdc_type_modification(FULL_CPP_TYPE, JAVA_RAW_TYPE)
  %typemap(jstype) FULL_CPP_TYPE "JAVA_RAW_TYPE"
%enddef
