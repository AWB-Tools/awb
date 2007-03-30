/**************************************************************************
 * INTEL CONFIDENTIAL Copyright (c) 2007 Intel Corp.
 *
 * Recipient is granted a non-sublicensable copyright license under
 * Intel copyrights to copy and distribute this code internally only.
 * This code is provided "AS IS" with no support and with no
 * warranties of any kind, including warranties of MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE or INTELLECTUAL PROPERTY
 * INFRINGEMENT. By making any use of this code, Recipient agrees that
 * no other licenses to any Intel patents, trade secrets, copyrights
 * or other intellectual property rights are granted herein, and no
 * other licenses shall arise by estoppel, implication or by operation
 * of law. Recipient accepts all risks of use.
 **************************************************************************/

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
