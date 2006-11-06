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

#ifndef _FASTLIST_
#define _FASTLIST_

////////////////////////////////////////
//  class FastList<T>
//
//  Fast Linked list implementation of a list class.  There are no virtual
//  functions and the class has been optimized for speed.  The list is a
//  circularly linked to facliltate quick access to both ends of the list
//  with little penalty in speed.
template<class T>
class FastList
{
  friend class ConstIterator;
  friend class Iterator;

protected:
// The node structure is kept internal to the class because only the class
// itslef and its iterators need to know how the data is stored.
  class Node
  {
  public:
    T Data;
    Node* Next;
    
  public:
    Node() {}
    Node(const Node* node) : Data(node->Data) {}
    Node(const T& data) : Data(data) {}
    Node(const T& data, Node* node) : Data(data), Next(node) {}
    ~Node() {}
  };

// Again, this is a circularaly linked list class so we maintain a pointer
// to the end of the list.
  Node* Tail;

public:
  class ConstIterator
  {
  protected:
    union {
      const FastList<T>* cList;
      FastList<T>* List;
    };

    FastList<T>::Node* Curr;
    FastList<T>::Node* Prev;

  public:
    ConstIterator();
    ConstIterator(const FastList<T>& list);
    ConstIterator(const ConstIterator& iter);
    ~ConstIterator();
    
    void Attach(const FastList<T>& list);
    void Detach();
    bool IsAttached() const;
    void CheckAttached() const;
    
    bool Next();
    bool ToHead();
    bool AtHead();
    bool AtTail();

    bool Get(T& data) const;

    const ConstIterator& operator++();
    const ConstIterator operator++(int);

    const ConstIterator& operator=(const ConstIterator& iter);

    bool operator==(const ConstIterator& iter) const;
    bool operator!=(const ConstIterator& iter) const;

    const T& operator*() const ;
    const T* operator->() const;
    operator const T* () const;
  };

  class Iterator : public ConstIterator
  {
  private:
    void Attach(const FastList<T>& list);

  public:
    Iterator();
    Iterator(FastList<T>& list);
    Iterator(const Iterator& iter);
    ~Iterator();
    
    void Attach(FastList<T>& list);
    void InsertBefore(const T& data);
    void InsertAfter(const T& data);
    bool Remove(T& data);
    bool Remove();

    bool Set(const T& data);
    const Iterator& operator=(const Iterator& iter);

    T& operator*();
    T* operator->();
    operator T* ();
  };

protected:
  void Del();

public:
  FastList();
  FastList(const FastList<T>& list);
  ~FastList();

  bool IsEmpty() const;

  void InsertBeforeFirst(const T& data);
  void InsertAfterLast(const T& data);

  bool RemoveFirst(T& data);
  bool RemoveFirst();

  bool GetFirst(T& data) const;
  bool GetLast(T& data) const;

  Iterator GetIterator();
  ConstIterator GetConstIterator() const;

  const FastList<T>& operator=(const FastList<T>& list);
};

////////////////////////////////////////
//  class ListStack<T>
//
template<class T>
class ListStack
{
protected:
  FastList<T> List;

public:
  ListStack() {}
  explicit ListStack(const ListStack& stack) : List(stack.List) {}
  ~ListStack() {}

  bool IsEmpty() const { return List.IsEmpty(); }

  bool GetTop(T& data) { return List.GetFirst(data); }
  void Push(const T& data) { List.InsertBeforeFirst(data); }
  bool Pop(T& data) { return List.RemoveFirst(data); }
  bool Pop() { return List.RemoveFirst(); }

  const ListStack& operator=(const ListStack& stack)
    { List = stack.List; return *this; }
};

////////////////////////////////////////
//  class ListQueue<T>
//
template<class T>
class ListQueue
{
protected:
  FastList<T> List;

public:
  ListQueue() {}
  explicit ListQueue(const ListQueue& queue) : List(queue.List) {}
  ~ListQueue() {}

  bool IsEmpty() const { return List.IsEmpty(); }

  bool GetHead(T& data) { return List.GetFirst(data); }
  void Enqueue(const T& data) { List.InsertAfterLast(data); }
  bool Dequeue(T& data) { return List.RemoveFirst(data); }
  bool Dequeue() { return List.RemoveFirst(); }

  const ListQueue& operator=(const ListQueue& queue)
    { List = queue.List; return *this; }
};

////////////////////////////////////////
//  class FastList<T>
//
template<class T>
inline 
FastList<T>::FastList()
{
  Tail = NULL;
}

template<class T>
inline 
FastList<T>::FastList(const FastList<T>& list)
{
  Tail = NULL;
  operator=(list);
}

template<class T>
inline 
FastList<T>::~FastList()
{
  Del();
}

// Delete all of the elements in the list.
template<class T>
inline void
FastList<T>::Del()
{
  if (!IsEmpty()) {
    Node* n = Tail;
    Tail = Tail->Next;
    n->Next = NULL;
    
    while(Tail) {
      n = Tail;
      Tail = Tail->Next;
      delete n;
    }
  }

  Tail = NULL;
}

// Return true if the list is empty.
template<class T>
inline bool
FastList<T>::IsEmpty() const
{
  return (Tail == NULL);
}

// Insert an element at the beginning of the list before the first element.
template<class T>
inline void
FastList<T>::InsertBeforeFirst(const T& data)
{
  Node* n = new Node(data);

  if (IsEmpty()) {
    Tail = n;
    Tail->Next = Tail;
    return;
  }

  n->Next = Tail->Next;
  Tail->Next = n;
}

// Insert an element at the end of the list after the last element.
template<class T>
inline void
FastList<T>::InsertAfterLast(const T& data)
{
  Node* n = new Node(data);

  if (IsEmpty()) {
    Tail = n;
    Tail->Next = Tail;
    return;
  }

  n->Next = Tail->Next;
  Tail->Next = n;

  Tail = Tail->Next;
}

// Remove the first element from the list and set d equal to the data
//   contained at that location.
// Return true on success.
// Return false if List is empty.
template<class T>
inline bool
FastList<T>::RemoveFirst(T& data)
{
  if (!GetFirst(data))
    return false;

  Node* n = Tail->Next;
  Tail->Next = n->Next;
  if (Tail == n)
    Tail = NULL;

  delete n;
  
  return true;
}

// Remove the first element from the list.
// Return true on success.
// Return false if list is empty.
template<class T>
inline bool
FastList<T>::RemoveFirst()
{
  if (IsEmpty())
    return false;

  Node* n = Tail->Next;
  Tail->Next = n->Next;

  if (Tail == n)
    Tail = NULL;

  delete n;
  
  return true;
}

// Set d equal to the first element in the list.
// Return false if the list is empty.
template<class T>
inline bool
FastList<T>::GetFirst(T& data) const
{
  if (IsEmpty())
    return false;

  data = Tail->Next->Data;
  return true;
}

// Set d equal to the last element in the list.
// Return false if the list is empty
template<class T>
inline bool
FastList<T>::GetLast(T& data) const
{
  if (IsEmpty())
    return false;

  data = Tail->Data;
  return true;
}

template<class T>
inline typename FastList<T>::Iterator
FastList<T>::GetIterator()
{
  Iterator iter(*this);
  return iter;
}

template<class T>
inline typename FastList<T>::ConstIterator
FastList<T>::GetConstIterator() const
{
  ConstIterator iter(*this);
  return iter;
}


// Copy the list
template<class T>
inline const FastList<T>&
FastList<T>::operator=(const FastList<T>& list)
{
  Del();

  if (list.IsEmpty())
    return *this;


  Tail = new Node(list.Tail->Data);

  Node* n = list.Tail->Next;
  Node* t = Tail;

  while (n != list.Tail)
  {
    t->Next = new Node(n);
    t = t->Next;
    n = n->Next;
  }

  t->Next = Tail;

  return *this;
}

////////////////////////////////////////
//  class FastList<T>::ConstIterator
//
template<class T>
inline 
FastList<T>::ConstIterator::ConstIterator()
{
  cList = NULL;
  Curr = NULL;
  Prev = NULL;
}

template<class T>
inline 
FastList<T>::ConstIterator::ConstIterator(const FastList<T>& list)
{
  Attach(list);
}

template<class T>
inline 
FastList<T>::ConstIterator::ConstIterator(const FastList<T>::ConstIterator& iter)
{
  cList = iter.cList;
  Curr = iter.Curr;
  Prev = iter.Prev;
}

template<class T>
inline 
FastList<T>::ConstIterator::~ConstIterator()
{}


template<class T>
inline void 
FastList<T>::ConstIterator::CheckAttached() const
{
  ASSERTX(IsAttached());
}

template<class T>
inline bool
FastList<T>::ConstIterator::IsAttached() const
{
  return cList != NULL;
}

template<class T>
inline void 
FastList<T>::ConstIterator::Attach(const FastList<T>& list)
{
  cList = &list;
  ToHead();
}

template<class T>
inline void 
FastList<T>::ConstIterator::Detach()
{
  cList = NULL;
  Curr = NULL;
  Prev = NULL;
}

template<class T>
inline bool 
FastList<T>::ConstIterator::ToHead()
{
  // Should be attached to a list before iterating through it.
  CheckAttached();
  
  if (!cList->IsEmpty()) {
    Prev = cList->Tail;
    Curr = Prev->Next;
  }
  
  return true;
}

template<class T>
inline bool 
FastList<T>::ConstIterator::AtHead()
{
  // Should be attached to a list before iterating through it.
  CheckAttached();
  
  return cList->IsEmpty() || Curr == cList->Tail->Next;
}

template<class T>
inline bool 
FastList<T>::ConstIterator::AtTail()
{
  // Should be attached to a list before iterating through it.
  CheckAttached();
  
  return cList->IsEmpty() || Curr == cList->Tail;
}

template<class T>
inline bool 
FastList<T>::ConstIterator::Next()
{
  CheckAttached();

  if (Curr == cList->Tail)
    return false;

  Prev = Curr;
  Curr = Curr->Next;
  return true;
}

template<class T>
inline bool 
FastList<T>::ConstIterator::Get(T& data) const
{
  if (!Curr)
    return false;

  data = Curr->Data;
 
  return true;
}

template<class T>
inline const typename FastList<T>::ConstIterator&
FastList<T>::ConstIterator::operator=(const FastList<T>::ConstIterator& iter)
{
  cList = iter.cList;
  Curr = iter.Curr;
  Prev = iter.Prev;

  return *this;
}

template<class T>
inline const typename FastList<T>::ConstIterator&
FastList<T>::ConstIterator::operator++()
{
  Next();
  return *this;
}

template<class T>
inline const typename FastList<T>::ConstIterator 
FastList<T>::ConstIterator::operator++(int)
{
  ConstIterator iter = *this;
  Next();
  return iter;
}

template<class T>
inline bool 
FastList<T>::ConstIterator::operator==(const FastList<T>::ConstIterator& iter) const
{
  return cList == iter.cList && Curr == iter.Curr;
}

template<class T>
inline bool
FastList<T>::ConstIterator::operator!=(const FastList<T>::ConstIterator& iter) const
{
  return !operator==(iter);
}

template<class T>
inline const T&
FastList<T>::ConstIterator::operator*() const
{
  ASSERTX(Curr);
  return Curr->Data;
}

template<class T>
inline const T*
FastList<T>::ConstIterator::operator->() const
{
  if (!Curr)
    return NULL;

  return &Curr->Data;
}

template<class T>
inline
FastList<T>::ConstIterator::operator const T*() const
{
  if (!Curr)
    return NULL;

  return &Curr->Data;
}

////////////////////////////////////////
//  class FastList<T>::Iterator
//
template<class T>
inline 
FastList<T>::Iterator::Iterator()
  : ConstIterator()
{}

template<class T>
inline 
FastList<T>::Iterator::Iterator(FastList<T>& list)
{
  Attach(list);
  ToHead();
}

template<class T>
inline 
FastList<T>::Iterator::Iterator(const FastList<T>::Iterator& iter)
  : ConstIterator(iter)
{}

template<class T>
inline 
FastList<T>::Iterator::~Iterator()
{}

template<class T>
inline void 
FastList<T>::Iterator::Attach(FastList<T>& list)
{
  List = &list;
  ToHead();
}

// Inserts a node before the one that we are currently on.
template<class T>
inline void 
FastList<T>::Iterator::InsertBefore(const T& data)
{
  // The user should be attached to a list before trying to iterate through it.
  CheckAttached();

  //Boundary Condidion:
  //If the list is empty then we must create the first node in the list.
  if (!Curr)
  {
    ASSERTX(!Prev);
    Prev = new Node(data);
    Prev->Next = Prev;
    Curr = Prev;

    List->Tail = Prev;
    return;
  }
   
  ASSERTX(Prev);

  Curr = new Node(data, Prev->Next);
  Prev->Next = Curr;
}

// Esentially the same code as insert before.
template<class T>
inline void 
FastList<T>::Iterator::InsertAfter(const T& data)
{
  CheckAttached();

  if (List->IsEmpty())
  {
    ASSERTX(!Prev);
    Curr = new Node(data);
    Curr->Next = Curr;
    Prev = Curr;

    List->Tail = Curr;
    return;
  }

  ASSERTX(Curr);
  
  Node* n = new Node(data, Curr->Next);
  Curr->Next = n;

  if (Curr == List->Tail)
    List->Tail = Curr->Next;
}

template<class T>
inline bool 
FastList<T>::Iterator::Remove(T& data)
{
  CheckAttached();
  
  if (List->IsEmpty())
    return false;

  ASSERTX(Prev);

  data = Curr->Data;
  if (Curr == Prev)
  {
    List->Tail = NULL;
    delete Curr;
    Curr = NULL;
    Prev = NULL;
  }
  else
  {
    Prev->Next = Curr->Next;
    delete Curr;
    Curr = Prev->Next;
  }

  return true;
}

template<class T>
inline bool 
FastList<T>::Iterator::Remove()
{
  CheckAttached();

  if (List->IsEmpty())
    return false;

  ASSERTX(Prev);
      
  if (Curr == Prev)
  {
    List->Tail = NULL;
    delete Curr;
    Curr = NULL;
    Prev = NULL;
  }
  else
  {
    Prev->Next = Curr->Next;
    delete Curr;
    Curr = Prev->Next;
  }

  return true;
}

template<class T>
inline bool 
FastList<T>::Iterator::Set(const T& data)
{
  if (!Curr)
    return false;

  Curr->Data = data;
 
  return true;
}

template<class T>
inline const typename FastList<T>::Iterator&
FastList<T>::Iterator::operator=(const FastList<T>::Iterator& iter)
{
  List = iter.List;
  Curr = iter.Curr;
  Prev = iter.Prev;

  return *this;
}

template<class T>
inline T&
FastList<T>::Iterator::operator*()
{
  ASSERTX(Curr);
  return Curr->Data;
}

template<class T>
inline T*
FastList<T>::Iterator::operator->()
{
  if (!Curr)
    return NULL;

  return &Curr->Data;
}

template<class T>
inline
FastList<T>::Iterator::operator T*()
{
  if (!Curr)
    return NULL;

  return &Curr->Data;
}

#endif //_FASTLIST_
