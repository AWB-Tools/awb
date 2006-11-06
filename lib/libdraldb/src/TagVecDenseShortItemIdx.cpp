// ==================================================
//Copyright (C) 2003-2006 Intel Corporation
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file TagVecDenseShortItemIdx.cpp
  */

#include "asim/TagVecDenseShortItemIdx.h"

/**
 * Creator of this class. Just allocates space for the entries.
 *
 * @return new object.
 */
TagVecDenseShortItemIdx::TagVecDenseShortItemIdx(TagVecItemIdx* ovec)
{
    baseCycle = ovec->baseCycle;
    nextEntry=0;
    for (int i=0;i<CYCLE_CHUNK_SIZE;i++)
    {
        if (ovec->valvec[i].used)
        {
            ++nextEntry;
        }
    }
    // Allocates space just for the number of entries.
    valvec = new TagVecDenseShortItemIdxNode[nextEntry];

    // Copies all the entries.
    int pos=0;
    for (int i=0;i<CYCLE_CHUNK_SIZE;i++)
    {
        if (ovec->valvec[i].used)
        {
            valvec[pos].value = ovec->valvec[i].value;
            valvec[pos].offset = i;
            ++pos;
        }
    }
}

/**
 * Destructor of this class. Deletes the array that was allocated.
 *
 * @return destroys the object.
 */
TagVecDenseShortItemIdx::~TagVecDenseShortItemIdx()
{
    delete [] valvec;
}

/**
  * Object already compressed.
  *
  * @return itself.
  */
ZipObject*
TagVecDenseShortItemIdx::compressYourSelf(INT32 cycle, bool last)
{
    return this;
}

/**
  * Dumps the content of the vector.
  *
  * @return void.
  */
void
TagVecDenseShortItemIdx::dumpCycleVector()
{
}
