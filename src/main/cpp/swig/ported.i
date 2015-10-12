/**
 * This file contains macros modifying code generation for already ported classes
 */

/**
 * If a class has been ported, an interface is created manually on the Java side.
 * In the remaining Java application this interface type will be used. The replaced
 * Java class also implements this interface. This allows a quick switch between
 * native and Java implementation (e.g. for debugging purposes)
 * This macro changes the type of function arguments and return types to this
 * interface. Therefore the generated Java proxy class has to implement this
 * interface as well.
 *
 * @param FULL_CPP_TYPE specifies the C++ type including namespace and optional
 *        template type
 * @param FULL_JAVA_TYPE name of the manually created Java interface (including
 *        package and generic type)
 *
 * usage example:
 * %ported_type_modification(cx3d::spatial_organization::Triangle3D<cx3d::PhysicalNode>,
 *                           ini.cx3d.spatialOrganization.interfaces.Triangle3D<ini.cx3d.physics.PhysicalNode>);
 */
%define %ported_type_modification(FULL_CPP_TYPE, FULL_JAVA_TYPE)
  %typemap(javainterfaces) FULL_CPP_TYPE "FULL_JAVA_TYPE"
  %typemap(jstype) FULL_CPP_TYPE "FULL_JAVA_TYPE"
%enddef

/**
 * This macro adds an equals method to the generated Java proxy class.
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
%define %ported_add_equals(FULL_CPP_TYPE, JAVA_CLASS_NAME)
    %typemap(javacode) FULL_CPP_TYPE %{
      public boolean equals(Object o) {
        if (o != null && o instanceof JAVA_CLASS_NAME) {
          return equalTo((JAVA_CLASS_NAME) o);
        }
        return false;
      }
    %}
%enddef
