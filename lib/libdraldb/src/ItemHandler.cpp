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
  * @file  ItemHandler.cpp
  */

#include "asim/ItemHandler.h"
#include "asim/DBItoa.h"

/**
 * All the pointers are NULL at the beginning.
 */
ItemTagHeap*   ItemHandler::itemtagheap = NULL;
TagDescVector* ItemHandler::tagdescvec  = NULL;
StrTable*      ItemHandler::strtbl      = NULL;

/**
 * Sets the values of the static variables.
 *
 * @return void.
 */
void 
ItemHandler::initialize()
{
    itemtagheap = ItemTagHeap::getInstance();
    tagdescvec = TagDescVector::getInstance();
    strtbl = StrTable::getInstance();
}

/**
 * Dumps the content of an ItemHandler.
 *
 * @return void.
 */
void
ItemHandler::dumpItem()
{
    INT32 cidx = itemIdx;
    printf ("ItemId: %d at itemIdx %d\n", itemtagheap->getItemId(this), itemIdx);
    bool eol = itemtagheap->isLast(this);
    while (!eol)
    {
        itemtagheap->skipToNextEntry(this);
        eol = itemtagheap->isLast(this);
        printf("TagId = %d (%s),Value=%llu, PostCycle=%d, SOV=%d, Mutable=%d, chunkIdx=%d, tagIdx=%d\n",
               itemtagheap->getTagId(this),tagdescvec->getTagDescription(itemtagheap->getTagId(this)).latin1(),
               itemtagheap->getValue(this),
               itemtagheap->getCycle(this),(int)(itemtagheap->isSOV(this)),
               (int)(itemtagheap->isMutable(this)),
               chunkIdx, tagIdx
               );
    }
}

/**
 * Dumps the content of an ItemHandler.
 *
 * @return void.
 */
void
ItemHandler::dumpRegression()
{
    INT32 cidx = itemIdx;
    char str[256];

    sprintf(str, FMT32X, itemtagheap->getItemId(this));
    printf("I=%s\n", str);
    bool eol = itemtagheap->isLast(this);
    while (!eol)
    {
        skipToNextEntry();
        eol = itemtagheap->isLast(this);
        sprintf(str, FMT16X, itemtagheap->getTagId(this));
        printf("T=%s,", str);
        sprintf(str, FMT64X, itemtagheap->getValue(this));
        printf("V=%s,", str);
        sprintf(str, FMT32X, itemtagheap->getCycle(this));
        printf("C=%s,", str);
        sprintf(str, FMT16X, (UINT16) itemtagheap->isSOV(this));
        printf("S=%s,", str);
        sprintf(str, FMT16X, (UINT16) itemtagheap->isMutable(this));
        printf("M=%s\n", str);
    }
}
