%module cx3d

%{
#include "string_builder.h"

using namespace cx3d;
%}

//transpartently load native library - convenient for user
%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("cx3d");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }
%}

%include "std_string.i"

// add the original header files here
%include "string_builder.h"
