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
  * @file TagVecDenseShortItemIdx.h
  */

#ifndef _DRALDB_TAGVECDENSESHORTITEMIDX_H
#define _DRALDB_TAGVECDENSESHORTITEMIDX_H

#include "asim/DralDBDefinitions.h"
#include "asim/TagVecItemIdx.h"

/**
  * @brief
  * This class is a tag vector that stores index ids.
  *
  * @description
  * This implementation of the index tag vector just uses 20 bits
  * to store the item id and 12 for the offset. Only 20 bits are
  * needed because is guaranteed that the maximum value enter is
  * never greater than 2^20.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */

// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning
// Warning Warning Warning Warning Warning Warning Warning Warning Warning Warning

// This class holds UINT20 values, not UINT64 nor QString nor SOVList
// this is only for ItemIdx saved on Enter/ExitNode and MoveItems commands.

/**
  * @typedef TagVecDenseShortItemIdxNode
  * @brief
  * Entries of a tag vector item index.
  */
typedef struct
{
    UINT32 value   : 20; // Item id of the entry.
    UINT32 offset  : 12; // Offset of the entry
} TagVecDenseShortItemIdxNode;

class TagVecDenseShortItemIdx : public TagVec
{
    public:
        TagVecDenseShortItemIdx(TagVecItemIdx* source);
        virtual ~TagVecDenseShortItemIdx();

        inline bool getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle);
        inline bool getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle);

        inline bool addTagValue(INT32 cycle, UINT64   value);
        inline bool addTagValue(INT32 cycle, QString  value);
        inline bool addTagValue(INT32 cycle, SOVList* value);

        void dumpCycleVector();
        inline TagVecEncodingType getType();

        ZipObject* compressYourSelf(INT32 cycle, bool last=false);

    protected:
        TagVecDenseShortItemIdxNode* valvec; // Array of indexes.
        INT32 nextEntry; // Position of the next entry.
        INT32 baseCycle; // Base cycle of the vector.
};

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecDenseShortItemIdx::addTagValue(INT32 cycle, UINT64 value)
{
    return false;
}

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecDenseShortItemIdx::addTagValue(INT32 cycle, QString  value)
{
    return false;
}

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecDenseShortItemIdx::addTagValue(INT32 cycle, SOVList* value)
{
    return false;
}

/**
 * Returns the type of vector.
 *
 * @return the type.
 */
TagVecEncodingType
TagVecDenseShortItemIdx::getType()
{
    return TVEType_DENSE_ITEMIDX;
}

/**
 * Gets the value (item id) in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecDenseShortItemIdx::getTagValue(INT32 cycle, UINT64* value, UINT32* atcycle)
{
    INT32 i=0;
    bool fnd = false ;
	UINT32 ccycle = 0;
    while (!fnd && ((valvec[i].offset+baseCycle)<=(UINT32)cycle) && (i<nextEntry))
    {
		ccycle = valvec[i].offset+baseCycle;
        fnd = ccycle == (UINT32)cycle ;
        *value = (UINT64)(valvec[i].value);
        ++i;
    }
	if (fnd && (atcycle!=NULL) )
    {
        *atcycle = ccycle;
    }
    return fnd;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return always false.
 */
bool
TagVecDenseShortItemIdx::getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle)
{
    return false;
}

#endif
