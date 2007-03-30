/*
 * Copyright (C) 2003-2006 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @author Michael Adler
 * @brief Lock-free free list.
 */

#ifndef _FREELIST_
#define _FREELIST_

#include "asim/syntax.h"
#include "asim/atomic.h"


template <class L_TYPE>
class ASIM_FREE_LIST_ELEMENT_CLASS
{
  private:
    L_TYPE *_freelist_next;

  public:
    ASIM_FREE_LIST_ELEMENT_CLASS() : _freelist_next(NULL) {};

    L_TYPE *GetFreeListNext() const { return _freelist_next; };
    void SetFreeListNext(L_TYPE *next) { _freelist_next = next; };
};
    

template <class L_TYPE>
class ASIM_FREE_LIST_CLASS
{
  private:
    typedef L_TYPE *L_TYPE_PTR;

    //
    // Head of the free list is two parts.  "list" is the true list head.
    // "txnId" is used to make sure that "list" isn't modified and replaced
    // with the same value by some other thread in the middle of a transaction.
    //
    union HEAD
    {
        struct
        {
            PTR_SIZED_UINT txnId;
            L_TYPE_PTR list;
        };

#if __WORDSIZE == 64
        UINT128 full __attribute__ ((aligned(16)));
#else
        UINT64  full __attribute__ ((aligned(8)));
#endif
    };
        
    HEAD head;
    ATOMIC_INT32 size;

  public:
    ASIM_FREE_LIST_CLASS();
    ~ASIM_FREE_LIST_CLASS();

    void Push(L_TYPE *obj);
    L_TYPE *Pop();

    bool Empty(void) const { return head.list == NULL; };
    INT32 Size(void) const { return size; };
};


template <class L_TYPE>
ASIM_FREE_LIST_CLASS<L_TYPE>::ASIM_FREE_LIST_CLASS()
    : size(0)
{
    // The data structure is expected to be aligned so a 128 bit compare
    // and exchange can work.
    ASSERTX((PTR_SIZED_UINT(&head) & (sizeof(head) - 1)) == 0);
    ASSERTX(sizeof(head.txnId) == sizeof(head.list));

    head.txnId = 0;
    head.list = NULL;
};


template <class L_TYPE>
ASIM_FREE_LIST_CLASS<L_TYPE>::~ASIM_FREE_LIST_CLASS()
{
    ASSERTX(head.list == NULL);
};


template <class L_TYPE>
void ASIM_FREE_LIST_CLASS<L_TYPE>::Push(
    L_TYPE *obj)
{
    if (obj == NULL) return;
    ASSERTX(obj->GetFreeListNext() == NULL);

    L_TYPE *oldList;
    do
    {
        oldList = head.list;
        obj->SetFreeListNext(oldList);
    }
    while (! CompareAndExchange((PTR_SIZED_UINT*)&head.list,
                                (PTR_SIZED_UINT)oldList,
                                (PTR_SIZED_UINT)obj));

    size++;
};


template <class L_TYPE>
L_TYPE *ASIM_FREE_LIST_CLASS<L_TYPE>::Pop()
{
    HEAD oldHead, newHead;
    do
    {
        oldHead.txnId = head.txnId++ + 1;
        oldHead.list = head.list;

        if (oldHead.list == NULL)
        {
            return NULL;
        }

        newHead.txnId = oldHead.txnId;
        newHead.list = oldHead.list->GetFreeListNext();
    }
    while (! CompareAndExchange(&head.full, oldHead.full, newHead.full));

    size--;

    oldHead.list->SetFreeListNext(NULL);
    return oldHead.list;
};


#endif // _FREELIST_
