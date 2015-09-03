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
 * macro definition to generate a Java interface from a CPP class that has
 * only virtual abstract methods (e.g. virtual void action() = 0;).
 * This approach enables two way communication between Java and CPP:
 * Java objects implementing the interface can be passed to a CPP object that
 * executes a Java defined function and vice versa.
 *
 * The Java interface is not generated automatically. It must be defined by the
 * developer beforehand using the Java equivalent method signatures as in the
 * CPP class
 *
 * inspired by:
 * http://stackoverflow.com/questions/8168517/generating-java-interface-with-swig
 *
 * example call:
 * GENERATE_JAVA_INTERFACE(cx3d, SimStateSerializable, cx3d::,
 *   ini.cx3d.SimStateSerializable,
 *   public StringBuilder simStateToJson(StringBuilder sb){
 *     return delegate.simStateToJson(sb);
 *   });
 *
 * @param SWIG_MODULE swig module name as defined in "%module module_name"
 * @param CPPCLASS class name of the abstract CPP class
 * @param CPPNS namespace that the cpp class is defined in
 *        e.g. cx3d::spatialOrganization::
 * @param EXISTING_JAVA_INTERFACE full name of the already defined Java interface
 *        e.g. ini.cx3d.Interface
 * @param JAVA_METHOD_DELEGATES the proxy class needs to delegate the method
 *        calls to the wrapped object. For each method in the interface declare
 *        a method that delegates the call. e.g.
 *        public void action(){
 *           return delegate.action();
 *        });
 */
%define GENERATE_JAVA_INTERFACE(SWIG_MODULE, CPPCLASS, CPPNS,
  EXISTING_JAVA_INTERFACE, JAVA_METHOD_DELEGATES)

  // enable cross language polymorphism for this cpp class
  %feature("director") CPPCLASS;

  // prepend "Internal" prefix for generated Java class
  %rename(Internal##CPPCLASS) CPPCLASS;

  // let generated Java class implement EXISTING_JAVA_INTERFACE
  %typemap(javainterfaces) CPPNS##CPPCLASS "EXISTING_JAVA_INTERFACE"

  // define type conversion from cpp to java
  %typemap(jstype) CPPNS##CPPCLASS& "EXISTING_JAVA_INTERFACE";

  // define wrapper function for Java objects that are passed to cpp code
  // preprocessor note: "Hello""World!" is equal to "Hello World!"
  %typemap(javain,pgcppname="n",
    pre= "    Internal"#CPPCLASS" n = "#SWIG_MODULE".makeInternal($javainput);")
    CPPNS##CPPCLASS&  "Internal"#CPPCLASS".getCPtr(n)"

  // declare proxy class that is used to "make a Java object Internal";
  %pragma(java) modulecode=%{

    // proxy class definition
    private static class Internal##CPPCLASS##Proxy extends Internal##CPPCLASS {
      private EXISTING_JAVA_INTERFACE delegate;
      public Internal##CPPCLASS##Proxy(EXISTING_JAVA_INTERFACE i) {
        delegate = i;
      }

      JAVA_METHOD_DELEGATES
    }

    // factory method that creates a new Internal##CPPCLASS##Proxy
    public static Internal##CPPCLASS makeInternal(EXISTING_JAVA_INTERFACE i) {
      if (i instanceof Internal##CPPCLASS) {
        // If it already *is* a Internal##CPPCLASS don't wrap it again
        return (Internal##CPPCLASS) i;
      }
      return new Internal##CPPCLASS##Proxy(i);
    }
  %}
%enddef
