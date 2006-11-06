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
  * @file TagVecItemIdx.cpp
  */

#include <stdio.h>

#include "asim/TagVecItemIdx.h"
#include "asim/TagVecDenseItemIdx.h"
#include "asim/TagVecDenseShortItemIdx.h"

/**
 * Creator of this class. Sets to 0 the array.
 *
 * @return new object.
 */
TagVecItemIdx::TagVecItemIdx(INT32 bcycle)
{
    baseCycle = bcycle;
    bzero((char *)valvec,sizeof(valvec));
}

/**
 * Destructor of this class. Nothing is done.
 *
 * @return destroys the object.
 */
TagVecItemIdx::~TagVecItemIdx()
{
}

/**
  * Compresses the vector to a dense vector.
  *
  * @return the compressed vector.
  */
ZipObject*
TagVecItemIdx::compressYourSelf(INT32 cycle, bool last)
{
    UINT32 maxValue=0;
    INT32 cnt=0;
    if (last || (!we && (cycle>(baseCycle+CYCLE_CHUNK_SIZE))) )
    // warning, cycle> is enforced to keep data uncompressed until the next chunk,
    // in this way, 2Dreams and other clients keep O(1) access until next compress
    // round. Since complex clients usually evaluate rules based on tag values and
    // moveitems, O(1) access is performance critical.
    {
        // 1) check density & max Value
        for (int i=0;i<CYCLE_CHUNK_SIZE;i++)
        {
            if (valvec[i].used)
            {
                ++cnt;
                maxValue = QMAX(maxValue,valvec[i].value);
            }
        }
        //printf ("TagVecItemIdx::compressYourSelf cycle=%d,cnt=%d,max=%d, last=%d\n",cycle,cnt,maxValue,(int)last);
        // If it's a low dense vector is compressed.
        if (cnt< (CYCLE_CHUNK_SIZE-1)/2)
        {
            // If the max item id can't be stored in 20 bits creates a normal dense item index vector.
            if (maxValue>=1048576)
            {
                return new TagVecDenseItemIdx(this);
            }
            // Creates a short item index vector.
            else
            {
                return new TagVecDenseShortItemIdx(this);
            }
        }
    }
    return this;
}

/**
  * Dumps the content of the vector.
  *
  * @return void.
  */
void
TagVecItemIdx::dumpCycleVector()
{
    printf ("Dumping cycle vector base cycle = %d. Encoding type is TVEType_ITEMIDX\n",baseCycle);
    for (int i=0;i<CYCLE_CHUNK_SIZE;i++)
    {
        if (valvec[i].used)
        {
            printf ("<%d,%u> ",baseCycle+i,valvec[i].value);
        }
    }
    printf("\n");
}
