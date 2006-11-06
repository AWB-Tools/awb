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

#ifndef _ARRAYLIST_
#define _ARRAYLIST_

// generic
#include <new>

/////////////////////////////////////////////////////////////////////////
// class ArrayList<class T, int N>
//
// This class is a templatized implementation of an array based linked
// list. It is designed to be used when the list's maximum size can be
// known at compile time.  It has an interface similar to what you
// might see in the Standard Template Library(STL) complete with
// iterators for access to elements. All functions provided by this
// list are O(1) time.  This is the case of a general list, but by
// having all nodes be allocated from a predefined array, we hope to
// get better cache behavior since node elements will not be scattered 
// about the heap.
//
template<class T, int N>
class ArrayList
{
private:
  // We're all friends here.
  friend class ConstIterator;
  friend class Iterator;
  friend class Node;

protected:
  ////////////////////////////////
  // struct ArrayList<T,N>::Node
  //
  // The node structure is the basic element of the linked list that is
  // managed.  The node contains pointers to the next element in the
  // list as well as the previous element in the list.  This allows us
  // to use a bidirectional iterator.
  //
  struct Node
  {
  public:
    T Data;
    Node* Prev;
    Node* Next;
  };

  union Raw
  {
    Raw* Next;
    char Data[sizeof(Node)];
  };

  const T Dummy;

  //////////////
  // Pre-allocated array that contains all of the elements in the list.
  // This reduces calls to new and delete and increases locality to
  // hopefully increase performance in the cache.  Since we have a
  // preallocated array, we also maintain a freelist.  The Next pointer
  // in the Node structure serves a dual purpose.  When a Node is in the 
  // list, the Next pointer points to the next element, but when a Node
  // is not in the list, the Next pointer is used to point to the next
  // free element in the free list.  The Prev pointer also serves a dual 
  // purpose.  For debugging purposes, the next pointer is set to NULL
  // when the node is on the free list and the prev pointer is checked
  // when iteration is performed to prevent the use of an iterator that
  // is not valid. i.e. has been freed.
  //
  Raw Array[N+1];
  Raw* FreeList;

  //////////////
  // Pointers to the first and last elements of the list.
  // NOTE: _End actually points the the element one past the end of the
  //       list!
  // Size keeps track of the current number of elements in the list.
  //
  Node* _Begin;
  Node* _End;
  int Size;

public:
  // Forward declarations.
  class Iterator;
  class ConstIterator;

  ////////////////////////////////////////
  // class ArrayList<T,N>::ConstIterator
  //
  // This class is the Constant Iterator for the ArrayList.  The const 
  // iterator is used when traversing a const ArrayList.  It will
  // allow the user to look at the elements in the list, but the user
  // will be unable to modify any of the values in the list and unable
  // to modify the list itself (i.e. will not be able to insert or
  // remove).
  //
  class ConstIterator
  {
  private:
    // The list must be able to access non public functions.
    friend class ArrayList<T,N>;

  protected:
    // We only need to store the current node that we point to.  We
    // could store the list as well, but that would only be necessary
    // for additional checking.  If we did store the list in addition
    // to the node, then when comparisons were made, we would be able
    // to determine whether the two iterators were from the same list.
    const Node* Curr;

    // The list class needs to be able to create iterators from
    // individual nodes, but normal accessors of the list should not
    // be able to do this.
    ConstIterator(Node* node);

  public:
    // Default constructor initializes the iterator.  When creating an
    // iterator with the default constructor, we are unable to call
    // any function other than operator=.  This is because the
    // iterator does not actually point to anything and is not
    // dereferencable.
    ConstIterator();

    // These two copy constructors are simple duplicate the position
    // of an iterator.  The second copy constructor can also be used
    // for implicit casting from an Iterator to a Const Iterator.
    ConstIterator(const ConstIterator& iter);
    ConstIterator(const Iterator& iter);

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

    // Changes what the iterator points to.
    const ConstIterator& operator=(const ConstIterator& iter);
    const ConstIterator& operator=(const Iterator& iter);

    // Simple comparison.
    bool operator==(const ConstIterator& iter) const;
    bool operator!=(const ConstIterator& iter) const;

    // operator* is used to dereference the iterator.  When the
    // iterator is dereferenced, what is returned is the data that is
    // at the node that the iterator is pointing to.
    // e.g.
    //     ArrayList<int, 20>::Iterator iter = list.Begin();
    //     int x = *iter;
    const T& operator*() const;

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
    //     ArrayList<my_struct*, 20>::Iterator iter = list.Begin();
    //     int i = iter->z;    // BAD!  iter-> returns a my_struct**
    //     int j = (*iter)->z  // OK!   iter* returns a my_struct*
    const T* operator->() const;
  };

  ////////////////////////////////////////
  // class ArrayList<T,N>::Iterator
  //
  // This class is the Mutable Iterator for the ArrayList.  The mutable
  // iterator is used when traversing a mutable ArrayList.  It will
  // allow the user to look at the elements in the list, as well as
  // modify the data in the list and modify the list itself by
  // insertion and removal.
  //
  // Look at the comments for ConstIterator since all of the functions 
  // are basically the same.
  //
  class Iterator
  {
  private:
    friend class ArrayList<T,N>;

  protected:
    Node* Curr;
    Iterator(Node* node);

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

    T& operator*() const;
    T* operator->() const;
  };

protected:
  // Initializes the list by setting up the free list and the _Begin
  // and _End pointers.
  void Init();

  // Used to manipulate the freelist by allocating and deallocating
  // space from teh preallocated array.
  Node* NewNode();
  void DeleteNode(Node* node);

  // Protected insertion and deletion functions that are provided to
  // remove common code from the public insertion and deletion
  // functions.  These functions provide no error checking since that
  // is left up to the public version of the function that calls it.
  Node* _Insert(Iterator pos, T data);
  Node* _Remove(Iterator pos);

public:
  // Default constructor and copy constructor.  Nothing special here.
  ArrayList();
  ArrayList(const ArrayList<T,N>& list);

  // Nothing needs to be done to destroy the list.
  ~ArrayList() {}

  // Simple functions that return statistics on use and capacity of
  // the array list.
  int GetOccupancy() const;
  int GetCapacity() const;
  int GetFreeSpace() const;
  bool IsFull() const;
  bool IsEmpty() const;

  // Returns the first dereferenceable element in the array list.  It
  // will point to a valid element as long as the array list is not
  // empty.  It is up to the user to make sure that the list is not
  // empty.
  T& Front();
  const T& Front() const;

  // Returns the last dereferenceable element in the array list.  It
  // will point to a valid element as long as the array list is not
  // empty.  It is up to the user to make sure that the list is not
  // empty.
  T& Back();
  const T& Back() const;

  // Returns an iterator that points to the first element in the array
  // list.  If the list is empty, this will be equal to End() and not
  // be dereferenceable. In other cases, it will be dereferenceable.
  Iterator Begin();
  ConstIterator Begin() const;

  // Returns an iterator that points to the end of the array list.
  // The element that it points to is actually just a place holder and
  // considered to be one beyond the end of the array list.  This is
  // somewhat like the null termination ('\0') that we would find at
  // the end of a C style string (char *).
  Iterator End();
  ConstIterator End() const;

  // Functions for insertion and removal of objects into and from the
  // array list.
  //

  // Inserts the data into a new element before the position pointed
  // to by pos.  If pos == Begin(), Begin will properly be updated.
  // The return value is an iterator that points to the newly inserted 
  // element.  End() is returned on a failure.
  Iterator Insert(Iterator pos, T data);

  // Inserts n copies of the data before the position pointed to by
  // pos.  Returns true on success and false on failure.  Failure can
  // occur by trying to insert beyond the capacity of the list.
  bool Insert(Iterator pos, T data, int n);
  
  // Inserts a range of elements before the position pointed to by
  // pos.  Begin is the first element in the range, and end is one
  // past the end. i.e. [begin, end) will be inserted before pos.
  // Returns true on success and false on failure.  Failure can occur
  // by trying to insert beyond the capacity of the list.
  bool Insert(Iterator pos, ConstIterator begin, ConstIterator end);

  // Remove the element pointed to by the iterator pos from the list.
  // The iterator pos is considered invalid if it has been removed
  // from the list and should be discarded.
  Iterator Remove(Iterator pos);

  // Removes all elements in the range [begin, end) from the list.
  // All iterators that point to elements in that range will be
  // invalidated an should be discarded.
  bool Remove(Iterator begin, Iterator end);


  // Copies the array list.  First all elements are removed from the
  // arraylist, then a memberwise copy is performed.
  const ArrayList<T,N>& operator=(const ArrayList<T,N>& list);

  // Simple memberwise comparisons of the arraylist.
  bool operator==(const ArrayList<T,N>& list);
  bool operator!=(const ArrayList<T,N>& list);
};

/////////////////////////////////////////
//  class ArrayList<T,N>::ConstIterator
//

/////////////////////////
// Protected Functions:

// This is a protected constructor.  It is used by the Array List
// class to create iterators from nodes.  Users of the class do not
// need to know anything about the structure of the linked list, so
// they do not need access to this function.
template<class T, int N>
inline
ArrayList<T,N>::ConstIterator::ConstIterator(Node* node)
  : Curr(node)
{}

//////////////////////
// Public Functions:

// Default constructor for the Iterator class.  This constructor
// can be used when the iterator is in a loop and the initial value
// will not be known.
template<class T, int N>
inline
ArrayList<T,N>::ConstIterator::ConstIterator()
  : Curr(NULL)
{}

// Copy constructor that will copy the position of a ConstIterator.
template<class T, int N>
inline
ArrayList<T,N>::ConstIterator::ConstIterator(const ConstIterator& iter)
{ operator=(iter); }

// The copy constructor will initialize a constant iterator from a
// mutable iterator.  This copy constructor can also be used for
// implicit and explicit casting from an Iterator to a ConstIterator.
template<class T, int N>
inline
ArrayList<T,N>::ConstIterator::ConstIterator(const Iterator& iter)
{ operator=(iter); }

// Standard pre-increment function. This operator advances the
// iterator to the next position in the container and returns an
// interator that points to the new position.  Behaves just like the
// standard pre-increment operator in C.
//
// The return value is const because the iterator returned is a
// temporary object and we do not want to allow the user to use a
// temporary object as if it were an iterator.  The user can still
// dereference that object though.
//
// For operator++() to function properly, the object must be
// dereferenceable.  If the iterator is at the End(), operator++()
// does not increment.
// 
template<class T, int N>
inline const typename ArrayList<T,N>::ConstIterator&
ArrayList<T,N>::ConstIterator::operator++()
{
  Curr = Curr->Next;
  return *this;
}

// Standard post-increment function.  This operator advances the
// iterator to the next position in the container and returns an
// iterator that points to the original position.  Behaves just like
// the standard post-increment operator in C.
//
// Similarly to operator++(), if the iterator is equal to End(), then
// the iterator is not incremented.
//
// Because of implementation constraints, post-increment can
// potentially perform less efficiently than pre-increment with some
// compilers.  So, to be safe, use ++iter; rather than iter++; If you
// don't care about the return value.
//
template<class T, int N>
inline const typename ArrayList<T,N>::ConstIterator
ArrayList<T,N>::ConstIterator::operator++(int)
{
  // Since we need to return an iterator to the previous element, we
  // just make a copy now and call the post-increment operator.  This
  // is where the potential performance penalty can be.  Though it is
  // not a huge penalty, a temporary object will be constructed to
  // store the value from before the increment, whereas there is no
  // need for this with the pre-increment operator.
  ConstIterator i = *this;
  operator++();
  return i;
}

// Standard pre-decrement.  This operator decrements the iterator to
// the previous position in the container and returns an Iterator
// which points to the new position.  It behaves just like the
// standard pre-decrement operator in C.
template<class T, int N>
inline const typename ArrayList<T,N>::ConstIterator&
ArrayList<T,N>::ConstIterator::operator--()
{
  Curr = Curr->Prev;
  return *this;
}

// Standard post-decrement.  This operator decrements the iterator to
// the previous position in the container and returns an Iterator
// which points to the original position.  It behaves just like the
// standard post-decrement operator in C.
template<class T, int N>
inline const typename ArrayList<T,N>::ConstIterator
ArrayList<T,N>::ConstIterator::operator--(int)
{
  // This is the same as post-increment.
  ConstIterator i = *this;
  operator--();
  return i;
}

// Copy constructor that allows us to copy another ConstIterator.  The 
// current iterator need not point to anything for this to be a valid
// operation.
template<class T, int N>
inline const typename ArrayList<T,N>::ConstIterator&
ArrayList<T,N>::ConstIterator::operator=(const ConstIterator& iter)
{
  // Check for assignment to self
  if (this == &iter)
    return *this;

  // The only data item that there is, is the current node pointer.
  // So, that is all that we need to copy.  If the iterator that is
  // being copied is garbage, and not dereferenceable, then the new
  // iterator will have the same property.
  Curr = iter.Curr;
  return *this;
}

// Copy constructor that allows us to copy an Iterator.  The 
// current iterator need not point to anything for this to be a valid
// operation.
template<class T, int N>
inline const typename ArrayList<T,N>::ConstIterator&
ArrayList<T,N>::ConstIterator::operator=(const Iterator& iter)
{
  // The only data item that there is, is the current node pointer.
  // So, that is all that we need to copy.  If the iterator that is
  // being copied is garbage, and not dereferenceable, then the new
  // iterator will have the same property.
  Curr = iter.Curr;
  return *this;
}

// Compares the equality of two iterators.  This does not guarantee
// that the two iterators are from the same list, but the assumption
// is that if the pointers are the same, then they are from the same
// list.  (You could probably trick this operator somehow, but that
// would be evil.)
template<class T, int N>
inline bool
ArrayList<T,N>::ConstIterator::operator==(const ConstIterator& iter) const
{ return Curr == iter.Curr; }

// Simply compares the inequality of two iterators.  See operator==.
template<class T, int N>
inline bool
ArrayList<T,N>::ConstIterator::operator!=(const ConstIterator& iter) const
{ return !operator==(iter); }

// This operator is used to dereference the iterator.  There is no
// check to guarantee that the iterator points to valid data.
template<class T, int N>
inline const T&
ArrayList<T,N>::ConstIterator::operator*() const
{ return Curr->Data; }

// This operator is used to point to a member of the data stored at
// the current node.  If the type T is a class, this behaves as if the 
// iterator were just like any other pointer to the class.  If the
// type T happens to be a pointer, then this fucntion does not really
// make much sense.
template<class T, int N>
inline const T*
ArrayList<T,N>::ConstIterator::operator->() const
{ return &Curr->Data; }

/////////////////////////////////////////
//  class ArrayList<T,N>::Iterator
//
template<class T, int N>
inline
ArrayList<T,N>::Iterator::Iterator(Node* node)
  : Curr(node)
{}

// Public Functions:
//
template<class T, int N>
inline
ArrayList<T,N>::Iterator::Iterator()
  : Curr(NULL)
{}

template<class T, int N>
inline
ArrayList<T,N>::Iterator::Iterator(const ArrayList<T,N>::Iterator& iter)
{ operator=(iter); }

template<class T, int N>
inline const typename ArrayList<T,N>::Iterator&
ArrayList<T,N>::Iterator::operator++()
{
  Curr = Curr->Next;
  return *this;
}

template<class T, int N>
inline const typename ArrayList<T,N>::Iterator
ArrayList<T,N>::Iterator::operator++(int)
{
  Iterator i = *this;
  operator++();
  return i;
}

template<class T, int N>
inline const typename ArrayList<T,N>::Iterator&
ArrayList<T,N>::Iterator::operator--()
{
  Curr = Curr->Prev;
  return *this;
}

template<class T, int N>
inline const typename ArrayList<T,N>::Iterator
ArrayList<T,N>::Iterator::operator--(int)
{
  Iterator i = *this;
  operator--();
  return i;
}

template<class T, int N>
inline const typename ArrayList<T,N>::Iterator&
ArrayList<T,N>::Iterator::operator=(const Iterator& iter)
{
  if (this == &iter)
    return *this;

  Curr = iter.Curr;
  return *this;
}

template<class T, int N>
inline bool
ArrayList<T,N>::Iterator::operator==(const Iterator& iter) const
{ return Curr == iter.Curr; }

template<class T, int N>
inline bool
ArrayList<T,N>::Iterator::operator!=(const Iterator& iter) const
{ return !operator==(iter);  }

template<class T, int N>
inline T&
ArrayList<T,N>::Iterator::operator*() const
{ return Curr->Data; }

template<class T, int N>
inline T*
ArrayList<T,N>::Iterator::operator->() const
{ return &Curr->Data; }

/////////////////////////////////////////
//  class ArrayList<T,N>
//

/////////////////////////
// Protected Functions:

// This function initializes the state of the ArrayList to a proper
// empty state.
template<class T, int N>
inline void
ArrayList<T,N>::Init()
{
  // Initially we are empty.  _Begin and _End point to the same thing, 
  // so there is nothing in the list.
  Size = 0;
  _Begin = _End = new((void*)&Array[0]) Node;

  // Initialize the freelist by putting every element in the Array on
  // the freelist.  (Except element 0 which has already been allocated 
  // as End().
  //
  // The freelist makes use of the Next pointer to store its poniter
  // information.  The freelist is maintained as a stack.
  FreeList = NULL;
  for (int count = 1; count <= GetCapacity(); count++) {
    Array[count].Next = FreeList;
    FreeList = &Array[count];
  }
}

// This function gets the first element off the top of the free list
// and returns a pointer to the new space.  If there is no more space
// on the freelist, then the function returns NULL to signify an error.
template<class T, int N>
inline typename ArrayList<T,N>::Node*
ArrayList<T,N>::NewNode()
{
  if (!FreeList)
    return NULL;

  Node* free = (Node*)FreeList;
  FreeList = FreeList->Next;

  return new((void*)free) Node;
}

// This function frees a Node which has been allocated from the
// freelist.  The node is simply added to the top of the stack.
template<class T, int N>
inline void
ArrayList<T,N>::DeleteNode(ArrayList<T,N>::Node* node)
{
  node->~Node();

  Raw* raw = (Raw*)node;
  raw->Next = FreeList;
  FreeList = raw;
}

// This is a protected function for performing an insert.  There is no
// error checking.  The error checking is left up to the public
// wrapper functions that provide various interfaces to Insert.  It
// simply creates a new node with the data specified and inserts that
// node BEFORE the node that the iterator points to.
// The function returns a pointer to the node that has been inserted.
template<class T, int N>
inline typename ArrayList<T,N>::Node*
ArrayList<T,N>::_Insert(Iterator pos, T data)
{
  // Get a new node from the freelist and assert that this operation
  // does not fail.  If the assertion is raised, then that means that
  // the freelist is full and we should not have tried to insert a new 
  // object.
  Node* node = NewNode();
  ASSERTX(node);

  // Put the data into the new node and fix up all pointers.  We must
  // also check to see if we are inserting before the beginning, in
  // which case, we must reset the pointer _Begin to point to the
  // newly inserted node.
  node->Data = data;
  node->Next = pos.Curr;

  if (pos.Curr == _Begin) {
    _Begin = node;
    node->Prev = node;
  } else {
    node->Prev = pos.Curr->Prev;
    pos.Curr->Prev->Next = node;
  }
  pos.Curr->Prev = node;

  // We are now bigger.
  ++Size;

  return node;
}

// Protected version of the remove function.  All error checking is
// supposed to happen in the public wrapper for this function, so
// there is no error checking.
// After this function completes, the iterator pos is no longer valid
// and should be discarded.
// Returns the node following the one that has been deleted.
template<class T, int N>
inline typename ArrayList<T,N>::Node*
ArrayList<T,N>::_Remove(Iterator pos)
{
  Node* curr = pos.Curr;
  Node* next = pos.Curr->Next;
  Node* prev = pos.Curr->Prev;

  curr->Data = Dummy;  

  // Patch up pointers to the list since we are removing a node.  Pay
  // special attention if we are at the beginning of the list.
  if (pos.Curr == _Begin) {
    _Begin = next;
    _Begin->Prev = _Begin;
  } else {
    prev->Next = next;
    next->Prev = prev;
  }

  // We are smaller now.
  --Size;

  // Return the node to the free list.
  DeleteNode(curr);

  return next;
}

/////////////////////
// Public Functions

// Default constructor that creates an empty list.
template<class T, int N>
inline
ArrayList<T,N>::ArrayList() : Dummy(T())
{ Init(); }

// Copy constructor.  Does a complete copy of all of the nodes in the
// list. (i.e. a "deep copy")
template<class T, int N>
inline
ArrayList<T,N>::ArrayList(const ArrayList<T,N>& list) : Dummy(T())
{ operator=(list); }

// Returns the number of elements in the list.
template<class T, int N>
inline int
ArrayList<T,N>::GetOccupancy() const
{ return Size; }

// Returns the maximum capacity of the list.
template<class T, int N>
inline int
ArrayList<T,N>::GetCapacity() const
{ return N; }

// Returns the number of elements that may be added to the list.
template<class T, int N>
inline int
ArrayList<T,N>::GetFreeSpace() const
{ return GetCapacity() - GetOccupancy(); }

// Returns true if the list is full.
template<class T, int N>
inline bool
ArrayList<T,N>::IsFull() const
{ return GetOccupancy() == GetCapacity(); }

// Returns true if the list is empty.
template<class T, int N>
inline bool
ArrayList<T,N>::IsEmpty() const
{ return GetOccupancy() == 0; }

// Returns the element at the front of the list.  It is up to the user 
// to ensure that this element will be dereferenceable.
template<class T, int N>
inline T&
ArrayList<T,N>::Front()
{ return *Begin(); }

// Returns the element at the front of the list.  It is up to the user 
// to ensure that this element will be dereferenceable.
template<class T, int N>
inline const T&
ArrayList<T,N>::Front() const
{ return *Begin(); }

// Returns the element at the back of the list.  It is up to the user 
// to ensure that this element will be dereferenceable.
template<class T, int N>
inline T&
ArrayList<T,N>::Back()
{ return *--End(); }

// Returns the element at the back of the list.  It is up to the user 
// to ensure that this element will be dereferenceable.
template<class T, int N>
inline const T&
ArrayList<T,N>::Back() const
{ return *--End(); }

// Returns a constant iterator that points to the beginning of the
// list.  Relies on an implicit cast from a Node* to an Iterator.
template<class T, int N>
inline typename ArrayList<T,N>::ConstIterator
ArrayList<T,N>::Begin() const
{ return _Begin; }

// Returns a constant iterator that points to the element ONE PAST the
// end of the list.  Relies on an implicit cast from a Node* to an
// Iterator.
template<class T, int N>
inline typename ArrayList<T,N>::ConstIterator
ArrayList<T,N>::End() const
{ return _End; }

// Returns a mutable iterator that points to the beginning of the
// list.  Relies on an implicit cast from a Node* to an Iterator.
template<class T, int N>
inline typename ArrayList<T,N>::Iterator
ArrayList<T,N>::Begin()
{ return _Begin; }

// Returns a mutable iterator that points to the element ONE PAST the
// end of the list.  Relies on an implicit cast from a Node* to an
// Iterator.
template<class T, int N>
inline typename ArrayList<T,N>::Iterator
ArrayList<T,N>::End()
{ return _End; }

// Insert a new element containing "data" before the current
// position.  Returns an iterator that points to the newly inserted
// element.  If the insertion fails, it returns an iterator pointing
// to End().
template<class T, int N>
inline typename ArrayList<T,N>::Iterator
ArrayList<T,N>::Insert(Iterator pos, T data)
{
  if(IsFull())
    return End();

  Iterator i = pos;
  _Insert(pos, data);

  return --i;
}

// Inserts n copies of data before the position pointed to by pos.
// Returns true on success and false on failure.
template<class T, int N>
inline bool
ArrayList<T,N>::Insert(Iterator pos, T data, int n)
{
  if (n > GetFreeSpace())
    return false;

  while(n-- > 0)    
    _Insert(pos, data);

  return true;
}

// Inserts the range of elements [begin, end) into the list before the 
// position pointed to by pos.
template<class T, int N>
inline bool
ArrayList<T,N>::Insert(Iterator pos, ConstIterator begin, ConstIterator end)
{
  while(begin != end) {
    if (IsFull())
      return false;
    _Insert(pos, *begin);
    ++begin;
  }

  return true;
}

// Removes the element pointed to by the iterator pos.  After this
// function completes, the iterator pos is no longer valid.  The
// function returns an iterator that points to the element following
// the one that has been removed.
template<class T, int N>
inline typename ArrayList<T,N>::Iterator
ArrayList<T,N>::Remove(Iterator pos)
{
  if (IsEmpty() || pos == End())
    return End();

  return _Remove(pos);
}

// This function removes the range [begin, end) from the list.  After
// this function completes, any iterators that point into that range
// should be considered invalid and should be discarded.
// The function returns true if the range has been successfully
// removed.
template<class T, int N>
inline bool
ArrayList<T,N>::Remove(Iterator begin, Iterator end)
{
  while(begin != end) {
    if (IsEmpty())
      return false;
    _Remove(begin);
    ++begin;
  }

  return true;
}

// Copy constructor that does a "deep copy" of another list, copying
// every element and creating a new node list.
template<class T, int N>
inline const ArrayList<T,N>&
ArrayList<T,N>::operator=(const ArrayList<T,N>& list)
{
  // Check for assignment to self.
  if (this == &list)
    return *this;

  Init();
  Insert(Begin(), list.Begin(), list.End());

  return *this;
}

// Compares two lists to see if they contain identical data.
template<class T, int N>
inline bool
ArrayList<T,N>::operator==(const ArrayList<T,N>& list)
{
  //Check comparison to self.
  if (this == &list)
    return true;

  // If the two lists are of different size, then they cannot be the
  // same list.
  if (GetOccupancy() != list.GetOccupancy())
    return false;

  // Get iterators to the beginning of the two lists.
  ConstIterator i = Begin();
  ConstIterator j = list.Begin();

  // Compare each element of the two lists.
  while(i != End()) {
    if (i != j)
      return false;

    ++i;
    ++j;
  }

  return true;
}

// Compares two lists to see if there are any differences.
template<class T, int N>
inline bool
ArrayList<T,N>::operator!=(const ArrayList<T,N>& list)
{ return !operator==(list); }

#endif //_ARRAYLIST_
