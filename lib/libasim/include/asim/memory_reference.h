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
 * @file memory.h
 * @author Michael Adler
 * @brief Architecture neutral memory reference descriptors.
 */

#ifndef _MEMORY_REFERENCE_
#define _MEMORY_REFERENCE_

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"


typedef class MEMORY_VIRTUAL_REFERENCE_CLASS *MEMORY_VIRTUAL_REFERENCE;

class MEMORY_VIRTUAL_REFERENCE_CLASS
{
  public:
    MEMORY_VIRTUAL_REFERENCE_CLASS(UINT64 va = 0, UINT64 nBytes = 0) :
        va(va),
        nBytes(nBytes)
    {};

    UINT64 GetVA(void) const { return va; };
    UINT64 GetNBytes(void) const { return nBytes; };

    bool ContainsVA(UINT64 testVA) const;
    
  private:
    UINT64 va;
    UINT64 nBytes;
};


inline bool
MEMORY_VIRTUAL_REFERENCE_CLASS::ContainsVA(UINT64 testVA) const
{
    return (va <= testVA) && (testVA < va + nBytes);
};


#endif // _MEMORY_REFERENCE_
