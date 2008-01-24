/**************************************************************************
 *Copyright (C) 2005-2006 Intel Corporation
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
 * @file dynamic_array.h
 * @author Michael Adler
 * @brief Simple class for managing an array of unknown initial size.
 */

#ifndef _DYNAMIC_ARRAY_H_
#define _DYNAMIC_ARRAY_H_


// ASIM core
#include "asim/syntax.h"

#ifndef SOFTSDV_STUB
#include "asim/mesg.h"
#endif

//
// This class is intended to be an easy replacement for fixed size arrays.
// It can be accessed like the old array.  Only the declaration of the
// array needs to be changed an a call to the Init() method added when
// the number of elements is known.
//

template <class DATA>
class DYNAMIC_ARRAY_CLASS
{
  public:
    DYNAMIC_ARRAY_CLASS(UINT32 elem = 0);
    ~DYNAMIC_ARRAY_CLASS();

    void Init(UINT32 elem);
    bool IsInitialized(void) const;

    DATA& operator[](UINT32 idx) const;

  private:
    UINT32 nElements;
    DATA *data;
};


template <class DATA>
inline
DYNAMIC_ARRAY_CLASS<DATA>::DYNAMIC_ARRAY_CLASS(UINT32 elem) : nElements(0), data(NULL)
{
    if (elem)
    {
        Init(elem);
    }
};


template <class DATA>
inline
DYNAMIC_ARRAY_CLASS<DATA>::~DYNAMIC_ARRAY_CLASS()
{
    if (nElements)
    {
        delete[] data;
        nElements = 0;
    }
};


template <class DATA>
inline void
DYNAMIC_ARRAY_CLASS<DATA>::Init(UINT32 elem)
{
    ASSERTX(nElements == 0);
    ASSERTX(elem > 0);

    nElements = elem;
    data = new DATA[elem];

    VERIFY(data != NULL, "Out of memory");
};


template <class DATA>
inline bool
DYNAMIC_ARRAY_CLASS<DATA>::IsInitialized(void) const
{
    return (nElements != 0);
};


template <class DATA>
inline DATA&
DYNAMIC_ARRAY_CLASS<DATA>::operator[](UINT32 idx) const
{
    ASSERTX(idx < nElements);
    return data[idx];
};


#endif // _DYNAMIC_ARRAY_H_
