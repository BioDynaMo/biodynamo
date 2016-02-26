/**
 * Custom modifications for std::shared_ptr typemaps based on SWIG 3.0.7.
 * It solves the issue of inheriting from a Java Interface.
 * The original swig boost_shared_ptr could not deal with the scenario in which
 * the generated class implements an interface with function arguments of itself.
 * e.g.:
 * Interface method (RationalInterface.java):
 * RationalInterface add(RationalInterface other);
 *
 * generated Java class with standard shared_ptr implementation (Rational.java)
 * public Rational add(Rational other) {
 *    long cPtr = spatialOrganizationJNI.Rational_add(swigCPtr, this, Rational.getCPtr(other), other);
 *    return (cPtr == 0) ? null : new Rational(cPtr, true);
 * }
 *
 * To comply with the interface method signatures, the type must be changed:
 * %typemap(jstype) Rational "RationalInterface"
 *
 * result after typechange (Rational.java):
 * public RationalInterface add(RationalInterface other) {
 *    long cPtr = spatialOrganizationJNI.Rational_add(swigCPtr, this, RationalInterface.getCPtr(other));
 *    return (cPtr == 0) ? null : new RationalInterface(cPtr, true);
 * }
 *
 * Now the method signature is correct, but unfortunately also the types inside the method body
 * have been changed! This does not compile as there is no constructor and getCPtr method
 * in RationalInterface. Therefore this modifications are necessary. The constructor and
 * method calls are replaced by static functions in the method body (swigCreate and getCPtr)
 * which perform the required actions. The rest remained unchanged.
 * Result with this shared_ptr implementation (Rational.java)
 * public RationalInterface add(RationalInterface other) {
 *     long cPtr = spatialOrganizationJNI.Rational_add(swigCPtr, this, Rational.getCPtr(other));
 *     return (cPtr == 0) ? null : Rational.swigCreate(cPtr, true);
 * }
 */

#define SWIG_SHARED_PTR_NAMESPACE std

 // Users can provide their own SWIG_SHARED_PTR_TYPEMAPS macro before including this file to change the
 // visibility of the constructor and getCPtr method if desired to public if using multiple modules.
 #ifndef CX3D_SHARED_PTR_TYPEMAPS
 #define CX3D_SHARED_PTR_TYPEMAPS(CONST, JCLASS, TYPE...) CX3D_SHARED_PTR_TYPEMAPS_IMPLEMENTATION(protected, protected, CONST, JCLASS, TYPE)
 #endif

 %include <shared_ptr.i>

// Create separate macro, because cx3d modifications for std::shared_ptr take
// one more argument.
// Workaround empty first macro argument bug
#define SWIGEMPTYHACK
// Main user macro for defining cx3d_shared_ptr typemaps for both const and non-const pointer types
%define %cx3d_shared_ptr(JCLASS, JSTYPE_WO_GENERICS, TYPE...)
  %feature("smartptr", noblock=1) TYPE { SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< TYPE > }
  CX3D_SHARED_PTR_TYPEMAPS(SWIGEMPTYHACK, JCLASS, "JSTYPE_WO_GENERICS", TYPE)
  CX3D_SHARED_PTR_TYPEMAPS(const, JCLASS, "JSTYPE_WO_GENERICS", TYPE)
%enddef

// Language specific macro implementing all the customisations for handling the smart pointer
%define CX3D_SHARED_PTR_TYPEMAPS_IMPLEMENTATION(PTRCTOR_VISIBILITY, CPTR_VISIBILITY, CONST, JCLASS, JSTYPE_WO_GENERICS, TYPE...)

 // %naturalvar is as documented for member variables
 %naturalvar TYPE;
 %naturalvar SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >;

 // destructor wrapper customisation
 %feature("unref") TYPE
 //"if (debug_shared) { cout << \"deleting use_count: \" << (*smartarg1).use_count() << \" [\" << (boost::get_deleter<SWIG_null_deleter>(*smartarg1) ? std::string(\"CANNOT BE DETERMINED SAFELY\") : ( (*smartarg1).get() ? (*smartarg1)->getValue() : std::string(\"NULL PTR\") )) << \"]\" << endl << flush; }\n"
                                "(void)arg1; delete smartarg1;"

 // Typemap customisations...

 // plain value
 %typemap(in) CONST TYPE ($&1_type argp = 0) %{
   argp = (*(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$input) ? (*(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$input)->get() : 0;
   if (!argp) {
     SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null $1_type");
     return $null;
   }
   $1 = *argp; %}
 %typemap(out) CONST TYPE
 %{ *(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$result = new SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >(new $1_ltype(($1_ltype &)$1)); %}

 // plain pointer
 %typemap(in) CONST TYPE * (SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *smartarg = 0) %{
   smartarg = *(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$input;
   $1 = (TYPE *)(smartarg ? smartarg->get() : 0); %}
 %typemap(out, fragment="SWIG_null_deleter") CONST TYPE * %{
   *(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$result = $1 ? new SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >($1 SWIG_NO_NULL_DELETER_$owner) : 0;
 %}

 // plain reference
 %typemap(in) CONST TYPE & %{
   $1 = ($1_ltype)((*(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$input) ? (*(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$input)->get() : 0);
   if (!$1) {
     SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "$1_type reference is null");
     return $null;
   } %}
 %typemap(out, fragment="SWIG_null_deleter") CONST TYPE &
 %{ *(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$result = new SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >($1 SWIG_NO_NULL_DELETER_$owner); %}

 // plain pointer by reference
 %typemap(in) TYPE *CONST& ($*1_ltype temp = 0)
 %{ temp = (TYPE *)((*(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$input) ? (*(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$input)->get() : 0);
    $1 = &temp; %}
 %typemap(out, fragment="SWIG_null_deleter") TYPE *CONST&
 %{ *(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&$result = new SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >(*$1 SWIG_NO_NULL_DELETER_$owner); %}

 // shared_ptr by value
 %typemap(in) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > ($&1_type argp)
 %{ argp = *($&1_ltype*)&$input;
    if (argp) $1 = *argp; %}
 %typemap(out) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >
 %{ *($&1_ltype*)&$result = $1 ? new $1_ltype($1) : 0; %}

 // shared_ptr by reference
 %typemap(in) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > & ($*1_ltype tempnull)
 %{ $1 = $input ? *($&1_ltype)&$input : &tempnull; %}
 %typemap(out) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &
 %{ *($&1_ltype)&$result = *$1 ? new $*1_ltype(*$1) : 0; %}

 // shared_ptr by pointer
 %typemap(in) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > * ($*1_ltype tempnull)
 %{ $1 = $input ? *($&1_ltype)&$input : &tempnull; %}
 %typemap(out) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *
 %{ *($&1_ltype)&$result = ($1 && *$1) ? new $*1_ltype(*$1) : 0;
    if ($owner) delete $1; %}

 // shared_ptr by pointer reference
 %typemap(in) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& (SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > tempnull, $*1_ltype temp = 0)
 %{ temp = $input ? *($1_ltype)&$input : &tempnull;
    $1 = &temp; %}
 %typemap(out) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *&
 %{ *($1_ltype)&$result = (*$1 && **$1) ? new SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >(**$1) : 0; %}

 // various missing typemaps - If ever used (unlikely) ensure compilation error rather than runtime bug
 %typemap(in) CONST TYPE[], CONST TYPE[ANY], CONST TYPE (CLASS::*) %{
 #error "typemaps for $1_type not available"
 %}
 %typemap(out) CONST TYPE[], CONST TYPE[ANY], CONST TYPE (CLASS::*) %{
 #error "typemaps for $1_type not available"
 %}


 %typemap (jni)    SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "jlong"
 %typemap (jtype)  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "long"
 %typemap (jstype) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "$typemap(jstype, TYPE)"

 %typemap(javain) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "JCLASS.getCPtr($javainput)"
                  // SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "$typemap(jstype, TYPE).getCPtr($javainput)"

  %typemap(javadirectorout) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                   SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "JCLASS.getCPtr($javacall)"

  %typemap(javadirectorin) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "($jniinput == 0) ? null : JCLASS.swigCreate($jniinput, false)"

  %typemap(directorin, descriptor="L"JSTYPE_WO_GENERICS";") SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& %{
                          *(SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&j$1 = $1 ? new std::shared_ptr< CONST TYPE >($1) : 0;
                          %}

  %typemap(directorout, descriptor="L"JSTYPE_WO_GENERICS";") SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& %{
                          SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE  > *argp;
                          argp = *( SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > **)&jresult;
                          if (!argp) {
                            c_result = SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >(nullptr);
                          } else {
                            c_result = *argp;
                          }
                        %}

 %typemap(javaout) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > {
     long cPtr = $jnicall;
     return (cPtr == 0) ? null : JCLASS.swigCreate(cPtr, true);
   }
 %typemap(javaout) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > & {
     long cPtr = $jnicall;
     return (cPtr == 0) ? null : JCLASS.swigCreate(cPtr, true);
   }
 %typemap(javaout) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > * {
     long cPtr = $jnicall;
     return (cPtr == 0) ? null : JCLASS.swigCreate(cPtr, true);
   }
 %typemap(javaout) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& {
     long cPtr = $jnicall;
     return (cPtr == 0) ? null : JCLASS.swigCreate(cPtr, true);
   }

 %typemap(javaout) CONST TYPE {
     return new $typemap(jstype, TYPE)($jnicall, true);
   }
 %typemap(javaout) CONST TYPE & {
     return new $typemap(jstype, TYPE)($jnicall, true);
   }
 %typemap(javaout) CONST TYPE * {
     long cPtr = $jnicall;
     return (cPtr == 0) ? null : new $typemap(jstype, TYPE)(cPtr, true);
   }
 %typemap(javaout) TYPE *CONST& {
     long cPtr = $jnicall;
     return (cPtr == 0) ? null : new $typemap(jstype, TYPE)(cPtr, true);
   }

 // Base proxy classes
 %typemap(javabody) TYPE %{
   private long swigCPtr;
   protected boolean swigCMemOwn;

   PTRCTOR_VISIBILITY $javaclassname(long cPtr, boolean cMemoryOwn) {
     swigCMemOwn = cMemoryOwn;
     swigCPtr = cPtr;
   }

   CPTR_VISIBILITY static long getCPtr(Object o) {
     if(o == null){
       return 0;
     } else if(!(o instanceof $javaclassname)){
       throw new RuntimeException("Object " + o + " must be of type $javaclassname. Use Proxy to wrap this object");
     }
     $javaclassname obj = ($javaclassname) o;
     return (obj == null) ? 0 : obj.swigCPtr;
   }

   CPTR_VISIBILITY static $javaclassname swigCreate(long cPtr, boolean cMemoryOwn) {
     return new $javaclassname(cPtr, cMemoryOwn);
   }
 %}

 // Derived proxy classes
 %typemap(javabody_derived) TYPE %{
   private long swigCPtr;
   private boolean swigCMemOwnDerived;

   PTRCTOR_VISIBILITY $javaclassname(long cPtr, boolean cMemoryOwn) {
     super($imclassname.$javaclazznameSWIGSmartPtrUpcast(cPtr), true);
     swigCMemOwnDerived = cMemoryOwn;
     swigCPtr = cPtr;
   }

   CPTR_VISIBILITY static long getCPtr(Object o) {
     if(!(o instanceof $javaclassname)){
       throw new RuntimeException("Object " + o + " must be of type $javaclassname. Use Proxy to wrap this object");
     }
     $javaclassname obj = ($javaclassname) o;
     return (obj == null) ? 0 : obj.swigCPtr;
   }

   CPTR_VISIBILITY static $javaclassname swigCreate(long cPtr, boolean cMemoryOwn) {
     return new $javaclassname(cPtr, cMemoryOwn);
   }
 %}

 %typemap(javadestruct, methodname="delete", methodmodifiers="public synchronized") TYPE {
     if (swigCPtr != 0) {
       if (swigCMemOwn) {
         swigCMemOwn = false;
         $jnicall;
       }
       swigCPtr = 0;
     }
   }

 %typemap(javadestruct_derived, methodname="delete", methodmodifiers="public synchronized") TYPE {
     if (swigCPtr != 0) {
       if (swigCMemOwnDerived) {
         swigCMemOwnDerived = false;
         $jnicall;
       }
       swigCPtr = 0;
     }
     super.delete();
   }


 %template() SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >;
 %enddef
