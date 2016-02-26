/**
 * This file enables transparent type conversions between std::list<T> (C++)
 * and java.util.AbstractSequentialList<T> (Java).
 * For each distinct template type one Java class is generated that
 * extends from java.util.AbstractList
 */
%{
#include <list>
#include <stdexcept>
#include "list_iterator_cpp.h"
%}

/**
* CPP declaration of std::list
* Contains only the needed definitions, but does not cover the whole functionality.
* Add further declarations on demand.
*/
namespace std {
template<class T> class list {
 public:
	typedef T &reference;
	typedef const T& const_reference;
	typedef T &iterator;
	typedef const T& const_iterator;

	list();
	list(unsigned int size, const T& value = T());
	list(const list<T> &);
	~list();

  %rename(size_impl) size;
	unsigned int size() const;
};
}  // namespace std

%include "list_iterator_cpp.h"

/**
 * Macro definition to create a Java class for the given std;:list<CPP_TYPE>.
 * The generated class extends from java.util.AbstractSequentialList
 * Elements are not copied between Java and C++! Java accesses the underlying
 * C++ data structure directly. The class ListIteratorCpp acts as a
 * Helper that does the necessary iterator modifications needed for
 * a functional AbstractSequentialList subclass.
 *
 * @param CPP_TYPE defines the data type for the std::list typemap
 * @param TEMPLATE_SUFFIX determines the name of the generated java class.
 *        It is appended to ListT_
 *        Naming recommendation: e.g. JAVA_TYPE=Double, SIZE=3 -> Double_3_
 *        Manual name specification is necessary due to naming conflicts:
 *        e.g. std::list<Rational> and std::list<std::shared_ptr<Rational>>
 *        would map to the same name (ListT_Rational) using built-in name generation.
 * @param JAVA_TYPE Java equivalent of CPP_TYPE. Can also be of primitive type
 *
 * usage example:
 * %stdlist_typemap(std::shared_ptr<cx3d::spatial_organization::Tetrahedron<cx3d::physics::PhysicalNode>>,
 *                  Tetrahedron,
 *                  ini.cx3d.spatialOrganization.Tetrahedron);
 */
%define %stdlist_typemap(CPP_TYPE, TEMPLATE_SUFFIX, JAVA_TYPE)
  %typemap(javabase) std::list<CPP_TYPE>, std::list<CPP_TYPE>& "java.util.AbstractSequentialList<"#JAVA_TYPE">";
  %typemap(javacode) std::list<CPP_TYPE>, std::list<CPP_TYPE>& %{
    @Override
    public int size() {
      return (int) size_impl();
    }

    private ListIterator iterator = null;

    @Override
    public java.util.ListIterator<JAVA_TYPE> listIterator(int i) {
      if(iterator == null){
        createIterator();
      }
      repositionIterator(i);
      return iterator;
    }

    private void createIterator() {
      iterator = new ListIterator(this);
    }

    /**
     * This method repositions the iterator. After this method returns
     * iterator will be at target position
     *
     * TODO(lukas) make more efficient
     * determine minimum travel distance not only based on current position, but also
     * on begin and end.
     */
    private void repositionIterator(int targetPosition){
      int itPos = iterator.nextIndex() - 1;
      if(itPos == targetPosition) {
        return;
      } else {
        int steps = Math.abs(itPos - targetPosition);
        while(steps-- != 0){
          if (itPos < targetPosition){
            iterator.incrementIterator();
          } else {
            iterator.decrementIterator();
          }
        }
      }
    }

    static class ListIterator implements java.util.ListIterator<JAVA_TYPE> {
      private ListIteratorCppT_##TEMPLATE_SUFFIX delegate = null;

      public ListIterator(final ListT_##TEMPLATE_SUFFIX list) {
        delegate = new ListIteratorCppT_##TEMPLATE_SUFFIX(list);
      }

      @Override
      public boolean hasNext() {
        return delegate.hasNext();
      }

      @Override
      public JAVA_TYPE next() {
        return delegate.next();
      }

      @Override
      public boolean hasPrevious() {
        return delegate.hasPrevious();
      }

      @Override
      public JAVA_TYPE previous() {
        return delegate.previous();
      }

      @Override
      public int nextIndex() {
        return delegate.nextIndex();
      }

      @Override
      public int previousIndex() {
        return delegate.previousIndex();
      }

      @Override
      public void remove() {
        delegate.remove();
      }

      @Override
      public void set(JAVA_TYPE value) {
        delegate.set(value);
      }

      @Override
      public void add(JAVA_TYPE value) {
        delegate.add(value);
      }

      public void incrementIterator() {
        delegate.incrementIterator();
      }

      public void decrementIterator() {
        delegate.decrementIterator();
      }
    }
  %}

  %pragma(java) modulecode=%{
    static ListT_##TEMPLATE_SUFFIX convertToNativeListT_##TEMPLATE_SUFFIX(java.util.AbstractSequentialList<JAVA_TYPE> list) {
        if (list instanceof ListT_##TEMPLATE_SUFFIX) {
            return (ListT_##TEMPLATE_SUFFIX) list;
        }
        ListT_##TEMPLATE_SUFFIX nativeList = new ListT_##TEMPLATE_SUFFIX();
        for (JAVA_TYPE el : list) {
            nativeList.add(el);
        }
        return nativeList;
    }
  %}

  %typemap(jstype)  std::list<CPP_TYPE>,
                    const std::list<CPP_TYPE>& "java.util.AbstractSequentialList<"#JAVA_TYPE">";

  %typemap(javain,
    pre = "    ListT_"#TEMPLATE_SUFFIX" temp$javainput = $module.convertToNativeListT_"#TEMPLATE_SUFFIX"($javainput);",
    pgcppname="temp$javainput") const std::list<CPP_TYPE>& "$javaclassname.getCPtr(temp$javainput)"

  %typemap(jstype)  CPP_TYPE cx3d::ListIteratorCpp<CPP_TYPE>::next,
                    CPP_TYPE cx3d::ListIteratorCpp<CPP_TYPE>::previous  "JAVA_TYPE";
  %typemap(jstype)  const CPP_TYPE & "JAVA_TYPE";

  %typemap(javadirectorout) std::list<CPP_TYPE> "ListT_"#TEMPLATE_SUFFIX".getCPtr((ListT_"#TEMPLATE_SUFFIX") $javacall)";

  %typemap(directorin, descriptor="Ljava/util/AbstractSequentialList;") std::list<CPP_TYPE>, std::list<CPP_TYPE>&, const std::list<CPP_TYPE>& %{
      *(std::list<CPP_TYPE> **)&j$1 = (std::list<CPP_TYPE>*) &$1;
%}

  %template(ListT_ ##TEMPLATE_SUFFIX) std::list<CPP_TYPE>;
  %template(ListIteratorCppT_ ##TEMPLATE_SUFFIX) cx3d::ListIteratorCpp<CPP_TYPE>;

  // delete typemap to avoid ripple effects
  %typemap(jstype)  const CPP_TYPE &;

%enddef
