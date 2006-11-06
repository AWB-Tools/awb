/*
 * Copyright (c) 2001 Nathan L. Binkert
 * All rights reserved.
 *
 * Permission to redistribute, use, copy, and modify this software
 * without fee is hereby granted, provided that the following
 * conditions are met:
 *
 * 1. This entire notice is included in all source code copies of any
 *    software which is or includes a copy or modification of this
 *    software.
 * 2. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @author Nathan Binkert
 * @brief
 */

#ifndef _VECTOR_
#define _VECTOR_

// ASIM core
#include "asim/utils.h"

namespace asim
{

/////////////////////////////////////////////////////////////////////////
// class Vector<class T>
//
// This class is a templatized implementation of a vector.  It is
// esentially an array class which can grow.  It has an interface
// similar to what you might see in the Standard Template Library(STL)
// complete with iterators for access to elements. All functions
// provided by this vector are O(1) time unless otherwise noted.
//
template<class T>
class Vector
{
private:
  // We're all friends here.
  friend class ConstIterator;
  friend class Iterator;

protected:
  //////////////
  //  Storage space for the Vector.
  //
  T *Start;
  T *Stop;
  T *StopStorage;

  //////////////
  // The current size of the Vector and the capacity of the Vector.
  // When the capacity is exceeded, the vector will grow to accomodate 
  // new elements.
  int Size() const;
  int Capacity() const;

public:
  // Forward declarations.
  class Iterator;
  class ConstIterator;

  ////////////////////////////////////////
  // class Vector<T>::ConstIterator
  //
  // This class is the Constant Iterator for the Vector.  The const
  // iterator is used when traversing a const Vector.  It will allow
  // the user to look at the elements in the vector, but the user will
  // be unable to modify any of the values in the vector and unable to
  // modify the vector itself (i.e. will not be able to insert or
  // remove).
  //
  class ConstIterator
  {
  private:
    // The vector must be able to access non public functions.
    friend class Vector<T>;

  protected:
    // We need to store more data than just a pointer to the current
    // element in the vector.  This is because we need to be able to
    // do bounds checking on the vector in case we continue past the
    // end.
    const T *Element;

    // The vector class needs to be able to create arbitrary iterators
    // from a pointer to the vector class and a posision, but normal
    // accessors of the vector should not be able to do this.
    ConstIterator(T *e);

  public:
    // Default constructor initializes the iterator.  When creating an
    // iterator with the default constructor, we are unable to call
    // any function other than operator=.  This is because the
    // iterator does not actually point to anything and is not
    // dereferencable.
    ConstIterator();

    // These two copy constructors will simply duplicate the position
    // of an iterator.  The second copy constructor can also be used
    // for implicit casting from an Iterator to a Const Iterator.
    ConstIterator(const ConstIterator& iter);
    explicit ConstIterator(const Iterator& iter);

    // No destructor is needed for the iterator.
    ~ConstIterator() {}

    // Preincrement, postincrement, predecrement, postdecrement.
    // One unfortunate side affect of the way that this code must be
    // written is that preincrement (and predecrement) is faster that
    // postincrement (and postdecrement).  So, unless you need the
    // functionality, favor the pre* version rather than the post*
    // version.
    // e.g.
    //   use:
    //     ++iter;
    //   in place of:
    //     iter++;
    //
    const ConstIterator& operator++();
    const ConstIterator operator++(int);
    const ConstIterator& operator--();
    const ConstIterator operator--(int);

    // Since Iterators are supposed to behave like pointers, we must
    // overload these operators so that we can add an offset.
    const ConstIterator& operator+=(int offset);
    const ConstIterator& operator-=(int offset);

    const ConstIterator operator+(int offset) const;
    const ConstIterator operator-(int offset) const;

    bool operator==(const ConstIterator& r) const;
    bool operator!=(const ConstIterator& r) const;
    bool operator< (const ConstIterator& r) const;
    bool operator> (const ConstIterator& r) const;
    bool operator<=(const ConstIterator& r) const;
    bool operator>=(const ConstIterator& r) const;

    bool operator==(const Iterator& r) const;
    bool operator!=(const Iterator& r) const;
    bool operator< (const Iterator& r) const;
    bool operator> (const Iterator& r) const;
    bool operator<=(const Iterator& r) const;
    bool operator>=(const Iterator& r) const;

    // Changes what the iterator points to.
    const ConstIterator& operator=(const ConstIterator& iter);
    const ConstIterator& operator=(const Iterator& iter);

    // operator* is used to dereference the iterator.  When the
    // iterator is dereferenced, what is returned is the data that is
    // at the node that the iterator is pointing to.
    // e.g.
    //     Vector<int>::Iterator iter = vector.Begin();
    //     int x = *iter;
    const T& operator*() const;

    // Since we are talking about a vector here, we should be able to
    // use random access to get to anywhere we want to in the vector in
    // O(1) time.
    const T& operator[](int index) const;

    // operator-> is used to point to data members of the data that
    // the iterator is pointing to.  N.B. if the iterator data type is 
    // a pointer. operator-> does not make sense because it would be
    // working on a pointer to a pointer.
    // e.g.  THIS IS BAD!
    //     struct my_struct {
    //       int x;
    //       int y;
    //       int z;
    //     };
    //     ...
    //     Vector<my_struct*>::Iterator iter = vector.Begin();
    //     int i = iter->z;    // BAD!  iter-> returns a my_struct**
    //     int j = (*iter)->z  // OK!   iter* returns a my_struct*
    const T* operator->() const;
  };

  ////////////////////////////////////////
  // class Vector<T>::Iterator
  //
  // This class is the Mutable Iterator for the Vector.  The mutable
  // iterator is used when traversing a mutable Vector.  It will allow
  // the user to look at the elements in the vector, as well as modify
  // the data in the vector and modify the vector itself by insertion and
  // removal.
  //
  // Look at the comments for ConstIterator since all of the functions 
  // are basically the same.
  //
  class Iterator
  {
  private:
    friend class Vector<T>;

  protected:
    T *Element;

    Iterator(T *e);

  public:
    Iterator();
    Iterator(const Iterator& iter);
    ~Iterator() {}

    const Iterator& operator++();
    const Iterator operator++(int);
    const Iterator& operator--();
    const Iterator operator--(int);

    const Iterator& operator+=(int offset);
    const Iterator& operator-=(int offset);

    const Iterator operator+(int offset) const;
    const Iterator operator-(int offset) const;

    bool operator==(const Iterator& r) const;
    bool operator!=(const Iterator& r) const;
    bool operator< (const Iterator& r) const;
    bool operator> (const Iterator& r) const;
    bool operator<=(const Iterator& r) const;
    bool operator>=(const Iterator& r) const;

    bool operator==(const ConstIterator& r) const;
    bool operator!=(const ConstIterator& r) const;
    bool operator< (const ConstIterator& r) const;
    bool operator> (const ConstIterator& r) const;
    bool operator<=(const ConstIterator& r) const;
    bool operator>=(const ConstIterator& r) const;

    const Iterator& operator=(const Iterator& iter);

    T& operator*() const;
    T& operator[](int index) const;
    T* operator->() const;
  };

protected:
  // Clean up after the vector.
  void Fini();

  // Private Shuffling functions.  See Shuffle() for a more detailed
  // description.
  //
  // ShuffleLeft() - can ONLY shuffle a negative count (to the left)
  // ShuffleRight() - can ONLY shuffle a positive count (to the right)
  //                  and will call ShuffleRightAlloc() if memory must
  //                  be allocated.
  // ShuffleRightAlloc() - can ONLY shuffle a positive count and can only
  //                       be used in the case where space allocation
  //                       is necessary.
  bool ShuffleLeft(T *pos, int count);
  bool ShuffleRight(T *pos, int count);
  bool ShuffleRightAlloc(T *pos, int count);

public:
  // Default constructor and copy constructor.  Nothing special here.
  Vector(int cap = 16);
  Vector(const Vector<T>& vector);

  // We must free the memory when we destroy the vector.
  // N.B.  We are assuming that this class will not be derived from
  //       and are not providing a virtual destructor.  If a vector
  //       class with virtual functions is desired, then this class
  //       must be wrapped by another class that will do this
  //       properly.  It is possible to derive from this class.  If
  //       this is done, the Fini() function must be called to clean
  //       up.  (Again because there is no virtual destructor.)  It
  //       would be much better to wrap this class though.
  ~Vector();

  // Change the current size of the vector.  This will notify the
  // vector that it is now longer so that other operations will
  // perform properly.
  // 
  // If new space is allocated, all iterators are invalidated.  This
  // function will return true if new space is allocated, and false
  // otherwise.
  bool Resize(int new_size);

  // Shuffle elements in the vector.  This function will shuffle in
  // either direction (positive count is to the right, negative is to
  // the left.  On a shuffle right, the gap left is not zeroed.  Also,
  // on a shuffle right, the capacity of the vector may be increased.
  // These calls may be used to remove objects from the middle of the
  // vector or insert a space in the middle for new objects.
  //
  // Iterators >= (pos - count) will be invalidated, unless there is
  // new space allocated, in which case, all iterators are
  // invalidated.
  //
  // This function will return true if new space is allocated, and
  // false otherwise.
  bool Shuffle(T *pos, int count);

  // Simple functions that return statistics on use and capacity of
  // the vector.
  int GetOccupancy() const;
  int GetCapacity() const;
  int GetFreeSpace() const;
  bool IsFull() const;
  bool IsEmpty() const;

  // Returns an iterator that points to the first element in the
  // vector.  If the vector is empty, this will be equal to End() and
  // not be dereferenceable. In other cases, it will be
  // dereferenceable.
  Iterator Begin();
  ConstIterator Begin() const;

  // Returns an iterator that points to the end of the vector.  The
  // element that it points to is actually just a place holder and
  // considered to be one beyond the end of the vector.  This is
  // somewhat like the null termination ('\0') that we would find at
  // the end of a C style string (char *).
  Iterator End();
  ConstIterator End() const;

  // Functions for insertion and removal of objects into and from the
  // vector.
  //

  // Inserts the data into a new element before the position pointed
  // to by pos.  If pos == Begin(), Begin will properly be updated.
  // The return value is an iterator that points to the newly inserted 
  // element.  End() is returned on a failure.
  //
  // N.B. This operation is an O(N) operation.  Things inserted at the 
  // beginning must shuffle all elements to the right and will
  // actually take Theta(N) time.  Insertion at the end does not
  // require shuffling elements down the vector, and is therefore
  // constant time.
  Iterator Insert(Iterator pos, T data);

  // Inserts n copies of the data before the position pointed to by
  // pos.  Returns true on success and false on failure.
  bool Insert(Iterator pos, T data, int n);
  
  // Inserts a range of elements before the position pointed to by
  // pos.  Begin is the first element in the range, and end is one
  // past the end. i.e. [begin, end) will be inserted before pos.
  // Returns true on success and false on failure.
  bool Insert(Iterator pos, ConstIterator begin, ConstIterator end);

  // Remove the element pointed to by the iterator pos from the
  // vector.  The iterator pos is considered invalid if it has been
  // removed from the vector and should be discarded.
  //
  // N.B. This operation is also O(N) for the same reasons as Insert.
  Iterator Remove(Iterator pos);

  // Removes all elements in the range [begin, end) from the vector.
  // All iterators that point to elements in that range will be
  // invalidated an should be discarded.
  bool Remove(Iterator begin, Iterator end);


  // Copies the vector.  First all elements are removed from the
  // vector, then a memberwise copy is performed.
  const Vector<T>& operator=(const Vector<T>& vector);

  // Simple memberwise comparisons of the vector.
  bool operator==(const Vector<T>& vector);
  bool operator!=(const Vector<T>& vector);
};

///////////////////////////////////
// class Vector<T>::ConstIterator
//

////////////////////////
// Protected Functions:
//

// This is a protected constructor.  It is used by the Vector class to 
// create an iterator from any position in the vector class.
template<class T>
inline
Vector<T>::ConstIterator::ConstIterator(T *e)
  : Element(e)
{}

/////////////////////
// Public Functions
//

// Default constructor for the Vector class.  This constructor can be
// used when the iterator is in a loop and the initial value will not
// be known.  When this constructor is used, the iterator is invalid
// and will not be dereferenceable.
template<class T>
inline
Vector<T>::ConstIterator::ConstIterator()
  : T(NULL)
{}

template<class T>
inline
Vector<T>::ConstIterator::ConstIterator(const ConstIterator& iter)
{ operator=(iter); }

template<class T>
inline
Vector<T>::ConstIterator::ConstIterator(const Iterator& iter)
{ operator=(iter); }

template<class T>
inline const typename Vector<T>::ConstIterator&
Vector<T>::ConstIterator::operator++()
{ ++Element; }

template<class T>
inline const typename Vector<T>::ConstIterator
Vector<T>::ConstIterator::operator++(int)
{
  ConstIterator iter = *this;
  ++(*this);
  return iter;
}

template<class T>
inline const typename Vector<T>::ConstIterator&
Vector<T>::ConstIterator::operator--()
{ --Element; }

template<class T>
inline const typename Vector<T>::ConstIterator
Vector<T>::ConstIterator::operator--(int)
{
  ConstIterator iter = *this;
  --(*this);
  return iter;
}

template<class T>
inline const typename Vector<T>::ConstIterator&
Vector<T>::ConstIterator::operator+=(int offset)
{ Element += offset; return *this; }

template<class T>
inline const typename Vector<T>::ConstIterator&
Vector<T>::ConstIterator::operator-=(int offset)
{ Element -= offset; return *this; }

template<class T>
inline const typename Vector<T>::ConstIterator&
Vector<T>::ConstIterator::operator=(const ConstIterator& iter)
{
  if (&iter == this)
    return *this;

  Element = iter.Element;

  return *this;
}

template<class T>
inline const typename Vector<T>::ConstIterator&
Vector<T>::ConstIterator::operator=(const Iterator& iter)
{
  Element = iter.Element;
  return *this;
}

template<class T>
inline const typename Vector<T>::ConstIterator
Vector<T>::ConstIterator::operator+(int offset) const
{ return ConstIterator(Element + offset); }

template<class T>
inline const typename Vector<T>::ConstIterator
Vector<T>::ConstIterator::operator-(int offset) const
{ return ConstIterator(Element - offset); }

template<class T>
inline bool
Vector<T>::ConstIterator::operator==(const Iterator& iter) const
{ return (Element == iter.Element); }

template<class T>
inline bool
Vector<T>::ConstIterator::operator!=(const Iterator& iter) const
{ return (Element != iter.Element); }

template<class T>
inline bool
Vector<T>::ConstIterator::operator< (const Iterator& iter) const
{ return Element < iter.Element; }

template<class T>
inline bool
Vector<T>::ConstIterator::operator> (const Iterator& iter) const
{ return Element > iter.Element; }

template<class T>
inline bool
Vector<T>::ConstIterator::operator<=(const Iterator& iter) const
{ return Element <= iter.Element; }

template<class T>
inline bool
Vector<T>::ConstIterator::operator>=(const Iterator& iter) const
{ return Element >= iter.Element; }

template<class T>
inline bool
Vector<T>::ConstIterator::operator==(const ConstIterator& iter) const
{ return (Element == iter.Element); }

template<class T>
inline bool
Vector<T>::ConstIterator::operator!=(const ConstIterator& iter) const
{ return (Element != iter.Element); }

template<class T>
inline bool
Vector<T>::ConstIterator::operator< (const ConstIterator& iter) const
{ return Element < iter.Element; }

template<class T>
inline bool
Vector<T>::ConstIterator::operator> (const ConstIterator& iter) const
{ return Element > iter.Element; }

template<class T>
inline bool
Vector<T>::ConstIterator::operator<=(const ConstIterator& iter) const
{ return Element <= iter.Element; }

template<class T>
inline bool
Vector<T>::ConstIterator::operator>=(const ConstIterator& iter) const
{ return Element >= iter.Element; }

template<class T>
inline const T&
Vector<T>::ConstIterator::operator[](int index) const
{ return Element[index]; }

template<class T>
inline const T&
Vector<T>::ConstIterator::operator*() const
{ return *Element; }

template<class T>
inline const T*
Vector<T>::ConstIterator::operator->() const
{ return Element; }


//////////////////////////////
// class Vector<T>::Iterator
//

////////////////////////
// Protected Functions
//
template<class T>
inline
Vector<T>::Iterator::Iterator(T *e)
  : Element(e)
{ 
}

/////////////////////
// Public Functions
//
template<class T>
inline
Vector<T>::Iterator::Iterator()
  : Element(NULL)
{}

template<class T>
inline
Vector<T>::Iterator::Iterator(const Iterator& iter)
{ operator=(iter); }

template<class T>
inline const typename Vector<T>::Iterator&
Vector<T>::Iterator::operator++()
{ ++Element; return *this; }

template<class T>
inline const typename Vector<T>::Iterator
Vector<T>::Iterator::operator++(int)
{
  Iterator iter = *this;
  ++(*this);
  return iter;
}

template<class T>
inline const typename Vector<T>::Iterator&
Vector<T>::Iterator::operator--()
{ --Element; return *this; }

template<class T>
inline const typename Vector<T>::Iterator
Vector<T>::Iterator::operator--(int)
{
  Iterator iter = *this;
  --(*this);
  return iter;
}

template<class T>
inline const typename Vector<T>::Iterator&
Vector<T>::Iterator::operator+=(int offset)
{ Element += offset; return *this; }

template<class T>
inline const typename Vector<T>::Iterator&
Vector<T>::Iterator::operator-=(int offset)
{ Element -= offset; return *this; }

template<class T>
inline const typename Vector<T>::Iterator&
Vector<T>::Iterator::operator=(const Iterator& iter)
{
  if (this == &iter)
    return *this;

  Element = iter.Element;
  return *this;
}

template<class T>
inline const typename Vector<T>::Iterator
Vector<T>::Iterator::operator+(int offset) const
{ return Iterator(Element + offset); }

template<class T>
inline const typename Vector<T>::Iterator
Vector<T>::Iterator::operator-(int offset) const
{ return Iterator(Element - offset); }

template<class T>
inline bool
Vector<T>::Iterator::operator==(const Iterator& iter) const
{ return (Element == iter.Element); }

template<class T>
inline bool
Vector<T>::Iterator::operator!=(const Iterator& iter) const
{ return (Element != iter.Element); }

template<class T>
inline bool
Vector<T>::Iterator::operator< (const Iterator& iter) const
{ return Element < iter.Element; }

template<class T>
inline bool
Vector<T>::Iterator::operator> (const Iterator& iter) const
{ return Element > iter.Element; }

template<class T>
inline bool
Vector<T>::Iterator::operator<=(const Iterator& iter) const
{ return Element <= iter.Element; }

template<class T>
inline bool
Vector<T>::Iterator::operator>=(const Iterator& iter) const
{ return Element >= iter.Element; }

template<class T>
inline bool
Vector<T>::Iterator::operator==(const ConstIterator& iter) const
{ return (Element == iter.Element); }

template<class T>
inline bool
Vector<T>::Iterator::operator!=(const ConstIterator& iter) const
{ return (Element != iter.Element); }

template<class T>
inline bool
Vector<T>::Iterator::operator< (const ConstIterator& iter) const
{ return Element < iter.Element; }

template<class T>
inline bool
Vector<T>::Iterator::operator> (const ConstIterator& iter) const
{ return Element > iter.Element; }

template<class T>
inline bool
Vector<T>::Iterator::operator<=(const ConstIterator& iter) const
{ return Element <= iter.Element; }

template<class T>
inline bool
Vector<T>::Iterator::operator>=(const ConstIterator& iter) const
{ return Element >= iter.Element; }

template<class T>
inline T&
Vector<T>::Iterator::operator[](int index) const
{ return Element[index]; }

template<class T>
inline T&
Vector<T>::Iterator::operator*() const
{ return *Element; }

template<class T>
inline T*
Vector<T>::Iterator::operator->() const
{ return Element; }

////////////////////
// class Vector<T>
//

/////////////////////////
// Protected Functions

// Simple finalization funciton.  Should be called from the
// destructor.  If this class is derived from, then the derived class
// should call Fini() to clean up after itself since there is no
// virtual destructor.  Multiple calls to Fini will be safe.
template<class T>
inline void
Vector<T>::Fini()
{
  if (Start) {
    delete [] Start;
    Start = NULL;
  }
}

template<class T>
inline int
Vector<T>::Size() const
{ return Stop - Start; }

template<class T>
inline int
Vector<T>::Capacity() const
{ return StopStorage - Start; }

// This function will resize the vector.  The return value will
// indicate whether or not the vector was actually resized.  The
// vector will use array doubling, so the capacity of the vector will
// be rounded up to the next power of 2.  (i.e. round up to nearest
// 2^n.)  The storage for the vector will also never shrink.
template<class T>
inline bool
Vector<T>::Resize(int new_size)
{
  if (new_size <= 0) {
    Stop = Start;
    return false;
  }

  if (new_size < Capacity()) {
    Stop = Start + new_size;
    return false;
  }
  
  int new_cap = ceilPow2(new_size);
  T *new_start = new T[new_cap];
  
  T *elem = Start;
  T *new_elem = new_start;
  while (elem != Stop)
    *new_elem++ = *elem++;

  delete [] Start;

  Start = new_start;
  Stop = new_start + new_size;
  StopStorage = new_start + new_cap;

  return true;
}

// These functions will starting at the position pos, shuffle the
// elements count positions.  When Shuffling to the right, if there is
// not enough space to do the shuffle, new storage will be allocated.
// Like resize, the capacity will follow an array doubling scheme, and
// will be enlarged to the next power of two size.
template<class T>
inline bool
Vector<T>::ShuffleRightAlloc(T *pos, int count)
{
  ASSERTX(Size() + count > Capacity());

  if (pos == Stop)
    return Resize(Size() + count);
  
  int new_cap = ceilPow2(Size() + count);
  T *new_start = new T[new_cap];
  T *new_stop = new_start + Size() + count;
  
  T *elem = Start;
  T *new_elem = new_start;
  
  while (elem != pos)
    *new_elem++ = *elem++;
  new_elem += count;
  while (elem != Stop)
    *new_elem++ = *elem++;
  
  delete [] Start;
  
  Start = new_start;
  Stop = new_stop;
  StopStorage = new_start + new_cap;

  return true;
}

template<class T>
inline bool
Vector<T>::ShuffleRight(T *pos, int count)
{
  ASSERTX(count >= 0);

  if (Size() + count > Capacity()) {
    return ShuffleRightAlloc(pos, count);
  }

  if (pos != Stop) {
    T *new_elem = Stop + count;
    T *elem = Stop;

    do {
      *(--new_elem) = *(--elem);
    } while (elem != pos);
  }

  Stop += count;
  
  return false;
}

template<class T>
inline bool
Vector<T>::ShuffleLeft(T *pos, int count)
{
  ASSERTX(count <= 0);

  if (pos != Stop) {
    T *new_elem = pos + count; // count is negative
    T *elem = pos;

    do {
      *(new_elem++) = *(elem++);
    } while (elem != Stop);
  }
  
  Stop += count; // count is negative

  return false;
}

template<class T>
inline bool
Vector<T>::Shuffle(T *pos, int count)
{
  if (count == 0)
    return false;

  if (pos == Stop)
    return Resize(Size() + count);

  return (count > 0) ? ShuffleRight(pos, count) : ShuffleLeft(pos, count);
}

/////////////////////
// Public Functions
//
template<class T>
inline
Vector<T>::Vector(int cap)
{ 
  if (cap <= 16)
    cap = 16;
  else
    cap = ceilPow2(cap);

  Start = new T[cap];
  Stop = Start;
  StopStorage = Start + cap;
}

template<class T>
inline
Vector<T>::Vector(const Vector<T>& vector)
{ operator=(vector); }

template<class T>
inline
Vector<T>::~Vector()
{ Fini(); }

template<class T>
inline int
Vector<T>::GetOccupancy() const
{ return Size(); }

template<class T>
inline int
Vector<T>::GetCapacity() const
{ return Capacity(); }

template<class T>
inline int
Vector<T>::GetFreeSpace() const
{ return Capacity() - Size(); }

template<class T>
inline bool
Vector<T>::IsFull() const
{ return Size() == Capacity(); }

template<class T>
inline bool
Vector<T>::IsEmpty() const
{ return Size() == 0; }

template<class T>
inline typename Vector<T>::Iterator
Vector<T>::Begin()
{ return Iterator(Start); }

template<class T>
inline typename Vector<T>::ConstIterator
Vector<T>::Begin() const
{ return ConstIterator(Start); }

template<class T>
inline typename Vector<T>::Iterator
Vector<T>::End()
{ return Iterator(Stop); }

template<class T>
inline typename Vector<T>::ConstIterator
Vector<T>::End() const
{ return ConstIterator(Stop); }

template<class T>
inline typename Vector<T>::Iterator
Vector<T>::Insert(Iterator pos, T data)
{
  int off = pos.Element - Start;

  ShuffleRight(pos.Element, 1);
  Start[off] = data;

  return Iterator(Start + off);
}

template<class T>
inline bool
Vector<T>::Insert(Iterator pos, T data, int n)
{
  int off = pos.Element - Start;

  ShuffleRight(pos.Element, n);

  T *elem = Start + off;
  while(n--)
    this->elem[off + n] = data;

  return true;
}

template<class T>
inline bool
Vector<T>::Insert(Iterator pos, ConstIterator begin, ConstIterator end)
{
  if (end <= begin)
    return false;

  int off = pos.Element - Start;
  ShuffleRight(pos.Element, end - begin);

  T *elem = Start + off;
  while (begin != end)
    *elem++ == *begin++;

  return true;
}


template<class T>
inline typename Vector<T>::Iterator
Vector<T>::Remove(Iterator pos)
{
  T *elem = pos;
  ShuffleLeft(elem + 1, -1);

  return Iterator(elem);
}

template<class T>
inline bool
Vector<T>::Remove(Iterator begin, Iterator end)
{
  if (end <= begin)
    return false;

  ShuffleLeft(begin.Element, begin - end);
}

template<class T>
inline const Vector<T>&
Vector<T>::operator=(const Vector<T>& vector)
{
  if (&vector == this)
    return *this;

  Resize(vector.Size());

  T *elem = Start;
  T *velem = vector.Start;
  while (elem != Stop)
    *(elem++) = *(velem++);

  return *this;
}

template<class T>
inline bool
operator==(const Vector<T>& l, const Vector<T>& r)
{
  if (&l == &r)
    return true;

  if (l.GetSize() != r.GetSize())
    return false;

  typename Vector<T>::Iterator i = l.Begin();
  typename Vector<T>::Iterator j = r.Begin();

  while (i != l.End()) {
    assert(j != r.End());
    if (*i != *j)
      return false;
    
    ++i;
    ++j;
  }

  assert(j == r.End());

  return true;
}

template<class T>
inline bool
operator!=(const Vector<T>& l, const Vector<T>& r)
{ return !operator==(l, r); }


}

#endif //_VECTOR_
