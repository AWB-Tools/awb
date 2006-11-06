// ==================================================
// Copyright (C) 2003-2006 Intel Corporation
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file TagVecItemIdx.h
  */

#ifndef _DRALDB_TAGVECITEMIDX_H
#define _DRALDB_TAGVECITEMIDX_H

#include "asim/DralDBDefinitions.h"
#include "asim/TagVec.h"

// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning

// This class holds UINT31 values, not UINT64 nor QString nor SOVList
// this is only for ItemIdx saved on Enter/ExitNode and MoveItems commands.

/**
  * @brief
  * This class is a tag vector that stores index ids.
  *
  * @description
  * This implementation of the tag vector is used to store item
  * ids instead of values. An item id matches only if is requested
  * in the exact cycle that when it was set.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */

/**
  * @typedef TagVecItemIdxNode
  * @brief
  * Entries of a tag vector item index.
  */
typedef struct
{
    UINT32 value: 31; // Item id of the entry.
    UINT32 used :  1; // Used or not.
} TagVecItemIdxNode;

// fwd reference of dense versions
class TagVecDenseItemIdx;
class TagVecDenseShortItemIdx;

class TagVecItemIdx : public TagVec
{
    public:
        TagVecItemIdx(INT32 bcycle);
        virtual ~TagVecItemIdx();

        inline bool getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle);
        inline bool getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle);

        inline bool addTagValue(INT32 cycle, UINT64   value);
        inline bool addTagValue(INT32 cycle, QString  value);
        inline bool addTagValue(INT32 cycle, SOVList* value);

        void dumpCycleVector();
        inline TagVecEncodingType getType();

        ZipObject* compressYourSelf(INT32 cycle, bool last=false);

        friend class TagVecDenseItemIdx;
        friend class TagVecDenseShortItemIdx;

    protected:
        TagVecItemIdxNode valvec[CYCLE_CHUNK_SIZE]; // fixed size 4096 entries...
        INT32 baseCycle; // Base cycle of the vector.
};

/**
 * Adds a new tag in the vector.
 *
 * @return true if everything is ok.
 */
bool
TagVecItemIdx::addTagValue(INT32 cycle, UINT64 value)
{
    //printf ("TagVecItemIdx::addTagValue(cycle=%d,value=%llu\n",cycle,value);fflush(stdout);

    INT32 offset = cycle - baseCycle;
    Q_ASSERT ((offset>=0)&&(offset<CYCLE_CHUNK_SIZE));
    valvec[offset].value = (UINT32) value;
    valvec[offset].used  = 1;
    return true;
}

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecItemIdx::addTagValue(INT32 cycle, QString  value)
{
    return false;
}

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecItemIdx::addTagValue(INT32 cycle, SOVList* value)
{
    return false;
}

/**
 * Returns the type of vector.
 *
 * @return the type.
 */
TagVecEncodingType
TagVecItemIdx::getType()
{
    return TVEType_ITEMIDX;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecItemIdx::getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle)
{
    INT32 offset = cycle - baseCycle;
    Q_ASSERT ((offset>=0)&&(offset<CYCLE_CHUNK_SIZE));
    if (! valvec[offset].used )
    {
        return false;
    }
    *value = (UINT64)valvec[offset].value;
	if (atcycle!=NULL)
    {
        *atcycle = cycle;
    }
    return true;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecItemIdx::getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle)
{
    return false;
}

#endif
