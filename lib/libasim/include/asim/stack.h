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

#ifndef _STACK_
#define _STACK_

// ASIM core
#include "asim/deque.h"

template <class T, unsigned N>
class Stack : private Deque<T,N>
{
public:
  Stack() {}
  ~Stack() {}

  bool IsEmpty() const;
  bool IsFull() const;
  int GetOccupancy() const;
  int GetCapacity() const;
  int GetFreeSpace() const;

  bool GetTop(T &d) const;
  bool Push(const T &d);
  bool Pop(T &d);
  bool Pop();
};

template <class T, unsigned N>
inline bool
Stack<T,N>::IsEmpty() const
{ return Deque<T,N>::IsEmpty(); }

template <class T, unsigned N>
inline bool
Stack<T,N>::IsFull() const
{ return Deque<T,N>::IsFull(); }

template <class T, unsigned N>
inline int
Stack<T,N>::GetOccupancy() const
{ return Deque<T,N>::GetOccupancy(); }

template <class T, unsigned N>
inline int
Stack<T,N>::GetCapacity() const
{ return Deque<T,N>::GetCapacity(); }

template <class T, unsigned N>
inline int
Stack<T,N>::GetFreeSpace() const
{ return Deque<T,N>::GetFreeSpace(); }

template <class T, unsigned N>
inline bool
Stack<T, N>::GetTop(T &d) const
{
  if (IsEmpty())
    return false;

  d = Front();
  return true;
}

template <class T, unsigned N>
inline bool
Stack<T, N>::Push(const T &d)
{ return PushFront(d); }

template <class T, unsigned N>
inline bool
Stack<T, N>::Pop(T& d)
{ return PopFront(d); }

template <class T, unsigned N>
inline bool
Stack<T, N>::Pop()
{ return PopFront(); }
#endif //_STACK_
