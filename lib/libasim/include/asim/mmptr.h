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

template <class Type, bool LAZY_DEST> class ASIM_MM_CLASS;

#ifndef _MMPTR_
#define _MMPTR_

template <class Type, bool LAZY_DEST = false>
class mmptr
{
  private:
    Type *ptr;
    void copy(Type *p);
    void del();

  public:
    mmptr() { ptr = NULL; }
    mmptr(Type *p) { copy(p); }
    mmptr(const mmptr &mmp) { copy(mmp.ptr); }
    ~mmptr() { del(); ptr = NULL; }

  Type * ptr_value();

    Type *operator->() const { return ptr; }
    Type &operator*() const { return *ptr; }
    operator Type *() const { return ptr; }
     
#if 0
    mmptr &operator=(void *p) {
        ASSERTX(p == NULL);
        del();
        ptr = (Type *) p;
        return *this;
    }
#endif

    mmptr &operator=(Type *p) {
        if (p != ptr) {
            del();
            copy(p);
        }

        return *this;
    }

    mmptr &operator=(const mmptr &mmp) {
        return operator=(mmp.ptr);
    }
};  

template <class Type, bool LAZY_DEST>
Type * 
mmptr<Type, LAZY_DEST>::ptr_value() 
{ 
    return ptr; 
}

template <class Type, bool LAZY_DEST>
inline void 
mmptr<Type, LAZY_DEST>::copy(Type *p) 
{

    ptr = p;
    if(ptr) 
    {
        // when uncommenting this, you need to make DATA a public
        // member of mm in mm.h
	
        /*
        if (Type::data.ClassName == "CPU_INST_CLASS")
        {
            if (ptr->mmUid == 25005)
            {
                cout << "mid = " << uid << "\tcopy: " << fmt_p(ptr)
                     << " mmUid = " << ptr->mmUid << endl;
            }
        }
        */

        ((ASIM_MM_CLASS<Type, LAZY_DEST>*)ptr)->IncrRef();

    }
}
 
template <class Type, bool LAZY_DEST>
inline void 
mmptr<Type, LAZY_DEST>::del() 
{

    if(ptr) 
    {
        /*
        if (Type::data.ClassName == "CPU_INST_CLASS")
        {
            if (ptr->mmUid == 25005)
            {
                cout << "mid = " << uid << "\tdel: " << fmt_p(ptr)
                     << " mmUid = " << ptr->mmUid << endl;
            }
        }
        */

        ((ASIM_MM_CLASS<Type, LAZY_DEST>*)ptr)->DecrRef(); 
    }
}

#endif //_MMPTR_
