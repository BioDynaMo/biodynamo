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
 * This file enables transparent type conversions between mpz_class (C++ libgmp)
 * and java.math.BigInteger (Java)
 * Quick solution using string as temporary representation.
 * Not very efficient, but this typemap is only needed temporarily until the whole
 * module has been ported.
 * If this typemap is needed in production: IMPROVE solution!
 */
%{
#include "gmpxx.h"
using BigInteger = mpz_class;
%}

%typemap(jstype) BigInteger, BigInteger&  "java.math.BigInteger"
%typemap(jtype) BigInteger, BigInteger& "java.math.BigInteger"
%typemap(jni) BigInteger, BigInteger& "jobject"

// return typemap
%typemap(javaout) BigInteger {
  return $jnicall;
}
%typemap(out) BigInteger{
  // get java string from C++ BigInteger
  char* c_str = mpz_get_str(NULL, 10, $1.get_mpz_t());
  jstring jstr = JCALL1(NewStringUTF, jenv, c_str);

  // create and initialize Java BigInteger
  jclass clazz = JCALL1(FindClass, jenv, "java/math/BigInteger");
  jmethodID mid = JCALL3(GetMethodID, jenv, clazz, "<init>", "(Ljava/lang/String;)V");
  jobject bigint = JCALL3(NewObject, jenv, clazz, mid, jstr);

  $result = bigint;
  delete c_str;
}

// function argument typemap
%typemap(javain) const BigInteger& "$javainput"
%typemap(in) const BigInteger& %{
  if (!$input) {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "BigInteger was null");
    return $null;
  }

  // get Java String from Java BigInteger object
  jclass $1_clazz = jenv->GetObjectClass($input);
  jmethodID $1_mid = jenv->GetMethodID($1_clazz, "toString", "()Ljava/lang/String;");
  jstring $1_jstr = (jstring) jenv->CallObjectMethod($input, $1_mid);

  // convert jstring into C string
  const char *$1_c_str = (const char *)jenv->GetStringUTFChars($1_jstr, 0);
  if (!$1_c_str) return $null;

  // create C big integer object
  BigInteger $1_big_int;
  $1_big_int.set_str($1_c_str, 10);

  $1 = &$1_big_int;

  // free memory
  jenv->ReleaseStringUTFChars($1_jstr, $1_c_str);
%}
