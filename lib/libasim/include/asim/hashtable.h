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

#ifndef _ASIM_HASHTABLE_
#define _ASIM_HASHTABLE_

// generic
#include <math.h>
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"

///////////////////////////////////
// Some default Hashing Functions
//
inline unsigned Hash(const void* addr)
{
  union {
    const void *vaddr;
    UINT64 uladdr;
  } addr_u;

  // we need to make sure all bits are initialized even 
  // if sizeof(void*) < sizeof(UINT64)
  // the following condition only involves constants known at compile time
  if (sizeof(addr_u.vaddr) < sizeof(addr_u.uladdr)) {
    addr_u.uladdr = 0;
  }
  addr_u.vaddr = addr;

  int hash = (((addr_u.uladdr >> 14) ^ ((addr_u.uladdr >> 2) & 0xffff))) & 0x7FFFFFFF;
  
  return hash;
}

//
// Bobbie: Just return address since this is the page
// tag and is probably fairly unique. 
inline unsigned Hash(const UINT64 addr)
{
  return addr;
}

inline unsigned Hash(const char s[])
{
  int hash = 5381;
  
  while (*s)
    hash = ((hash << 5) + hash) + *s++;
  
  return hash;

}

/////////////////////////////
// class HashTable<K,D,N,F>
//
template<class K, class D, int N, unsigned (*F)(K) = Hash>
class HashTable
{
private:
  void Copy(const HashTable<K,D,N,F>& hash);
  void Probe(int& index, int& i) const;
  int FindIndex(const K& key) const;
  const K Dummy;

//protected:
public:
  enum StatusType { Free, Valid, Deleted };
  struct table
  {
    table() : Status(Free) {}
    StatusType Status;
    K Key;
    D Data;
  } Table[N];

  int MaxSize;
  int Size;

public:
  // Forward declarations
  class Iterator;
  class ConstIterator;

  class ConstIterator
  {
  private:
    friend class HashTable<K,D,N,F>;

  protected:
    const HashTable<K,D,N,F> *Hash;
    int Index;

    ConstIterator(const HashTable<K,D,N,F> *h, int i);

  public:
    ConstIterator();
    ConstIterator(const ConstIterator& iter);
    ConstIterator(const Iterator& iter);
    ~ConstIterator() {}

    const ConstIterator& operator++();
    const ConstIterator operator++(int);
    const ConstIterator& operator--();
    const ConstIterator operator--(int);
    
    const ConstIterator& operator=(const ConstIterator& iter);
    const ConstIterator& operator=(const Iterator& iter);

    bool operator==(const ConstIterator& iter) const;
    bool operator!=(const ConstIterator& iter) const;

    const D& operator*() const;
    const D* operator->() const;

    const K& GetKey() const;
  };

  class Iterator
  {
  private:
    friend class HashTable<K,D,N,F>;

  protected:
    HashTable<K,D,N,F> *Hash;
    int Index;

    Iterator(HashTable<K,D,N,F> *h, int i);

  public:
    Iterator();
    Iterator(const Iterator& iter);
    ~Iterator() {}

    const Iterator& operator++();
    const Iterator operator++(int);
    const Iterator& operator--();
    const Iterator operator--(int);
    
    const Iterator& operator=(const Iterator& iter);

    bool operator==(const Iterator& iter) const;
    bool operator!=(const Iterator& iter) const;

    D& operator*();
    D* operator->();

    const K& GetKey() const;
  };

public:
  HashTable();
  HashTable(const HashTable<K,D,N,F>& hash);
  ~HashTable();

  static int PrevPrime(int n);

  bool IsFull() const;
  bool IsEmpty() const;
  int GetSize() const;
  int GetMaxSize() const;

  ConstIterator Begin() const;
  Iterator Begin();
  ConstIterator End() const;
  Iterator End();

  ConstIterator Find(const K& key) const;
  Iterator Find(const K& key);
  Iterator Insert(const K& key, const D& data);
  Iterator Remove(Iterator& iter);

  void Dump(ostream& out = cout);
  void DumpData(ostream& out = cout);

  const HashTable<K,D,N,F>&
  operator=(const HashTable<K,D,N,F>& hash);
};

/////////////////////////////////////////////
//  class HashTable<K,D,N,F>::ConstIterator
//
template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::ConstIterator::ConstIterator()
  : Hash(NULL), Index(0)
{}

template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::ConstIterator::ConstIterator(const HashTable<K,D,N,F> *h, int i)
  : Hash(h), Index(i)
{}

// Copy constructor that will copy the position of another ConstIterator
template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::ConstIterator::ConstIterator(const ConstIterator& iter)
{ operator=(iter); }

// This copy constructor is used to initialize a constant iterator
// from a mutable iterator.  It can also be used to explicitly or
// implicitly cast from an Iterator to a ConstIterator.
template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::ConstIterator::ConstIterator(const Iterator& iter)
{ operator=(iter); }

//
// ConstIterator::operator++()
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::ConstIterator&
HashTable<K,D,N,F>::ConstIterator::operator++()
{
do
    {
    Index++;
    }
while((Hash->Table[Index].Status != Valid)  && Index < N);

ASSERTX(Index <= N);  
// if you do (foo.End()++ == foo.End())  then you are asking for trouble 

return *this;
}

//
// ConstIterator::operator++(int)
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::ConstIterator
HashTable<K,D,N,F>::ConstIterator::operator++(int)
{
  ASSERT(false, "Not Implemented.  Talk to Nathan Binkert.\n");
}

//
// ConstIterator::operator--()
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::ConstIterator&
HashTable<K,D,N,F>::ConstIterator::operator--()
{
  ASSERT(false, "Not Implemented.  Talk to Nathan Binkert.\n");
}

//
// ConstIterator::operator--(int)
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::ConstIterator
HashTable<K,D,N,F>::ConstIterator::operator--(int)
{
  ASSERT(false, "Not Implemented.  Talk to Nathan Binkert.\n");
}

// Copy constructor that allows us to copy another ConstIterator.  The 
// current iterator need not point to anything for this to be a valid
// operation.
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::ConstIterator&
HashTable<K,D,N,F>::ConstIterator::operator=(const ConstIterator& iter)
{
  // Check for assignment to self.
  if (this == &iter)
    return *this;

  // Copy the acutal iterator information.  If it's junk, then the new 
  // iterator will be junk too.
  Hash = iter.Hash;
  Index = iter.Index;

  return *this;
}

// Copy constructor that allows us to copy another ConstIterator.  The 
// current iterator need not point to anything for this to be a valid
// operation.
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::ConstIterator&
HashTable<K,D,N,F>::ConstIterator::operator=(const Iterator& iter)
{
  // Copy the acutal iterator information.  If it's junk, then the new 
  // iterator will be junk too.
  Hash = iter.Hash;
  Index = iter.Index;

  return *this;
}

// Compares the equality of two iterators.
template<class K, class D, int N, unsigned (*F)(K)>
inline bool
HashTable<K,D,N,F>::ConstIterator::operator==(const ConstIterator& iter) const
{ return Index == iter.Index && Hash == iter.Hash; }

// Compares the equality of two iterators.
template<class K, class D, int N, unsigned (*F)(K)>
inline bool
HashTable<K,D,N,F>::ConstIterator::operator!=(const ConstIterator& iter) const
{ return !operator==(iter); }

// This operator is used to dereference the iterator.  There is no
// check to guarantee that the iterator points to valid data.
template<class K, class D, int N, unsigned (*F)(K)>
inline const D&
HashTable<K,D,N,F>::ConstIterator::operator*() const
{ return Hash->Table[Index].Data; }

// This operator is used to point to a member of the data stored at
// the current node.  If the type T is a class, this behaves as if the
// iterator were just like any other pointer to the class.  If the
// type T happens to be a pointer, then this fucntion does not really
// make much sense.
template<class K, class D, int N, unsigned (*F)(K)>
inline const D*
HashTable<K,D,N,F>::ConstIterator::operator->() const
{ return &Hash->Table[Index].Data; }

// This function will return the Key at the current location.
// Nonstandard addition to iterator functionality, but useful.
template<class K, class D, int N, unsigned (*F)(K)>
inline const K&
HashTable<K,D,N,F>::ConstIterator::GetKey() const
{ return Hash->Table[Index].Key; }

////////////////////////////////////////
//  class HashTable<K,D,N,F>::Iterator
//
template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::Iterator::Iterator()
  : Hash(NULL), Index(0)
{}

template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::Iterator::Iterator(HashTable<K,D,N,F> *h, int i)
  : Hash(h), Index(i)
{}

// Copy constructor that will copy the position of another Iterator
template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::Iterator::Iterator(const Iterator& iter)
{ operator=(iter); }

//
// Iterator::operator++()
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::Iterator&
HashTable<K,D,N,F>::Iterator::operator++()
{
    do
    {
        Index++;
    }
    while((Hash->Table[Index].Status != Valid)  && Index < N);
    
    ASSERTX(Index <= N);  
// if you do (foo.End()++ == foo.End())  then you are asking for trouble 
    
    return *this;
}

//
// Iterator::operator++(int)
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::Iterator
HashTable<K,D,N,F>::Iterator::operator++(int)
{
  ASSERT(false, "Not Implemented.  Talk to Nathan Binkert.\n");
}

//
// Iterator::operator--()
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::Iterator&
HashTable<K,D,N,F>::Iterator::operator--()
{
  ASSERT(false, "Not Implemented.  Talk to Nathan Binkert.\n");
}

//
// Iterator::operator--(int)
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::Iterator
HashTable<K,D,N,F>::Iterator::operator--(int)
{
  ASSERT(false, "Not Implemented.  Talk to Nathan Binkert.\n");
}

// Copy constructor that allows us to copy another Iterator.  The
// current iterator need not point to anything for this to be a valid
// operation.
template<class K, class D, int N, unsigned (*F)(K)>
inline const typename HashTable<K,D,N,F>::Iterator&
HashTable<K,D,N,F>::Iterator::operator=(const Iterator& iter)
{
  // Check for assignment to self.
  if (this == &iter)
    return *this;

  // Copy the acutal iterator information.  If it's junk, then the new 
  // iterator will be junk too.
  Hash = iter.Hash;
  Index = iter.Index;

  return *this;
}

// Compares the equality of two iterators.
template<class K, class D, int N, unsigned (*F)(K)>
inline bool
HashTable<K,D,N,F>::Iterator::operator==(const Iterator& iter) const
{ return Index == iter.Index && Hash == iter.Hash; }

// Compares the inequality of two iterators.
template<class K, class D, int N, unsigned (*F)(K)>
inline bool
HashTable<K,D,N,F>::Iterator::operator!=(const Iterator& iter) const
{ return !operator==(iter); }

// This operator is used to dereference the iterator.  There is no
// check to guarantee that the iterator points to valid data.
template<class K, class D, int N, unsigned (*F)(K)>
inline D&
HashTable<K,D,N,F>::Iterator::operator*()
{ return Hash->Table[Index].Data; }

// This operator is used to point to a member of the data stored at
// the current node.  If the type T is a class, this behaves as if the 
// iterator were just like any other pointer to the class.  If the
// type T happens to be a pointer, then this fucntion does not really
// make much sense.
template<class K, class D, int N, unsigned (*F)(K)>
inline D*
HashTable<K,D,N,F>::Iterator::operator->()
{ return &Hash->Table[Index].Data; }

// This function will return the Key at the current location.
// Nonstandard addition to iterator functionality, but useful.
template<class K, class D, int N, unsigned (*F)(K)>
inline const K&
HashTable<K,D,N,F>::Iterator::GetKey() const
{ return Hash->Table[Index].Key; }

///////////////////////////////
//  Constructors & Destructor
//
template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::HashTable()
   : Dummy(K()), Size(0)
{
  MaxSize = N;
//  MaxSize = PrevPrime(N);
}

template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::HashTable(const HashTable<K,D,N,F>& hash)
   : Size(0), Dummy(K())
{
  MaxSize = N;
//  MaxSize = PrevPrime(N);
  Copy(hash);
}

template<class K, class D, int N, unsigned (*F)(K)>
inline
HashTable<K,D,N,F>::~HashTable()
{}

/////////////////////////
// Protected Functions
//
template<class K, class D, int N, unsigned (*F)(K)>
inline void
HashTable<K,D,N,F>::Copy(const HashTable<K,D,N,F>& hash)
{
  this->Size.Size;
  for (int count = 0; count < GetMaxSize(); count++) {
    bool status = hash.Table[count].Status;
    Table[count].Status = status;
    if (status == Valid) {
      Table[count].Key = hash.Table[count].Key;
      Table[count].Data = hash.Table[count].Data;
    }
  }
}

// We are doing open addressing.  To cut down on locality, we will
// use quadratic probing
template<class K, class D, int N, unsigned (*F)(K)>
inline void
HashTable<K,D,N,F>::Probe(int& index, int& i) const
{
  ASSERT(i < GetMaxSize(), "HashTable is full, cannot probe.\n");

//  index += 2 * ++i - 1;
  ++index; ++i;

// Implement the mod function.
  if (index >= GetMaxSize())
    index -= GetMaxSize();

// index could potentially equal 3 * GetMaxSize() - 2
  if (index >= GetMaxSize())
    index -= GetMaxSize();
}

// This function searches the hash for the key.  If the key is not
// found, it will return the index for the first deleted slot.  If
// there are no deleted slots, it will return the address of the first
// free slot.  If there are no free slots, the function will return
// -1.
template<class K, class D, int N, unsigned (*F)(K)>
inline int
HashTable<K,D,N,F>::FindIndex(const K& key) const
{
  int i = 0;
  int index = F(key) % GetMaxSize();

  // If the hash table is full, this is a special case.  We should
  // search the hash table for the element.  If we have searched the
  // table GetMaxSize() number of times, then we know that it is not
  // found.  Otherwise, return the index where the key can be found.
  if (IsFull()) {
    int count = 1;
    while (Table[index].Key != key) {
      ASSERTX(Table[index].Status == Valid);
      if (i == GetMaxSize())
	return -1;
      
      Probe(index, i);
    }

    return index;
  }

  while (Table[index].Status == Valid) {
    if (Table[index].Key == key)
      return index;

    Probe(index, i);
  }

  if (Table[index].Status == Free)
    return index;

  ASSERTX(Table[index].Status == Deleted);

  int del = index;
  Probe(index, i);
  while (Table[index].Status != Free) {
    if (Table[index].Status == Valid && Table[index].Key == key)
      return index;

    if (i == GetMaxSize())
      return del;

    Probe(index, i);
  }

  ASSERTX(Table[index].Status == Free);

  return del;
}

/////////////////////
// Public Functions
//
template<class K, class D, int N, unsigned (*F)(K)>
inline int
HashTable<K,D,N,F>::PrevPrime(int n)
{
  int i, decr;
  bool is_prime;
  
  if (!(n & 1))
    --n;

  if ((decr = n % 3) == 0) {
    n -= 2;
    decr = 2;
  }
  else if (decr == 1)
    decr = 4;

  for(;;) {
    is_prime = true;
    for (i = 5; (i*i) <= n; i += 6)
      if ( ((n % i) == 0 ) || ((n % (i + 2)) == 0) ) {
      is_prime = false;
      break;
    }
    if (is_prime)
      return n;
    n -= decr;
    /*  Toggle between 2 and 4
     */
    decr = 6 - decr;
  }
}

template<class K, class D, int N, unsigned (*F)(K)>
inline bool
HashTable<K,D,N,F>::IsFull() const
{ return GetSize() == GetMaxSize(); }

template<class K, class D, int N, unsigned (*F)(K)>
inline bool
HashTable<K,D,N,F>::IsEmpty() const
{ return GetSize() == 0; }

template<class K, class D, int N, unsigned (*F)(K)>
inline int
HashTable<K,D,N,F>::GetSize() const
{ return Size; }

template<class K, class D, int N, unsigned (*F)(K)>
inline int
HashTable<K,D,N,F>::GetMaxSize() const
{ return MaxSize; }

template<class K, class D, int N, unsigned (*F)(K)>
inline typename HashTable<K,D,N,F>::ConstIterator
HashTable<K,D,N,F>::Begin() const
{ 
// Begin should return the first VALID entry or else End()
int i=0;
while((Table[i].Status != Valid)  && i < N)
    {
    i++;
    }
return ConstIterator(this, i);
}

template<class K, class D, int N, unsigned (*F)(K)>
inline typename HashTable<K,D,N,F>::Iterator
HashTable<K,D,N,F>::Begin()
{ 
// Begin should return the first VALID entry or else End()
int i=0;
while((Table[i].Status != Valid)  && i < N)
    {
    i++;
    }
return Iterator(this, i);
 }

template<class K, class D, int N, unsigned (*F)(K)>
inline typename HashTable<K,D,N,F>::ConstIterator
HashTable<K,D,N,F>::End() const
{ return ConstIterator(this, N); }

template<class K, class D, int N, unsigned (*F)(K)>
inline typename HashTable<K,D,N,F>::Iterator
HashTable<K,D,N,F>::End()
{ return Iterator(this, N); }

template<class K, class D, int N, unsigned (*F)(K)>
inline typename HashTable<K,D,N,F>::ConstIterator
HashTable<K,D,N,F>::Find(const K& key) const
{
  int index = FindIndex(key);
  if (index < 0 || Table[index].Status != Valid)
    return End();

  return ConstIterator(this, index);
}

template<class K, class D, int N, unsigned (*F)(K)>
inline typename HashTable<K,D,N,F>::Iterator
HashTable<K,D,N,F>::Find(const K& key)
{
  int index = FindIndex(key);
  if (index < 0 || Table[index].Status != Valid)
    return End();

  return Iterator(this, index);
}

template<class K, class D, int N, unsigned (*F)(K)>
inline typename HashTable<K,D,N,F>::Iterator
HashTable<K,D,N,F>::Insert(const K& key, const D& data)
{
  int index = FindIndex(key);
  if (index < 0) {
      return End();
  }

  if (Table[index].Status == Valid) {
      return End();
  }

  Size++;
  Table[index].Status = Valid;
  Table[index].Key = key;
  Table[index].Data = data;

  return Iterator(this, index);
}

template<class K, class D, int N, unsigned (*F)(K)>
inline typename HashTable<K,D,N,F>::Iterator
HashTable<K,D,N,F>::Remove(HashTable<K,D,N,F>::Iterator &iter)
{
  if (iter == End())
    return End();

  Table[iter.Index].Status = Deleted;
  Table[iter.Index].Key = Dummy;
  Size--;
  return iter;
}

template<class K, class D, int N, unsigned (*F)(K)>
inline const HashTable<K,D,N,F>&
HashTable<K,D,N,F>::operator=(const HashTable<K,D,N,F> &hash)
{
  Copy(hash);
  return *this;
}

template<class K, class D, int N, unsigned (*F)(K)>
inline void
HashTable<K,D,N,F>::Dump(ostream& out)
{
  for (int count = 0; count < GetMaxSize(); count++) {
    out << "Table[" << count << "] = { " << Table[count].Status;
    if (Table[count].Status == Valid)
      out << ", " << Table[count].Key << ", " << Table[count].Data;
    out << "};\n";
  }
}

template<class K, class D, int N, unsigned (*F)(K)>
inline void
HashTable<K,D,N,F>::DumpData(ostream& out)
{
  for (int count = 0; count < GetMaxSize(); count++) {
    if (Table[count].Status == Valid)
      out << Table[count].Data;
  }
}
#endif //_ASIM_HASHTABLE_
