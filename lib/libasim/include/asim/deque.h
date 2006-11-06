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

#ifndef _DEQUE_
#define _DEQUE_

// generic
#include <new>

/////////////////////////////////////////////////////////////////////////
// class Deque<class T, int N>
//
// This class is a templatized implementation of a fixed size array
// based double ended queue.  This should be used when the maximum
// size of the deque is known at compile time.  Since it is array
// based, only insertion operations from the beginning and end are in
// O(1) time.  Other insertions would take O(N) time because elements
// in the list would have to be shifted.  Interface ideas for this
// class were taken from the Standard Template Library(STL).  This
// class also comes complete with random access iterators for looking
// through elements in the queue.
//
template <class T, int N>
class Deque
{
private:
  // We're all friends here.
  friend class Iterator;
  friend class ConstIterator;

protected:
  ///////////////
  // Data storage.

  // This struct is designed to be a chunk of storage for the type
  // that is used.  It is allocated in terms of "long" so that it will 
  // align the data on a proper boundary.  (64-bit on alpha, 32-bit on 
  // x86).
  // NB: If this data structure is used with a type that has a storage 
  // size of < sizeof(long), then there will be waste.
  struct Raw { 
    long foo[(sizeof(T) - 1)/sizeof(long) + 1]; 
  };
  // All of the data is stored in an array.  The Array holds one extra
  // element to give space for the End() pointer.
  Raw Data[N+1];

  // Since the array is of raw data, we provide an access function to
  // cast that data for you.  (Both a const and non-const are available.)
  T& Get(int index);
  const T& Get(int index) const;

  int Size;   // Current number of elements in use in the deque.
  int _Begin; // Pointer to the first element in the deque.
  int _End;   // Pointer one beyond the last element in the deque.

public:
  class Iterator;
  class ConstIterator
  {
  private:
    friend class Deque<T,N>;

  protected:
    const Deque<T,N>* TheDeque;
    int Index;
    ConstIterator(const Deque<T,N>* deque, int index);

  public:
    ConstIterator();
    ConstIterator(const Deque<T,N>& deque);
    ConstIterator(const ConstIterator& iter);
    ConstIterator(const Iterator& iter);
    ~ConstIterator();

    const ConstIterator& operator++();
    const ConstIterator operator++(int);
    const ConstIterator& operator--();
    const ConstIterator operator--(int);

    const ConstIterator& operator=(const ConstIterator& iter);
    const ConstIterator& operator=(const Iterator& iter);
    const ConstIterator& operator+=(int off);
    const ConstIterator& operator-=(int off);
    const ConstIterator operator+(int off);
    const ConstIterator operator-(int off);

    bool operator==(const ConstIterator& iter) const;
    bool operator!=(const ConstIterator& iter) const;
    bool operator<(const ConstIterator& iter) const;
    bool operator>(const ConstIterator& iter) const;
    bool operator<=(const ConstIterator& iter) const;
    bool operator>=(const ConstIterator& iter) const;

    const T& operator[](int index) const;
    const T& operator*() const ;
    const T* operator->() const;
  };

  class Iterator
  {
  private:
    friend class Deque<T,N>;

  protected:
    Deque<T,N>* TheDeque;
    int Index;
    Iterator(Deque<T,N>* deque, int index);

  public:
    Iterator();
    Iterator(Deque<T,N>& deque);
    Iterator(const Iterator& iter);
    ~Iterator();

    const Iterator& operator++();
    const Iterator operator++(int);
    const Iterator& operator--();
    const Iterator operator--(int);

    const Iterator& operator=(const Iterator& iter);
    const Iterator& operator+=(int off);
    const Iterator& operator-=(int off);
    const Iterator operator+(int off);
    const Iterator operator-(int off);

    bool operator==(const Iterator& iter) const;
    bool operator!=(const Iterator& iter) const;
    bool operator<(const Iterator& iter) const;
    bool operator>(const Iterator& iter) const;
    bool operator<=(const Iterator& iter) const;
    bool operator>=(const Iterator& iter) const;

    T& operator[](int index) const;
    T& operator*() const;
    T* operator->() const;
  };

protected:
  // Alloc and Free are designed to construct and destroy objects in
  // the preallocated space.
  // The reason for doing this is so that objects are created and
  // destroyed when they are inserted and removed from the deque.
  // This gives correct semantics for just about every use of this
  // structure, though there is some potential overhead if your
  // default constructor is heavyweight.  (Which it shouldn't be!)
  void Alloc(int i, const T& data);
  void Free(int i);

  // Increment and Decrement the index and handle wrapping.
  void Inc(int &i);
  void Dec(int &i);

  // protected push/pop functions that do the real work.
  void _PushFront(const T& data);
  void _PushBack(const T& data);
  void _PopFront();
  void _PopBack();

public:
  Deque();
  Deque(const Deque<T,N>& deque);
  ~Deque(){}
  
  int GetOccupancy() const;
  int GetCapacity() const;
  int GetFreeSpace() const;
  bool IsFull() const;
  bool IsEmpty() const;

  Iterator Begin();
  Iterator End();
  ConstIterator Begin() const;
  ConstIterator End() const;

  T& Front();
  T& Back();
  const T& Front() const;
  const T& Back() const;
  
  bool PushFront(const T& data);
  bool PushBack(const T& data);

  bool PopFront(T& data);
  bool PopBack(T& data);
  bool PopFront();
  bool PopBack();
 
  Iterator Insert(Iterator pos, const T& data);
  bool Insert(Iterator pos, const T& data, int n);
  bool Insert(Iterator pos, Iterator begin, Iterator end);
  bool Remove(Iterator pos);
  bool Remove(Iterator begin, Iterator end);

  const Deque<T,N>& operator=(const Deque<T,N>& deque);
  bool operator==(const Deque<T,N>& deque) const;
  bool operator!=(const Deque<T,N>& deque) const;
};

///////////////////////////////////////////////
// class Deque<class T, int N>::ConstIterator
//

// Protected functions
//
template<class T, int N>
inline
Deque<T,N>::ConstIterator::ConstIterator(const Deque<T,N>* deque, int index)
  : TheDeque(deque), Index(index)
{}

// Public functions
//
template<class T, int N>
inline
Deque<T,N>::ConstIterator::ConstIterator()
  : TheDeque(NULL)
{}

template<class T, int N>
inline
Deque<T,N>::ConstIterator::ConstIterator(const Deque<T,N>& deque)
{ operator=(deque.Begin()); }

template<class T, int N>
inline
Deque<T,N>::ConstIterator::ConstIterator(const ConstIterator& iter)
{ operator=(iter); }

template<class T, int N>
inline
Deque<T,N>::ConstIterator::ConstIterator(const Iterator& iter)
{ operator=(iter); }

template<class T, int N>
inline
Deque<T,N>::ConstIterator::~ConstIterator()
{}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator&
Deque<T,N>::ConstIterator::operator++()
{
  if (Index != TheDeque->_End) {
    if (++Index > TheDeque->GetCapacity())
      Index = 0;
  }

  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator
Deque<T,N>::ConstIterator::operator++(int)
{
  ConstIterator i = *this;
  operator++();
  return i;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator&
Deque<T,N>::ConstIterator::operator--()
{
  if (Index != TheDeque->_Begin) {
    if (--Index < 0)
      Index = TheDeque->GetCapacity();
  }

  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator
Deque<T,N>::ConstIterator::operator--(int)
{
  ConstIterator i = *this;
  operator--();
  return i;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator&
Deque<T,N>::ConstIterator::operator=(const ConstIterator& iter)
{
  Index = iter.Index;
  TheDeque = iter.TheDeque;
  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator&
Deque<T,N>::ConstIterator::operator=(const Iterator& iter)
{
  Index = iter.Index;
  TheDeque = iter.TheDeque;
  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator&
Deque<T,N>::ConstIterator::operator+=(int off)
{
  Index += off;

  int max = TheDeque->GetCapacity();
  if (Index > max)
    Index -= max + 1;

  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator&
Deque<T,N>::ConstIterator::operator-=(int off)
{
  Index -= off;

  if (Index < 0)
    Index += TheDeque->GetCapacity() + 1;

  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator
Deque<T,N>::ConstIterator::operator+(int off)
{
  ConstIterator i = *this;
  i += off;
  return i;
}

template<class T, int N>
inline const typename Deque<T,N>::ConstIterator
Deque<T,N>::ConstIterator::operator-(int off)
{
  ConstIterator i = *this;
  i -= off;
  return i;
}

template<class T, int N>
inline bool
Deque<T,N>::ConstIterator::operator==(const ConstIterator& iter) const
{ return TheDeque && TheDeque == iter.TheDeque && Index == iter.Index; }

template<class T, int N>
inline bool
Deque<T,N>::ConstIterator::operator!=(const ConstIterator& iter) const
{ return !operator==(iter); }

template<class T, int N>
inline bool
Deque<T,N>::ConstIterator::operator<(const ConstIterator& iter) const
{ return Index < iter.Index; }

template<class T, int N>
inline bool
Deque<T,N>::ConstIterator::operator>(const ConstIterator& iter) const
{ return Index > iter.Index; }

template<class T, int N>
inline bool
Deque<T,N>::ConstIterator::operator<=(const ConstIterator& iter) const
{ return !operator>(iter); }

template<class T, int N>
inline bool
Deque<T,N>::ConstIterator::operator>=(const ConstIterator& iter) const
{ return !operator<(iter); }

template<class T, int N>
inline const T&
Deque<T,N>::ConstIterator::operator[](int index) const
{
  index += Index;

  int max = TheDeque->GetCapacity();
  if (index < 0)
    index += max + 1;
  else if (index > max)
    index -= max + 1;

  return TheDeque->Get(Index);
}

template<class T, int N>
inline const T&
Deque<T,N>::ConstIterator::operator*() const 
{ return TheDeque->Get(Index); }

template<class T, int N>
inline const T*
Deque<T,N>::ConstIterator::operator->() const
{ return &TheDeque->Get(Index); }

///////////////////////////////////////////////
// class Deque<class T, int N>::Iterator
//

// Protected functions
//
template<class T, int N>
inline
Deque<T,N>::Iterator::Iterator(Deque<T,N>* deque, int index)
  : TheDeque(deque), Index(index)
{}

// Public functions
//
template<class T, int N>
inline
Deque<T,N>::Iterator::Iterator()
  : TheDeque(NULL)
{}

template<class T, int N>
inline
Deque<T,N>::Iterator::Iterator(Deque<T,N>& deque)
{ operator=(deque.Begin()); }

template<class T, int N>
inline
Deque<T,N>::Iterator::Iterator(const Iterator& iter)
{ operator=(iter); }

template<class T, int N>
inline
Deque<T,N>::Iterator::~Iterator()
{}

template<class T, int N>
inline const typename Deque<T,N>::Iterator&
Deque<T,N>::Iterator::operator++()
{
  if (Index != TheDeque->_End) {
    ++Index;

    if (Index > TheDeque->GetCapacity())
      Index = 0;
  }

  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::Iterator
Deque<T,N>::Iterator::operator++(int)
{
  Iterator i = *this;
  operator++();
  return i;
}

template<class T, int N>
inline const typename Deque<T,N>::Iterator&
Deque<T,N>::Iterator::operator--()
{
  if (Index != TheDeque->_Begin) {
    if (--Index < 0)
      Index = TheDeque->GetCapacity();
  }

  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::Iterator
Deque<T,N>::Iterator::operator--(int)
{
  Iterator i = *this;
  operator--();
  return i;
}

template<class T, int N>
inline const typename Deque<T,N>::Iterator&
Deque<T,N>::Iterator::operator=(const Iterator& iter)
{
  Index = iter.Index;
  TheDeque = iter.TheDeque;
  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::Iterator&
Deque<T,N>::Iterator::operator+=(int off)
{
  Index += off;

  int max = TheDeque->GetCapacity();
  if (Index > max)
    Index -= max + 1;

  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::Iterator&
Deque<T,N>::Iterator::operator-=(int off)
{
  Index -= off;

  if (Index < 0)
    Index += TheDeque->GetCapacity() + 1;

  return *this;
}

template<class T, int N>
inline const typename Deque<T,N>::Iterator
Deque<T,N>::Iterator::operator+(int off)
{
  Iterator i = *this;
  i += off;
  return i;
}

template<class T, int N>
inline const typename Deque<T,N>::Iterator
Deque<T,N>::Iterator::operator-(int off)
{
  Iterator i = *this;
  i -= off;
  return i;
}

template<class T, int N>
inline bool
Deque<T,N>::Iterator::operator==(const Iterator& iter) const
{ return TheDeque && TheDeque == iter.TheDeque && Index == iter.Index; }

template<class T, int N>
inline bool
Deque<T,N>::Iterator::operator!=(const Iterator& iter) const
{ return !operator==(iter); }

template<class T, int N>
inline bool
Deque<T,N>::Iterator::operator<(const Iterator& iter) const
{ return Index < iter.Index; }

template<class T, int N>
inline bool
Deque<T,N>::Iterator::operator>(const Iterator& iter) const
{ return Index > iter.Index; }

template<class T, int N>
inline bool
Deque<T,N>::Iterator::operator<=(const Iterator& iter) const
{ return !operator>(iter); }

template<class T, int N>
inline bool
Deque<T,N>::Iterator::operator>=(const Iterator& iter) const
{ return !operator<(iter); }

template<class T, int N>
inline T&
Deque<T,N>::Iterator::operator[](int index) const
{
  index += Index;

  int max = TheDeque->GetCapacity();
  if (index < 0)
    index += max + 1;
  else if (index > max)
    index -= max + 1;

  return TheDeque->Get(Index);
}

template<class T, int N>
inline T&
Deque<T,N>::Iterator::operator*() const
{ return TheDeque->Get(Index); }

template<class T, int N>
inline T*
Deque<T,N>::Iterator::operator->() const
{ return &TheDeque->Get(Index); }

///////////////////////////////////////////
// class Deque<class T, int N>
//

// Protected functions:
//

// Alloc does an in-place new of the data.  Basically, it says,
// construct an object T at the location I specify and don't call
// malloc to get memory since I've provided the location
template <class T, int N>
inline void
Deque<T,N>::Alloc(int i, const T& data)
{ new ((void *)&Data[i]) T(data); }

// Free destroys the object in place without calling delete (which
// would normally free memory).
template <class T, int N>
inline void
Deque<T,N>::Free(int i)
{ (*(T *)&Data[i]).~T(); }

// Since the data storage is actually allocated as a struct of a long
// array to just get the data, we need to cast it to what we really
// are using.  By returning a reference, we can just use it in place
// of a normal array access.
template <class T, int N>
inline T&
Deque<T,N>::Get(int index)
{ return *(T*)&Data[index]; }

// Const version of above.
template <class T, int N>
inline const T&
Deque<T,N>::Get(int index) const
{ return *(T*)&Data[index]; }

// Increment the index and wrap if we've hit the end of the array
template <class T, int N>
inline void
Deque<T,N>::Inc(int &i)
{
  if (++i > GetCapacity())
    i = 0;
}

// Decrement the index and wrap if we'v gone past the beginning of the array
template <class T, int N>
inline void
Deque<T,N>::Dec(int &i)
{
  if (--i < 0)
    i = GetCapacity();
}

// Allocate a new element at the beginning and insert the data.
template <class T, int N>
inline void
Deque<T,N>::_PushFront(const T& data)
{
  ++Size;
  Dec(_Begin);
  Alloc(_Begin, data);
}

// Allocate a new element at the end and insert the data.
template <class T, int N>
inline void
Deque<T,N>::_PushBack(const T& data)
{
  ++Size;
  Alloc(_End, data);
  Inc(_End);
}

// Remove and free the element at the beginning
template <class T, int N>
inline void
Deque<T,N>::_PopFront()
{
  --Size;
  Free(_Begin);
  Inc(_Begin);
}

// Remove and free the element at the end
template <class T, int N>
inline void
Deque<T,N>::_PopBack()
{
  --Size;
  Dec(_End);
  Free(_End);
}

// Constructors:
//
template <class T, int N>
inline
Deque<T,N>::Deque()
{
  Size = 0;
  _Begin = 0;
  _End = 0;
}

template <class T, int N>
inline
Deque<T,N>::Deque(const Deque<T,N>& deque)
{ operator=(deque); }

// Returns true if the storage space is full
template <class T, int N>
inline bool
Deque<T,N>::IsFull() const
{ return (GetOccupancy() == GetCapacity()); }

// Returns true if the storage space is empty
template <class T, int N>
inline bool
Deque<T,N>::IsEmpty() const
{ return (GetOccupancy() == 0); }

// Returns the current Size of the storages space
template <class T, int N>
inline int
Deque<T,N>::GetOccupancy() const
{ return Size; }

// Returns the capacity of the storage space
template <class T, int N>
inline int
Deque<T,N>::GetCapacity() const
{ return N; }

// Returns the free space in number of elements for the
// storage space
template <class T, int N>
inline int
Deque<T,N>::GetFreeSpace() const
{ return GetCapacity() - GetOccupancy(); }


template <class T, int N>
inline typename Deque<T,N>::Iterator
Deque<T,N>::Begin()
{ return Iterator(this, _Begin); }

template <class T, int N>
inline typename Deque<T,N>::Iterator
Deque<T,N>::End()
{ return Iterator(this, _End); }

template <class T, int N>
inline typename Deque<T,N>::ConstIterator
Deque<T,N>::Begin() const
{ return ConstIterator(this, _Begin); }

template <class T, int N>
inline typename Deque<T,N>::ConstIterator
Deque<T,N>::End() const
{ return ConstIterator(this, _End); }

// Access functions
//
template <class T, int N>
inline T&
Deque<T,N>::Front()
{ return *Begin(); }

template <class T, int N>
inline T&
Deque<T,N>::Back()
{ return *--End(); }

template <class T, int N>
inline const T&
Deque<T,N>::Front() const
{ return *Begin(); }

template <class T, int N>
inline const T&
Deque<T,N>::Back() const
{ return *--End(); }

template <class T, int N>
inline bool
Deque<T,N>::PushFront(const T& data)
{
  if (IsFull())
    return false;

  _PushFront(data);
  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::PushBack(const T& data)
{
  if (IsFull())
    return false;

  _PushBack(data);
  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::PopFront(T& data)
{
  if (IsEmpty())
    return false;

  --Size;
  data = Get(_Begin);
  Free(_Begin);
  Inc(_Begin);

  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::PopBack(T& data)
{
  if (IsEmpty())
    return false;

  --Size;
  Dec(_End);
  data = Get(_End);
  Free(_End);

  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::PopFront()
{
  if (IsEmpty())
    return false;

  _PopFront();
  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::PopBack()
{
  if (IsEmpty())
    return false;

  _PopBack();
  return true;
}

 
template <class T, int N>
inline typename Deque<T,N>::Iterator
Deque<T,N>::Insert(Iterator pos, const T& data)
{
  if (IsFull())
    return End();

  if (pos == Begin()) {
    _PushFront(data);
    return Begin();
  } else if (pos == End()) {
    int index = _End;
    _PushBack(data);
    return Iterator(this, index);
  } else {
    // Insertion in the middle is not yet supported
    ASSERTX(false);
  }

  return End();
}

template <class T, int N>
inline bool
Deque<T,N>::Insert(Iterator pos, const T& data, int n)
{
  if (n > GetFreeSpace())
    return false;

  while (n-- > 0)
    Insert(pos, data);

  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::Insert(Iterator pos, Iterator begin, Iterator end)
{
  while (begin != end) {
    if (IsFull())
      return false;
    Insert(pos, *begin);
    ++begin;
  }
  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::Remove(Iterator pos)
{
  if (IsEmpty())
    return false;

  if (pos == Begin()) {
    PopFront();
  } else if (pos == --End()) {
    PopBack();
  } else {
    // Removal from the middle is not yet supported
    ASSERTX(false);
  }

  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::Remove(Iterator begin, Iterator end)
{
  while (begin != end) {
    if (!Remove(begin))
      return false;
    ++begin;
  }
  return true;
}

template <class T, int N>
inline const Deque<T,N>&
Deque<T,N>::operator=(const Deque<T,N>& deque)
{
  Size = 0;
  _Begin = 0;
  _End = 0;
  Insert(begin(), deque.begin(), deque.End());

  return *this;
}

template <class T, int N>
inline bool
Deque<T,N>::operator==(const Deque<T,N>& deque) const
{
  ConstIterator i = Begin();
  ConstIterator j = deque.Begin();

  if (GetOccupancy() != deque.GetOccupancy())
    return false;

  while(i != End()) {
    if (i != j)
      return false;

    ++i;
    ++j;
  }

  return true;
}

template <class T, int N>
inline bool
Deque<T,N>::operator!=(const Deque<T,N>& deque) const
{ return !operator==(deque); }
#endif //_DEQUE_
