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

%module(directors="1") cx3d

%{
#include "string_builder.h"
// #include "sim_state_serializable.h"
using namespace cx3d;
%}

// transparently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d);

// typemap definitions, code modifications / additions
%include "std_string.i"
%include "generate_java_interface.i"

%rename(NativeStringBuilder) StringBuilder;

%typemap(javabody) cx3d::StringBuilder %{
  private long swigCPtr;
  protected boolean swigCMemOwn;

  public $javaclassname(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  public static long getCPtr($javaclassname obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }
%}

%typemap(directorin, descriptor="Lini/cx3d/swig/NativeStringBuilder;") cx3d::StringBuilder& %{
  *(cx3d::StringBuilder **)&j$1 = (cx3d::StringBuilder *) &$1;
%}

// add the original header files here
%include "string_builder.h"
// %include "sim_state_serializable.h"
