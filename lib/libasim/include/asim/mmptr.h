/*
 *Copyright (C) 2003-2006 Intel Corporation
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @author Nathan Binkert
 * @brief Automatic reference counting on pointers to MM objects.
 */

// generic
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/ioformat.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

template <class Type> class ASIM_MM_CLASS;

#ifndef _MMPTR_
#define _MMPTR_

template <class Type>
class mmptr
{
  private:
    Type *ptr;
    void copy(Type *p);
    bool del();

  public:
    mmptr() { ptr = NULL; }
    mmptr(Type *p) { copy(p); }
    mmptr(const mmptr &mmp) { copy(mmp.ptr); }
    ~mmptr()
    {
        bool killObj = del();
        Type *oldPtr = ptr;

        ptr = NULL;

        if (killObj)
        {
            ((ASIM_MM_CLASS<Type>*)oldPtr)->LastRefDropped();
        }
    }

    Type * ptr_value();

    Type *operator->() const { return ptr; }
    Type &operator*() const { return *ptr; }
    operator Type *() const { return ptr; }
     
    Type *operator=(const mmptr &mmp) const { return ptr; }

    mmptr &operator=(Type *p) {
        if (p != ptr) {
            bool killObj = del();
            Type *oldPtr = ptr;

            copy(p);

            if (killObj)
            {
                ((ASIM_MM_CLASS<Type>*)oldPtr)->LastRefDropped();
            }
        }

        return *this;
    }

    mmptr &operator=(const mmptr &mmp) {
        return operator=(mmp.ptr);
    }
};  

template <class Type>
Type * 
mmptr<Type>::ptr_value() 
{ 
    return ptr; 
}

template <class Type>
inline void 
mmptr<Type>::copy(Type *p) 
{

    ptr = p;
    if(ptr) 
    {
        ((ASIM_MM_CLASS<Type>*)ptr)->IncrRef();
    }
}
 
template <class Type>
inline bool
mmptr<Type>::del() 
{
    bool killObj = false;

    if(ptr) 
    {
        //
        // DecrRef returns the reference count following the decrement.  If
        // it reaches 0 return true so the caller knows to kill the object
        // after clearing any pointers to it.
        //
        if (((ASIM_MM_CLASS<Type>*)ptr)->DecrRef() <= 0)
        {
            killObj = true;
        }
    }

    return killObj;
}

#endif //_MMPTR_
