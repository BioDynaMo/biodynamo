%module cx3d

%{
#include "string_builder.h"

using namespace cx3d;
%}

//transpartently load native library - convenient for user
%include "load_library.i"
JAVA_LOAD_NATIVE_LIBRARY(cx3d);

%include "std_string.i"

// add the original header files here
%include "string_builder.h"
