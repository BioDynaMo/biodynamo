/**
 * This file contains partial macro applications for a specific type. It means, that it
 * defines a new macro and fixes the value of a number of arguments.
 * As a result it reduces the arity of the underlying macro.
 * Therefore, this macros are easier to use and result in a better readable
 * module file.
 * https://en.wikipedia.org/wiki/Partial_application
 *
 * e.g. given complex_macro(arg1, arg2, arg3, arg4, arg5);
 * set arg1=1, arg2=2, arg4=4, arg5=5:
 * -> partial macro:
 * %define %simplified(arg3)
 *    complex_macro(1, 2, arg3, 4, 5)
 * %enddef
 *
 * Documentation of macros and their arguments can be found in the included
 * files!
 */

%include "std_array_typemap.i"

%define %double_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_primitive_array_marshalling(SWIG_MODULE, double, Double_##SIZE, Double,
                                        double, D, SIZE);
%enddef

%define %double_stdarray_2dim_array_marshalling(SWIG_MODULE, SIZE_1, SIZE_2)
  %stdarray_2dim_array_marshalling(SWIG_MODULE, double, SIZE_1,
                                   ArrayT_Double_##SIZE_1##_##SIZE_2,
                                   double, D, SIZE_2);
%enddef

//------------------------------------------------------------------------------
// int

%define %int_stdarray_array_marshalling(SWIG_MODULE, SIZE)
  %stdarray_primitive_array_marshalling(SWIG_MODULE, int, Integer##SIZE, Integer,
                                        int, I, SIZE);
%enddef

%define %int_stdarray_2dim_array_marshalling(SWIG_MODULE, SIZE_1, SIZE_2)
  %stdarray_2dim_array_marshalling(SWIG_MODULE, int, SIZE_1,
                                   ArrayT_Integer_##SIZE_1##_##SIZE_2,
                                   int, I, SIZE_2);
%enddef
