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
  * @file  ItemHandler.h
  */

#ifndef _DRALDB_ITEMHANDLER_H
#define _DRALDB_ITEMHANDLER_H

#include "asim/draldb_syntax.h"
#include "asim/ItemTagHeap.h"
#include "asim/StrTable.h"

/**
  * @brief
  * This class is used as interface to access to the item tag heap.
  *
  * @description
  * ItemHandler is used like an iterator to access to the values
  * of the different tags of an item of the heap. All the functions
  * use the different methods defined in the ItemTagHeap to do all
  * the work. ItemHandler are supposed to be used in an iterator
  * fashion. First, an ItemHandler must be first created from
  * another ItemHandler or with the look-up creator (with an item
  * id). Then, the ItemHandler can iterate through all the tags of
  * the item where is pointing until reaches an invalid state.
  * Functions to point to the next item or to skip a tag value are
  * offered too.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class ItemHandler
{
    public:
        inline ItemHandler();

        inline INT32 getCurrentTagCycle();

        inline bool getCurrentTagValue(UINT64* tgvalue,INT32 cycle=2147483647, bool backpropagate=true);

        inline bool getCurrentTagValue(UINT16* tgId, UINT64* tgvalue,TagValueType*,INT16* tgbase,
               UINT64*tgwhen,INT32 cycle=2147483647, bool backpropagate=true);

        inline bool getTagByName(QString str, UINT64* tgvalue, TagValueType* tgvtype,
               INT16* tgbase, UINT64* tgwhen, INT32 cycle=2147483647, bool backpropagate=true);

        inline bool getTagById(UINT16 tgId, UINT64* tgvalue,TagValueType* tgvtype,
               INT16* tgbase, UINT64* tgwhen, INT32 cycle=2147483647, bool backpropagate=true);

        inline bool  hasTag(UINT16 tgId);
        inline bool  hasTag(QString tgname);

        inline INT32 getItemId();
        inline UINT16 getTagId ();

        inline bool skipToNextEntry();
        inline bool skipToNextTag();
        inline bool skipToNextItem();
        inline void goToFirstTag();
        inline void rewindToFirstTagValue();

        inline bool isValidItemHandler();
        inline bool isValidTagHandler();
        inline bool isMutableTag();
        inline bool isSOVTag();

        inline QString getFormatedTagValue();

        // for low level debugging
        void dumpItem();
        void dumpRegression();

        static void initialize();

        inline bool operator==(const ItemHandler & comp);

        inline INT32 uniqueIndex();

    protected:
        inline ItemHandler(INT32 itemId);
        inline void invalid();

        friend class ItemTagHeap;
        friend class TrackHeap;

    private:
        INT32 itemIdx;  // Index of the item of this handler.
        INT32 chunkIdx; // Index of the chunk where the actual tag is stored.
        INT32 tagIdx;   // Index within the chunk of the actual tag.
        bool valid_item; // True if is pointing a valid item.
        bool valid_tag;  // True if is pointing a valid tag. Can't be true if valid_item is false.

    private:
        static ItemTagHeap * itemtagheap;  // Pointer to the item tag heap.
        static TagDescVector * tagdescvec; // Pointer to the tag description vector.
        static StrTable * strtbl;          // Pointer to the string table conversion.
};

/**
 * Creator of this class. Just invalids the handler.
 *
 * @return new object.
 */
ItemHandler::ItemHandler()
{
    invalid();
}

/**
 * Creator of this class. Creates a new ItemHandler that points
 * to the tags of the item itemId. Forwards the call to the tag
 * heap.
 *
 * @return new object.
 */
ItemHandler::ItemHandler(INT32 itemId)
{
    itemtagheap->lookForItemId(this, itemId);
}

/**
 * Returns the cycle where the actual tag value was set. Only can
 * bre requested if the handler is pointing a tag that isn't ITEMID.
 *
 * @return the current cycle.
 */
INT32
ItemHandler::getCurrentTagCycle()
{
    Q_ASSERT(valid_tag);
    return itemtagheap->getCycle(this);
}

/**
 * Gets the id, value, type, base and set cycle of the current
 * tag of the handler.
 *
 * @return true if everything is ok.
 */
bool
ItemHandler::getCurrentTagValue(UINT16* tgId, UINT64* tgvalue,TagValueType*tgtype,INT16* tgbase,
       UINT64*tgwhen, INT32 cycle, bool backpropagate)
{
    Q_ASSERT(valid_tag);
    * tgId = itemtagheap->getTagId(this);
    return itemtagheap->getTag(this,tgvalue,tgtype,tgbase,tgwhen,cycle, backpropagate);
}

/**
 * Gets the value of the current tag pointed by the handler.
 *
 * @return true if everything is ok.
 */
bool
ItemHandler::getCurrentTagValue(UINT64* tgvalue, INT32 cycle, bool backpropagate)
{
    Q_ASSERT(valid_tag);
    //printf("GetCurrentValue : %i\n", cycle);
    return itemtagheap->fastGetTag(this, tgvalue, cycle, backpropagate);
}

/**
 * Gets all the information of a tag of the item pointed by the 
 * handler using its name.
 *
 * @return if a value has been found.
 */
bool
ItemHandler::getTagByName(QString str, UINT64* tgvalue, TagValueType* tgvtype,
       INT16* tgbase, UINT64* tgwhen, INT32 cycle, bool backpropagate)
{
    Q_ASSERT(valid_item);
    return itemtagheap->getTagByName(this, str, tgvalue, tgvtype, tgbase, tgwhen, cycle, backpropagate);
}

/**
 * Gets all the information of a tag  of the item pointed by the
 * handler using its id.
 *
 * @return if a value has been found.
 */
bool
ItemHandler::getTagById(UINT16 tgId, UINT64* tgvalue,TagValueType* tgvtype,
       INT16* tgbase, UINT64* tgwhen,INT32 cycle, bool backpropagate)
{
    Q_ASSERT(valid_item);
    return itemtagheap->getTagById(this, tgId, tgvalue, tgvtype, tgbase, tgwhen, cycle, backpropagate);
}

/**
 * Looks if the item pointed by the handler has a tag defined.
 *
 * @return true if the item has tgId defined.
 */
bool
ItemHandler::hasTag(UINT16 tgId)
{
    Q_ASSERT(valid_item);
    return itemtagheap->getTagPosition(this, tgId);
}

/**
 * Looks if the item pointed by the handler has a tag defined.
 *
 * @return true if the item has tgId defined.
 */
bool
ItemHandler::hasTag(QString tgname)
{
    Q_ASSERT(valid_item);

    INT32 tgId = tagdescvec->scanTagName(tgname);
    if(tgId < 0)
    {
        return false;
    }
    return hasTag((UINT16) tgId);
}

/**
 * Gets the itemid of the item of this handler.
 *
 * @return the itemid.
 */
INT32
ItemHandler::getItemId()
{
    Q_ASSERT(valid_item);
    return itemtagheap->getItemId(this);
}

/**
 * Gets the mutable flag of the actual tag pointed by this handler.
 *
 * @return true if the actual tag is mutable.
 */
bool
ItemHandler::isMutableTag()
{
    Q_ASSERT(valid_tag);
    return itemtagheap->isMutable(this);
}

/**
 * Gets the SOV flag of the actual tag.
 *
 * @return true if the actual tag is a SOV.
 */
bool
ItemHandler::isSOVTag()
{
    Q_ASSERT(valid_tag);
    return itemtagheap->isSOV(this);
}

/**
 * Changes the handler state to point to the next entry of the
 * item of the handler.
 *
 * @return true if a next entry exists.
 */
bool
ItemHandler::skipToNextEntry()
{
    if(!valid_tag)
    {
        return false;
    }
    else
    {
        return itemtagheap->skipToNextEntry(this);
    }
}

/**
 * Changes the handler state to point to the next tag of the
 * item of the handler.
 *
 * @return true if a next tag exists.
 */
bool
ItemHandler::skipToNextTag()
{
    if(!valid_tag)
    {
        return false;
    }
    else
    {
        return itemtagheap->skipToNextTag(this);
    }
}

/**
 * Changes the handler state to point to the first tag of the
 * item of the handler.
 *
 * @return void.
 */
void
ItemHandler::goToFirstTag()
{
    Q_ASSERT(valid_item);
    itemtagheap->resetTagState(this);
}

/**
 * Just returns the field's value.
 *
 * @return true if it's a valid item handler.
 */
bool
ItemHandler::isValidItemHandler()
{
    return valid_item;
}

/**
 * Just returns the field's value.
 *
 * @return true if it's a valid tag handler.
 */
bool
ItemHandler::isValidTagHandler()
{
    return valid_tag;
}

/**
 * Uses the itemtagheap to set the handler to point the next item.
 *
 * @return true if another item exists.
 */
bool
ItemHandler::skipToNextItem()
{
    if(!valid_item)
    {
        return false;
    }
    else
    {
        return itemtagheap->skipToNextItem(this);
    }
}

/**
 * Rewinds the handler to the first value of the current tag.
 *
 * @return void.
 */
void
ItemHandler::rewindToFirstTagValue()
{
    Q_ASSERT(valid_tag);
    itemtagheap->rewindToFirstTagValue(this);
}

/**
 * Gets the tag id of the current tag.
 *
 * @return the tag id.
 */
UINT16
ItemHandler::getTagId()
{
    return itemtagheap->getTagId(this);
}

/**
 * Returns a string with the formatted value of the actual tag.
 *
 * @return the formatted value.
 */
QString
ItemHandler::getFormatedTagValue()
{
    Q_ASSERT(valid_tag);
    UINT64 value;
    INT32 tagId = itemtagheap->getTagId(this);
    value = itemtagheap->getValue(this);
    return tagdescvec->getFormatedTagValue(tagId, value);
}

/**
 * Invalidates the state of the item handler.
 *
 * @return void.
 */
void
ItemHandler::invalid()
{
    valid_item = false;
    valid_tag = false;
    itemIdx = -1;
    chunkIdx = -1;
    tagIdx = -1;
}

/**
 * Compares two handlers and return if they point to the same 
 * entry.
 *
 * @return true if both handler points to the same entry.
 */
bool
ItemHandler::operator==(const ItemHandler & comp)
{
    return
        (
            (this->itemIdx == comp.itemIdx) &&
            (this->chunkIdx == comp.chunkIdx) &&
            (this->tagIdx == comp.tagIdx)
        );
}

/**
 * Returns an integer that is unique for the position where the
 * handler points to.
 *
 * @return a unique index for the handler.
 */
INT32
ItemHandler::uniqueIndex()
{
    if(chunkIdx == -1)
    {
        return itemIdx;
    }
    else
    {
        return (MAX_ITEM_ENTRIES + chunkIdx * TAGS_PER_CHUNK + tagIdx);
    }
}

#endif
