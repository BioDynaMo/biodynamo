//This macro can be used to transparently load a native library.
//The user of this library doesn't have to do anything

%define JAVA_LOAD_NATIVE_LIBRARY(LIBNAME)
%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("LIBNAME");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }
%}
%enddef
