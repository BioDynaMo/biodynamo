/*
 Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
 Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
 Sabina Pfister, Adrian M. Whatley & Lukas Breitwieser.

 This file is part of CX3D.

 CX3D is free software: you can redistribute it and/or modify
 it under the terms of the GNU General virtual License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 CX3D is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General virtual License for more details.

 You should have received a copy of the GNU General virtual License
 along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * This file enables transparent type conversions between std::array and Java
 * arrays / lists. For each distinct template type one Java class is generated that
 * extends from java.util.AbstractList
 * Marhalling to Java Arrays has been implemented. This was needed due to the
 * existing cx3d codebase. This is not the most efficient solution as it involves
 * copying of array elements. As alternative, marshalling to java.util.List<E>
 * and directly accessing elements would solve this issues (but has not been
 * implemented yet).
 */
%{
#include <array>
#include <memory>
%}

/**
 * CPP declaration of std::array
 * Contains only the most basic methods, but does not cover the whole functionality.
 * Add further declarations on demand.
 */
namespace std {
  template<class T, class N> class array {
    public:
      typedef size_t size_type;
      typedef T value_type;
      typedef const value_type& const_reference;
      array();
      %rename(size_impl) size;
      size_type size() const;
      %rename(isEmpty) empty;
      bool empty() const;
      %extend {
          const_reference get_impl(int i) throw (std::out_of_range) {
              return self->at(i);
          }
          void set_impl(int i, const value_type& val) throw (std::out_of_range) {
              self->at(i) = val;
          }
      }
  };
}

/**
 * Macro definition to create a Java class for the given template types
 * The generated class extends from java.util.AbstractList and implements
 * java.util.RandomAccess.
 *
 * @param CPP_TYPE defines the data type for the std::array typemap
 * @param TEMPLATE_SUFFIX determines the name of the generated java class.
 *        It is appended to ArrayT_
 *        Naming recommendation: e.g. JAVA_TYPE=Double, SIZE=3 -> Double_3_
 *        Manual name specification is necessary due to naming conflicts:
 *        e.g. std::array<Rational, 3> and std::array<std::shared_ptr<Rational, 3>
 *        would map to the same name (ArrayT_Double_3) using built-in name generation.
 * @param JAVA_TYPE Java equivalent of CPP_TYPE, must be a non primitive type
 *        For usage with primitive types use their Object equivalent: double -> Double
 * @param SIZE is the size type for std::array. Every different size is a different type
 *
 * example call:
 * %stdarray_typemap(std::shared_ptr<cx3d::spatial_organization::Rational>,
 *                   shared_ptr_Rational_3,
 *                   ini.cx3d.spatialOrganization.interfaces.Rational,
 *                   3);
 */
%define %stdarray_typemap(CPP_TYPE, TEMPLATE_SUFFIX, JAVA_TYPE, SIZE)
  %typemap(javabase) std::array<CPP_TYPE, SIZE>, std::array<CPP_TYPE, SIZE>& "java.util.AbstractList<"#JAVA_TYPE">";
  %typemap(javainterfaces) std::array<CPP_TYPE, SIZE>, std::array<CPP_TYPE, SIZE>& "java.util.RandomAccess";
  %typemap(javacode) std::array<CPP_TYPE, SIZE>, std::array<CPP_TYPE, SIZE>& %{
    public JAVA_TYPE get(int idx) {
      return get_impl(idx);
    }

    public int size() {
      return (int)size_impl();
    }

    public JAVA_TYPE set(int idx, JAVA_TYPE d) {
      JAVA_TYPE old = get_impl(idx);
      set_impl(idx, d);
      return old;
    }
  %}

  %template(ArrayT_##TEMPLATE_SUFFIX) std::array<CPP_TYPE, SIZE>;
%enddef

/**
 * Macro definition used to generate Java array marshalling to std::array.
 * Despite the macro name, it can also enable it for non primitive types. There
 * is simpler macro definition for non primitive types though (stdarray_array_marshalling).
 * It forwards the call to this implementation.
 * Automatically generates a Java object for the given types. A separate call
 * to %stdarray_typemap is not needed.
 * If Java code calls with a wrong array length, an IllegalArgumentException will be thrown.
 *
 * @param CPP_TYPE defines the data type for the std::array typemap
 * @param TEMPLATE_SUFFIX determines the name of the generated Java class.
 *        It is appended to ArrayT_
 *        Naming recommendation: e.g. JAVA_TYPE=Double, SIZE=3 -> Double_3_
 *        Manual name specification is necessary due to naming conflicts:
 *        e.g. std::array<Rational, 3> and std::array<std::shared_ptr<Rational, 3>
 *        would map to the same name (ArrayT_Double_3) using automatic name generation.
 * @param JAVA_TYPE Java equivalent of CPP_TYPE, must be a non primitive type
 *        For usage with primitive types use their Object equivalent: double -> Double
 * @param JAVA_ARR_TYPE needed for primitive types where JAVA_ARR_TYPE and JAVA_TYPE differs
 *        e.g. double: JAVA_TYPE=Double, JAVA_ARR_TYPE=double
 * @param SIZE is the size type for std::array. Every different size is a different type
 *
 * example call for Java double[] <-> C++ std::array<double, 3> conversion:
 * %stdarray_primitive_array_marshalling(double, Double_3, Double, double, 3);
 */
%define %stdarray_primitive_array_marshalling(CPP_TYPE, TEMPLATE_SUFFIX, JAVA_TYPE, JAVA_ARR_TYPE, SIZE)
  %stdarray_typemap(CPP_TYPE, TEMPLATE_SUFFIX, JAVA_TYPE, SIZE);

  %pragma(java) modulecode=%{
    static ArrayT_##TEMPLATE_SUFFIX wrapArrayInArrayT_##TEMPLATE_SUFFIX(JAVA_ARR_TYPE[] arg){
      ArrayT_##TEMPLATE_SUFFIX array =  new ArrayT_##TEMPLATE_SUFFIX();
      if(arg.length != SIZE) {
          throw new IllegalArgumentException("This function call only supports arrays with length SIZE!");
      }
      for(int i = 0; i < SIZE; i++) {
          array.set(i, arg[i]);
      }
      return array;
    }

    static JAVA_ARR_TYPE[] unwrapArrayInArrayT_##TEMPLATE_SUFFIX(long cPtr, boolean cMemoryOwn) {
      ArrayT_##TEMPLATE_SUFFIX array = new ArrayT_##TEMPLATE_SUFFIX(cPtr, cMemoryOwn);
      JAVA_ARR_TYPE[] arr = new JAVA_ARR_TYPE[array.size()];
      for(int i = 0; i < array.size(); i++) {
        arr[i] = array.get(i);
      }
      return arr;
    }
  %}

  %typemap(jstype) std::array<CPP_TYPE, SIZE>, std::array<CPP_TYPE, SIZE>& #JAVA_ARR_TYPE"[]"
  %typemap(javaout) std::array<CPP_TYPE, SIZE>, std::array<CPP_TYPE, SIZE>& "{ return $module.unwrapArrayInArrayT_"#TEMPLATE_SUFFIX"($jnicall, $owner);}"

  %typemap(javain,
          pre = "    ArrayT_"#TEMPLATE_SUFFIX" temp$javainput = $module.wrapArrayInArrayT_"#TEMPLATE_SUFFIX"($javainput);",
          pgcppname="temp$javainput") const std::array<CPP_TYPE, SIZE>, const std::array<CPP_TYPE, SIZE>&, std::array<CPP_TYPE, SIZE> "$javaclassname.getCPtr(temp$javainput)"

  // javain for non-const function parameters; could have been modified on CPP side -> synchronise
  %typemap(javain,
           pre = "    ArrayT_"#TEMPLATE_SUFFIX" temp$javainput = $module.wrapArrayInArrayT_"#TEMPLATE_SUFFIX"($javainput);",
           post = "        for (int i= 0; i < $javainput.length; i++) {"
                  "            $javainput[i] = temp$javainput.get(i);"
                  "        }",
           pgcppname="temp$javainput") std::array<CPP_TYPE, SIZE>& "$javaclassname.getCPtr(temp$javainput)"
%enddef

/**
 * This macro definition is used to enable Java array marshalling for non
 * primitive data types. For primite types and parameter documentation see macro:
 * %stdarray_primitive_array_marshalling
 * Automatically generates a Java object for the given types. A separate call
 * to %stdarray_typemap is not needed.
 *
 * example call for Java ini.cx3d.spatialOrganization.interfaces.Rational[] <->
 *    C++ std::array<std::shared_ptr<cx3d::spatial_organization::Rational>, 3> conversion:
 * %stdarray_array_marshalling(std::shared_ptr<cx3d::spatial_organization::Rational>,
 *                             shared_ptr_Rational_3,
 *                             ini.cx3d.spatialOrganization.interfaces.Rational,
 *                             3);
 */
%define %stdarray_array_marshalling(CPP_TYPE, TEMPLATE_SUFFIX, JAVA_TYPE, SIZE)
  %stdarray_primitive_array_marshalling(CPP_TYPE,
                              TEMPLATE_SUFFIX,
                              JAVA_TYPE,
                              JAVA_TYPE,
                              SIZE);
%enddef
