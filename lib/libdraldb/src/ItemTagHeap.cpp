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
  * @file ItemTagHeap.cpp
  */

#include "asim/ItemTagHeap.h"
#include "asim/ItemHandler.h"
#include "asim/StrTable.h"
#include "asim/PrimeList.h"
#include "asim/ItemHandler.h"

/**
 * The instance is NULL at the beginning.
 */
ItemTagHeap* ItemTagHeap::_myInstance = NULL;

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer.
 *
 * @return the instance of the class.
 */
ItemTagHeap*
ItemTagHeap::getInstance()
{
    if (_myInstance==NULL)
    {
        _myInstance = new ItemTagHeap();
    }

    Q_ASSERT(_myInstance!=NULL);
    return _myInstance;
}

/**
 * Destroys the unique instance of the class. The instance is
 * destroyed if it was previously created.
 *
 * @return void.
 */
void
ItemTagHeap::destroy()
{
    if (_myInstance!=NULL)
    {
        delete _myInstance;
    }
}

/**
 * Creator of this class. Just calls the init function.
 *
 * @return new object.
 */
ItemTagHeap::ItemTagHeap()
{
    myLogMgr = LogMgr::getInstance();
    tdv = TagDescVector::getInstance();
    dralVersion = 9999999;
    canonicalItemId = (UINT16) (TagDescVector::getItemId_TagId());
    itemVector = new ItemHeapVector();
    tagVector = new TagHeapVector();
    dict = Dict2064::getInstance();
    conf = DBConfig::getInstance();

    // the lookup cache
    lookUpCache = new QIntCache<UINT32>(CACHED_ITEMIDS,PrimeList::nearPrime(CACHED_ITEMIDS));
    lookUpCache->setAutoDelete(true);

    reset();
}

/**
 * Destructor of this class. Deletes the created objects.
 *
 * @return destroys the object.
 */
ItemTagHeap::~ItemTagHeap()
{
    delete itemVector;
    delete tagVector;
    delete lookUpCache;
}

/**
 * Resets the class.
 *
 * @return void.
 */
void
ItemTagHeap::reset()
{
    firstEffectiveCycle = -99999;
    nextItemVectorEntry = 0;
    nextTagVectorEntry = 1;
}

/**
 * Inserts a SOV for the tag tag_id to the item item_id.
 *
 * @return void.
 */
void
ItemTagHeap::newTag(INT32 item_id, UINT16 tag_id, SOVList * list, UINT32 cycle)
{
    ItemHandler handler;    ///< Used to store the new tags.
    TagHeapNode * tag_node; ///< Pointer used to store the values.

    // Gets a handler for the item.
    lookForItemId(&handler, item_id);

    Q_ASSERT(handler.valid_item);

    // Each value of the set of values is added.
    for(SOVList::iterator it = list->begin(); it!=list->end();it++)
    {
        // Allocates a new slot for the value.
        tag_node = allocateNewSlot(&handler);

        tag_node->dkey = dict->getKeyFor(* it, cycle-firstEffectiveCycle);
        tag_node->cycle = cycle-firstEffectiveCycle;
        tag_node->tagId = tag_id;
        tag_node->isSOV = 1;
        tag_node->isMutable = 0;
    }
}

/**
 * Inserts a new value for the tag tag_id to the item item_id.
 *
 * @return void.
 */
void
ItemTagHeap::newTag(INT32 item_id, UINT16 tag_id, UINT64 value, UINT32 cycle)
{
    ItemHandler handler;    ///< Used to store the new tags.
    TagHeapNode * tag_node; ///< Pointer used to store the values.
    bool mutable_tag;       ///< Used to know if the tag is mutable or not.

    // Gets a handler for the item.
    lookForItemId(&handler, item_id);

    Q_ASSERT(handler.isValidItemHandler());
    Q_ASSERT(tag_id != canonicalItemId);

    mutable_tag = false;
    // Tries to find an old value for the tag.
    // Skips the ITEMID in the first evaluation.
    while(skipToNextTag(&handler))
    {
        // The first time the tag is found the loop ends.
        if(tagVector->ref(handler.chunkIdx).content[handler.tagIdx].tagId == tag_id)
        {
            mutable_tag = true;
            break;
        }
    }

    if(mutable_tag)
    {
        // If is the first time that is mutable then the first value is set to mutable.
        // Faster setting it to true always than adding a branch.
        // TODO: should it be set later to avoid problems with the second value that
        // is set and has the same value than the first??
        tagVector->ref(handler.chunkIdx).content[handler.tagIdx].isMutable = true;

        // Mutable tag case.
        // First check if we must compress mutable tags.
        if(conf->getCompressMutable())
        {
            TagHeapNode * last_node; ///< Points to the last value of the current tag.

            do
            {
                last_node = &tagVector->ref(handler.chunkIdx).content[handler.tagIdx];
            }
            while(skipToNextEntry(&handler) && tagVector->ref(handler.chunkIdx).content[handler.tagIdx].tagId == tag_id);

            // If the same value is added, then it's not added.
            if(dict->getValueByCycle(last_node->dkey, last_node->cycle) == value)
            {
                return;
            }
        }
        else
        {
            // Goes until the last value of the tag.
            // The first evaluation skips the first value, that's always present.
            while(skipToNextEntry(&handler) && tagVector->ref(handler.chunkIdx).content[handler.tagIdx].tagId == tag_id)
            {
            }
        }

        // Looks if the new tag is added at the end of the tags.
        if(!handler.isValidTagHandler())
        {
            // If is added at the end just a new slot is allocated.
            tag_node = allocateNewSlot(&handler);
        }
        else
        {
            // If is added between two tags, creates a gap to store the new value.
            tag_node = shiftSlot(&handler);
        }
    }
    else
    {
        // Non-mutable tag case.
        // Just allocates a new slot.
        tag_node = allocateNewSlot(&handler);
    }
    tag_node->dkey = dict->getKeyFor(value, cycle - firstEffectiveCycle);
    tag_node->cycle = cycle - firstEffectiveCycle;
    tag_node->tagId = tag_id;
    tag_node->isSOV = 0;
    tag_node->isMutable = mutable_tag;
}

/**
 * Returns the item_id of the entry poited by the handler.
 *
 * @return the item_id.
 */
INT32
ItemTagHeap::getItemId(ItemHandler * handler)
{
    return itemVector->ref(handler->itemIdx).itemId;
}

/**
 * Just modifies the handler to point the next item.
 *
 * @return if has another item.
 */
bool
ItemTagHeap::skipToNextItem(ItemHandler * handler)
{
    handler->itemIdx++;
    if(handler->itemIdx == nextItemVectorEntry)
    {
        // If we were in the last item then it gets invalid.
        handler->invalid();
        return false;
    }
    else
    {
        resetTagState(handler);
        return true;
    }
}

/**
 * This function returns the index of the next entry if there's
 * another entry for this item.
 *
 * @return if has another entry.
 */
bool
ItemTagHeap::skipToNextEntry(ItemHandler * handler)
{
    if(isLast(handler))
    {
        // If it was pointing the last tag then the handler points to an invalid tag.
        handler->valid_tag = false;
        return false;
    }
    else
    {
        // Special case for ITEMID.
        if(handler->chunkIdx == -1)
        {
            // If the item has a first chunk of tags, then some more tags are left.
            if(itemVector->ref(handler->itemIdx).first_chunk != 0)
            {
                handler->chunkIdx = itemVector->ref(handler->itemIdx).first_chunk;
                handler->tagIdx = 0;
                return true;
            }
            else
            {
                handler->valid_tag = false;
                // No more tags left.
                return false;
            }
        }

        // Points to the next tag.
        handler->tagIdx++;
        // If has reached the end of the chunk.
        if(handler->tagIdx == TAGS_PER_CHUNK)
        {
            Q_ASSERT(tagVector->ref(handler->chunkIdx).getNextChunk() != 0);
            // Set the chunk as the next one and the tagIdx is the first one of the chunk.
            handler->chunkIdx = tagVector->ref(handler->chunkIdx).getNextChunk();
            handler->tagIdx = 0;
        }
        return true;
    }
}

/**
 * Moves the handler to the next tag that is pointing the handler.
 *
 * @return if has another tag.
 */
bool
ItemTagHeap::skipToNextTag(ItemHandler * handler)
{
    UINT16 base_tag_id; ///< Initial tag id.

    // Special case for ITEMID.
    if(handler->chunkIdx == -1)
    {
        // If the item has a first chunk of tags, then some more tags are left.
        if(itemVector->ref(handler->itemIdx).first_chunk != 0)
        {
            handler->chunkIdx = itemVector->ref(handler->itemIdx).first_chunk;
            handler->tagIdx = 0;
            return true;
        }
        else
        {
            handler->valid_tag = false;
            // No more tags left.
            return false;
        }
    }

    base_tag_id = tagVector->ref(handler->chunkIdx).content[handler->tagIdx].tagId;
    // Skips entries until the actual tag has another id or the end of the tags is reached.
    while(skipToNextEntry(handler) && getTagId(handler) == base_tag_id)
    {
    }

    // If it's still pointing to a valid tag the handler has successfully pointed to the next tag.
    return handler->valid_tag;
}

/**
 * Rewind the handler to the first value of the actual tag. There's
 * no fast way to do this search because we don't have a pointer to
 * the previous chunk, so we need to search through all the tags again
 * until we find again the same tag.
 *
 * @return void.
 */
void
ItemTagHeap::rewindToFirstTagValue(ItemHandler * handler)
{
    UINT16 base_tag_id; ///< Id we must find again.

    // Special case for ITEMID.
    if(handler->chunkIdx == -1)
    {
        return;
    }

    // Only is done something if is not a simple tag.
    if(
        tagVector->ref(handler->chunkIdx).content[handler->tagIdx].isSOV ||
        tagVector->ref(handler->chunkIdx).content[handler->tagIdx].isMutable
    )
    {
        base_tag_id = tagVector->ref(handler->chunkIdx).content[handler->tagIdx].tagId;

        // Points the handler to the first tag.
        handler->chunkIdx = itemVector->ref(handler->itemIdx).first_chunk;
        handler->tagIdx = 0;
        // Runs through the tag list until it find the tag again.
        while(tagVector->ref(handler->chunkIdx).content[handler->tagIdx].tagId != base_tag_id)
        {
            skipToNextEntry(handler);
        }
    }
}

/**
 * Returns a new handler that points to the item with id itemid.
 * If the item doesn't exists, then the ItemHandler returned is
 * invalid.
 *
 * @return a handler pointing to the item.
 */
bool
ItemTagHeap::lookForItemId(ItemHandler * handler, INT32 itemid)
{
    UINT32 * pt; ///< Used to know the cache hit.
    INT32 i;     ///< Used to make the linear search.

    // TODO : direct mapping??
    // Tries to find the mapping in the lookup cache.
    pt = lookUpCache->find((long) itemid);
    if(pt != NULL)
    {
        handler->itemIdx = * pt;
    }
    else
    {
        // If unlucky perform a search.
        i = 0;
        // Runs through all the item vector until finds the item.
        while(i < nextItemVectorEntry && itemVector->ref(i).itemId != itemid)
        {
            i++;
        }

        // Looks if found.
        if(i == nextItemVectorEntry)
        {
            // We haven't found it, so we return an invalid handler.
            handler->invalid();
            return false;
        }
        else
        {
            lookUpCache->insert((long) itemid, new UINT32(i));
            handler->itemIdx = i;
        }
    }
    handler->valid_item = true;
    resetTagState(handler);
    return true;
}

/**
 * Returns a handler pointing to the first item of the tag heap.
 * If no items have been set, false is returned.
 *
 * @return true if a first item handler is returned.
 */
bool
ItemTagHeap::getFirstItem(ItemHandler * handler)
{
    if(nextItemVectorEntry == 0)
    {
        // If no item has been added then returns false.
        handler->invalid();
        return false;
    }
    else
    {
        // Gets the first item entry.
        lookForItemId(handler, itemVector->ref(0).itemId);
        return true;
    }
}

/**
 * Returns a handler pointing to the last item of the tag heap.
 * If no items have been set, false is returned.
 *
 * @return true if a the handler is valid.
 */
bool
ItemTagHeap::getLastItem(ItemHandler * handler)
{
    if(nextItemVectorEntry == 0)
    {
        // If no item has been added then returns false.
        handler->invalid();
        return false;
    }
    else
    {
        // Gets the last item entry.
        lookForItemId(handler, itemVector->ref(nextItemVectorEntry - 1).itemId);
        return true;
    }
}

/**
 * Returns the tag value, tag type, tag base and when the tag was
 * inserted of the entry in the index idx.
 *
 * @return index if everything is correct. -1 otherwise.
 */
bool
ItemTagHeap::getTag(ItemHandler * handler, UINT64* tgvalue,TagValueType* tgvtype,
                INT16* tgbase, UINT64* tgwhen, UINT32 cycle, bool backpropagate)
{
    TagHeapNode * node; ///< Pointer to the tag pointed by handler.
    INT32 tgId;         ///< Id of the tag pointed by handler.

    // Special case when the handler points to the ITEMID.
    if(handler->chunkIdx == -1)
    {
        * tgvtype = tdv->getTagValueType(canonicalItemId);
        * tgbase  = tdv->getTagValueBase(canonicalItemId);
        * tgvalue = (UINT64) itemVector->ref(handler->itemIdx).itemId;
        * tgwhen  = 0;
        return true;
    }

    node = &(tagVector->ref(handler->chunkIdx).content[handler->tagIdx]);
    tgId = node->tagId;
    * tgvtype = tdv->getTagValueType(tgId);
    * tgbase  = tdv->getTagValueBase(tgId);

    // If the entry has a fixed tag.
    if(!node->isMutable)
    {
        // The value is defined.
        if(((cycle - firstEffectiveCycle) >= node->cycle) || (conf->getTagBackPropagate()) || (dralVersion < 2))
        {
            * tgvalue = getValue(handler);
            * tgwhen  = node->cycle + firstEffectiveCycle;
            return true;
        }
        // The value is not defined yet.
        else
        {
            return false;
        }
    }
    // Gets the value using the mutable function.
    else
    {
        return getMutableTagValue(handler, cycle - firstEffectiveCycle, tgvalue, tgwhen, backpropagate);
    }
}

/**
 * Returns the tag identity, and tag value of the entry in the
 * index idx.
 *
 * @return index if everything is correct. -1 otherwise.
 */
bool
ItemTagHeap::getTag(ItemHandler * handler, UINT16* tgIdent,UINT64* tgvalue,UINT32 cycle, bool backpropagate)
{
    TagHeapNode * node; ///< Pointer to the tag slot.

    // Special case when the handler points to the ITEMID.
    if(handler->chunkIdx == -1)
    {
        * tgIdent = canonicalItemId;
        * tgvalue = (UINT64) itemVector->ref(handler->itemIdx).itemId;
        return true;
    }

    node = &(tagVector->ref(handler->chunkIdx).content[handler->tagIdx]);
    * tgIdent = node->tagId;
    if(!node->isMutable)
    {
        if(((cycle - firstEffectiveCycle) >= node->cycle) || (conf->getTagBackPropagate()) || (dralVersion < 2))
        {
            * tgvalue = getValue(handler);
            return true;
        }
        else
        {
            // Undefined at this point.
            return false;
        }
    }
    else
    {
        UINT64 dummy; ///< Dummy variable.

        return getMutableTagValue(handler, cycle - firstEffectiveCycle, tgvalue, &dummy, backpropagate);
    }
}

/**
 * Returns value of the entry in the index idx.
 *
 * @return index if everything is correct. -1 otherwise.
 */
bool
ItemTagHeap::fastGetTag(ItemHandler * handler, UINT64 * tgvalue, UINT32 cycle, bool backpropagate)
{
    TagHeapNode * node; ///< Pointer to the tag slot.

    // Special case when the handler points to the ITEMID.
    if(handler->chunkIdx == -1)
    {
        * tgvalue = (UINT64) itemVector->ref(handler->itemIdx).itemId;
        return true;
    }

    node = &(tagVector->ref(handler->chunkIdx).content[handler->tagIdx]);
    if(!node->isMutable)
    {
        if(((cycle - firstEffectiveCycle) >= node->cycle) || (conf->getTagBackPropagate()) || (dralVersion < 2))
        {
            * tgvalue = dict->getValueByCycle(node->dkey, node->cycle);
            return true;
        }
        else
        {
            // Undefined at this point.
            return false;
        }
    }
    else
    {
        UINT64 dummy; ///< Dummy Variable.

        return getMutableTagValue(handler, cycle - firstEffectiveCycle, tgvalue, &dummy, backpropagate);
    }
}

/**
 * Just forwards the function to getTagById converting the string
 * to an integer.
 *
 * @return if the item has a tag with this name.
 */
bool
ItemTagHeap::getTagByName(ItemHandler * handler, QString str, UINT64* tgvalue,
         TagValueType* tgvtype, INT16* tgbase, UINT64* tgwhen, UINT32 cycle, bool backpropagate)
{
    // Get the tag number to avoid lots of string comparisions.
    INT32 rcvTgId = tdv->scanTagName(str);
    if(rcvTgId < 0)
    {
        return false;
    }
    return getTagById(handler, (UINT16) rcvTgId,tgvalue,tgvtype,tgbase,tgwhen,cycle, backpropagate);
}

/**
 * This function first finds the index where the tag with tag_id
 * rcvTgId of the item in this entry (idx)is defined. Then calls
 * another function to obtain all the information of this tag.
 *
 * @return idx if everything is correct. -1 otherwise.
 */
bool
ItemTagHeap::getTagById(ItemHandler * handler, UINT16 rcvTgId, UINT64* tgvalue,
         TagValueType* tgvtype, INT16* tgbase, UINT64* tgwhen,UINT32 cycle, bool backpropagate)
{
    // Points the handler to the first tag.
    resetTagState(handler);

    // Special case when ITEMID is requested.
    if(rcvTgId == canonicalItemId)
    {
        * tgvtype = tdv->getTagValueType(canonicalItemId);
        * tgbase  = tdv->getTagValueBase(canonicalItemId);
        * tgvalue = (UINT64) itemVector->ref(handler->itemIdx).itemId;
        * tgwhen  = 0;
        return true;
    }

    // Skips the ITEMID.
    skipToNextEntry(handler);

    // Tries to find the tag with id rcvTgId.
    while(handler->isValidTagHandler() && tagVector->ref(handler->chunkIdx).content[handler->tagIdx].tagId != rcvTgId)
    {
        skipToNextEntry(handler);
    }

    // If it's pointing to a correct position.
    if(handler->valid_tag)
    {
        // Returns if the tag has a set value.
        return getTag(handler, tgvalue, tgvtype, tgbase, tgwhen, cycle, backpropagate);
    }

    // No value found.
    return false;
}

/**
 * Finds the index where the tag rcvTgId is defined beginning in
 * the index idx.
 *
 * @return the tag index.
 */
bool
ItemTagHeap::getTagPosition(ItemHandler * handler, UINT16 rcvTgId)
{
    // Points the handler to the first tag.
    resetTagState(handler);

    // Special case when ITEMID is requested.
    if(rcvTgId == canonicalItemId)
    {
        return true;
    }

    // Skips the ITEMID.
    skipToNextEntry(handler);

    // Tries to find the tag with id rcvTgId.
    while(handler->valid_tag && tagVector->ref(handler->chunkIdx).content[handler->tagIdx].tagId != rcvTgId)
    {
        skipToNextEntry(handler);
    }

    // True if is pointing to the tag.
    return handler->valid_tag;
}

/**
 * Returns the cycle when the actual tag was set.
 *
 * @return the cycle.
 */
UINT32
ItemTagHeap::getCycle(ItemHandler * handler)
{
    if(handler->chunkIdx == -1)
    {
        // If is pointing to the ITEMID 0 is returned.
        return 0;
    }
    else
    {
        return tagVector->ref(handler->chunkIdx).content[handler->tagIdx].cycle + firstEffectiveCycle;
    }
}

/**
 * Returns the value that is stored in the handler entry of the heap.
 * The dictionary is used to know the value using the key and cycle.
 *
 * @return the value in the handler position.
 */
UINT64
ItemTagHeap::getValue(ItemHandler * handler)
{
    // Special case when is pointing to ITEMID.
    if(handler->chunkIdx == -1)
    {
        return (UINT64) itemVector->ref(handler->itemIdx).itemId;
    }
    else
    {
        return dict->getValueByCycle(tagVector->ref(handler->chunkIdx).content[handler->tagIdx].dkey, tagVector->ref(handler->chunkIdx).content[handler->tagIdx].cycle);
    }
}

/**
 * Returns the tag id of the item in the handler.
 *
 * @return the tag id.
 */
UINT16
ItemTagHeap::getTagId(ItemHandler * handler)
{
    // Special case when is pointing to ITEMID.
    if(handler->chunkIdx == -1)
    {
        return canonicalItemId;
    }
    else
    {
        return tagVector->ref(handler->chunkIdx).content[handler->tagIdx].tagId;
    }
}

/**
 * Returns a boolean indicating if the current handler is the last
 * entry of an item. At least one tag in the item is supposed.
 *
 * @return true if is the last.
 */
bool
ItemTagHeap::isLast(ItemHandler * handler)
{
    // Special case when is pointing to ITEMID.
    if(handler->chunkIdx == -1)
    {
        return (itemVector->ref(handler->itemIdx).first_chunk == 0);
    }

    // Is the last if is in the last chunk and is the last slot inside the chunk.
    if(
        tagVector->ref(handler->chunkIdx).getNextChunk() == 0 &&
        (INT32) tagVector->ref(handler->chunkIdx).getFirstFreeSlot() == handler->tagIdx + 1
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Returns if the handler is a SOV or not.
 *
 * @return true if is a SOV. False otherwise.
 */
bool
ItemTagHeap::isSOV(ItemHandler * handler)
{
    // Special case when is pointing to ITEMID.
    if(handler->chunkIdx == -1)
    {
        return false;
    }
    else
    {
        return tagVector->ref(handler->chunkIdx).content[handler->tagIdx].isSOV;
    }
}

/**
 * Returns true if the handler is mutable. False otherwise.
 *
 * @return if the position is mutable.
 */
bool
ItemTagHeap::isMutable(ItemHandler * handler)
{
    // Special case when is pointing to ITEMID.
    if(handler->chunkIdx == -1)
    {
        return false;
    }
    else
    {
        return tagVector->ref(handler->chunkIdx).content[handler->tagIdx].isMutable;
    }
}

/**
 * Finds where the target_tagid has the value target_value.
 *
 * @return void.
 */
void
ItemTagHeap::lookForIntegerValue(ItemHandler * handler, UINT16 target_tagid, UINT64 target_value, UINT32 cycle, INT32 start_item)
{
    // TODO
    Q_ASSERT(false);
/*    INT32 pos = starting_point;
    INT32 itemIdPos = -1;
    UINT16 tagId;
    UINT16 itemId_TagId = TagDescVector::getItemId_TagId();

    INT16  tgbase;
    TagValueType tgvtype;
    UINT64 tgvalue;
    UINT64 tgwhen;

    bool fnd = false;
    while ( (!fnd) && (pos<nextEntry) )
    {
        tagId = aetagvector->at(pos).tagId;
        if (tagId==itemId_TagId)
        {
            itemIdPos=pos;
        }
        if (tagId==target_tagid)
        {
            bool ok = getTag(pos,&tgvalue,&tgvtype,&tgbase,&tgwhen,cycle);
            fnd = ok && (tgvalue == target_value);
        }
        ++pos;
    }

    INT32 result;
    if (fnd)
    {
        result = (itemIdPos);
        *tgPos = pos-1;
    }
    else
    {
        result = -1;
    }
    return (result);*/
}

/**
 * Finds where the target_tagid has the value target_value.
 *
 * @return the item_id of the item that matched the value.
 */
void
ItemTagHeap::lookForStrValue(ItemHandler * handler, UINT16 target_tagid, QString target_value, bool csensitive, bool exactMatch, UINT32 cycle, INT32 start_item)
{
    // TODO
    Q_ASSERT(false);
/*    if (starting_point<0)
    {
        return (-1);
    }

    StrTable* strtbl = StrTable::getInstance();

    INT32 pos = starting_point;
    INT32 itemIdPos = -1;
    UINT16 tagId;
    UINT16 itemId_TagId = TagDescVector::getItemId_TagId();

    INT16  tgbase;
    TagValueType tgvtype;
    UINT64 tgvalue;
    UINT64 tgwhen;

    if (!csensitive)
    {
        target_value = target_value.upper();
    }

    bool fnd = false;
    while ( (!fnd) && (pos<nextEntry) )
    {
        tagId = aetagvector->at(pos).tagId;
        if (tagId==itemId_TagId)
        {
            itemIdPos=pos;
        }
        if (tagId==target_tagid)
        {
            bool ok = getTag(pos,&tgvalue,&tgvtype,&tgbase,&tgwhen,cycle);
            QString fvalue = strtbl->getString((INT32)tgvalue);
            if (ok && (fvalue!=QString::null))
            {
                if (exactMatch)
                {
                    if (!csensitive)
                    {
                        fvalue = fvalue.upper();
                    }
                    fnd = (fvalue==target_value);
                }
                else
                {
                    fnd = (fvalue.contains(target_value,csensitive)>0);
                }
            }
        }
        ++pos;
    }

    INT32 result;
    if (fnd)
    {
        result = itemIdPos;
        *tgPos = pos-1;
    }
    else
    {
        result = -1;
    }
    return (result);*/
}

/**
 * Finds where the target_tagid matches the regular expression
 * target_value.
 *
 * @return the item_id of the item that matched the value.
 */
void
ItemTagHeap::lookForStrValue(ItemHandler * handler, UINT16 target_tagid, QRegExp target_value, UINT32 cycle, INT32 start_item)
{
    // TODO
    Q_ASSERT(false);
/*    if (starting_point<0)
    {
        return (-1);
    }

    StrTable* strtbl = StrTable::getInstance();
    UINT16 itemId_TagId = TagDescVector::getItemId_TagId();

    INT32 pos = starting_point;
    INT32 itemIdPos = -1;
    UINT16 tagId;

    INT16  tgbase;
    TagValueType tgvtype;
    UINT64 tgvalue;
    UINT64 tgwhen;

    bool fnd = false;
    while ( (!fnd) && (pos<nextEntry) )
    {
        tagId = aetagvector->at(pos).tagId;
        if (tagId==itemId_TagId)
        {
            itemIdPos=pos;
        }
        if (tagId==target_tagid)
        {
            bool ok = getTag(pos,&tgvalue,&tgvtype,&tgbase,&tgwhen,cycle);
            QString fvalue = strtbl->getString((INT32)tgvalue);
            if (ok && (fvalue!=QString::null))
            {
                fnd = (fvalue.contains(target_value)>0);
            }
        }
        ++pos;
    }

    INT32 result;
    if (fnd)
    {
        result = itemIdPos;
        *tgPos = pos-1;
    }
    else
    {
        result = -1;
    }
    return (result);*/
}


/**
 * This function searches through the contents of the mutable tag
 * starting in the position idx and finds the first value set after
 * col cycle.
 *
 * @return the index of the entry with the mutable value.
 */
bool
ItemTagHeap::getMutableTagValue(ItemHandler * handler, UINT32 cycle, UINT64 * result, UINT64 * when, bool backpropagate)
{
    UINT16 base_tag_id; ///< Tag Id of the mutable tag.
    ItemHandler last;   ///< Item handler pointing to the last visited value of the tag.
    ItemHandler actual; ///< Item handler pointing to the actual value of the tag.

    Q_ASSERT(handler->chunkIdx != -1);

    base_tag_id = tagVector->ref(handler->chunkIdx).content[handler->tagIdx].tagId;

    // Checks if have a value defined.
    if(tagVector->ref(handler->chunkIdx).content[handler->tagIdx].cycle > cycle)
    {
        //if(backpropagate)
        if(conf->getTagBackPropagate() || (dralVersion < 2))
        {
            * result = dict->getValueByCycle(tagVector->ref(handler->chunkIdx).content[handler->tagIdx].dkey, tagVector->ref(handler->chunkIdx).content[handler->tagIdx].cycle);
            * when = (UINT64)((tagVector->ref(handler->chunkIdx).content[handler->tagIdx].cycle) + firstEffectiveCycle);
            return true;
        }
        else
        {
            return false;
        }
    }

    actual = * handler;
    // Searches the different values of the tag until a value that was set previous to cycle.
    // Needs two copies because we can't rewind an entry.
    while(
        actual.valid_tag &&
        (tagVector->ref(actual.chunkIdx).content[actual.tagIdx].tagId == base_tag_id) &&
        (tagVector->ref(actual.chunkIdx).content[actual.tagIdx].cycle <= cycle)
    )
    {
        last = actual;
        skipToNextEntry(&actual);
    }

    * result = dict->getValueByCycle(tagVector->ref(last.chunkIdx).content[last.tagIdx].dkey, tagVector->ref(last.chunkIdx).content[last.tagIdx].cycle);
    * when = (UINT64)((tagVector->ref(last.chunkIdx).content[last.tagIdx].cycle) + firstEffectiveCycle);
    return true;
}

/**
 * Allocates a new tag slot for the item that contains the handler.
 *
 * @return size of the object.
 */
TagHeapNode *
ItemTagHeap::allocateNewSlot(ItemHandler * handler)
{
    // Checks whether the item has the first chunk allocated or not.
    if(itemVector->ref(handler->itemIdx).first_chunk == 0)
    {
        UINT32 chunkIdx; ///< Chunk allocated.

        // Must allocate the first chunk.
        chunkIdx = nextTagVectorEntry++;
        // Marks that the item has at least 1 tag.
        itemVector->ref(handler->itemIdx).first_chunk = chunkIdx;
        // Allocates the slot.
        (* tagVector)[chunkIdx].setFirstFreeSlot(1);
        tagVector->ref(chunkIdx).setNextChunk(0);
        // Returns the pointer to the tag allocated.
        return &(tagVector->ref(chunkIdx).content[0]);
    }
    else
    {
        UINT32 chunkIdx;    ///< Used to go until the last chunk of the actual item.
        UINT32 newChunkIdx; ///< Used to allocate a new chunk if necessary.

        chunkIdx = itemVector->ref(handler->itemIdx).first_chunk;

        // Goes until the last chunk.
        while(tagVector->ref(chunkIdx).getNextChunk() != 0)
        {
            chunkIdx = tagVector->ref(chunkIdx).getNextChunk();
        }
        // Looks if the last chunk is full.
        if(tagVector->ref(chunkIdx).getFirstFreeSlot() == 4)
        {
            // We must allocate a new chunk.
            newChunkIdx = nextTagVectorEntry++;
            // Sets the new allocated chunk as the next of the old chunk.
            tagVector->ref(chunkIdx).setNextChunk(newChunkIdx);
            // Allocates the slot.
            (* tagVector)[newChunkIdx].setFirstFreeSlot(1);
            tagVector->ref(newChunkIdx).setNextChunk(0);
            return &(tagVector->ref(newChunkIdx).content[0]);
        }
        else
        {
            UINT32 slot; ///< Slot allocated.

            // Just increments the occupancy.
            slot = tagVector->ref(chunkIdx).allocateSlot();
            return &(tagVector->ref(chunkIdx).content[slot]);
        }
    }
}

/**
 * This functions frees the slot pointed by the handler. This is
 * needed when a mutable tag is added.
 *
 * @return a pointer to the freed slot.
 */
TagHeapNode *
ItemTagHeap::shiftSlot(ItemHandler * handler)
{
    ItemHandler temp; ///< Handler used to pass the recursive parameter.
    TagHeapNode * node; ///< Gets the pointer to the new free tag slot.

    // If we want to shift the last position or a simple tag we can allocate
    // a new tag slot and copy it to the last position.
    if(
        isLast(handler) ||
        (!tagVector->ref(handler->chunkIdx).content[handler->tagIdx].isMutable &&
        !tagVector->ref(handler->chunkIdx).content[handler->tagIdx].isSOV)
    )
    {
        temp.itemIdx = handler->itemIdx;
        // Allocates a new slot.
        node = allocateNewSlot(&temp);
    }
    else
    {
        // Just shifts the next position.
        temp = * handler;
        skipToNextEntry(&temp);
        node = shiftSlot(&temp);
    }
    // Copies the old last tag to the new last tag, so the old last position is freed.
    node->cycle = tagVector->ref(handler->chunkIdx).content[handler->tagIdx].cycle;
    node->dkey = tagVector->ref(handler->chunkIdx).content[handler->tagIdx].dkey;
    node->isMutable = tagVector->ref(handler->chunkIdx).content[handler->tagIdx].isMutable;
    node->isSOV = tagVector->ref(handler->chunkIdx).content[handler->tagIdx].isSOV;
    node->tagId = tagVector->ref(handler->chunkIdx).content[handler->tagIdx].tagId;
    return &(tagVector->ref(handler->chunkIdx).content[handler->tagIdx]);
}

/**
 * Resets the state tag state of handler. This ItemHandler must
 * be valid and sets the fields chunkIdx, tagIdx and valid_tag of
 * handler.
 *
 * @return void.
 */
void
ItemTagHeap::resetTagState(ItemHandler * handler)
{
    Q_ASSERT(handler->valid_item);
    // Points it to the first tag, that is always the ITEMID.
    // The physical separation of ITEMID from the other tags needs
    // to be solved with a hack. chunkIdx == -1 means that the handler
    // is actually pointing to this special tag.

    handler->chunkIdx = -1;
    handler->tagIdx = -1;
    handler->valid_tag = true;
}

/**
 * Dumps all the tags of all the items to the standard output.
 *
 * @return void.
 */
void
ItemTagHeap::dumpItemTagHeap()
{
    ItemHandler hnd;

    getFirstItem(&hnd);
    while(hnd.isValidItemHandler())
    {
        hnd.dumpItem();
        hnd.skipToNextItem();
    }
}

/**
 * Dumps all the tags of all the items to the standard output.
 *
 * @return void.
 */
void
ItemTagHeap::dumpRegression()
{
    ItemHandler hnd;

    getFirstItem(&hnd);
    while(hnd.isValidItemHandler())
    {
        hnd.dumpRegression();
        hnd.skipToNextItem();
    }
}

/**
 * Returns the size of the tag heap.
 *
 * @return size of the object.
 */
INT64
ItemTagHeap::getObjSize() const
{
    INT64 alloc;
    alloc = itemVector->getNumSegments() * itemVector->getSegmentSize() * sizeof(ItemHeapNode);
    alloc = tagVector->getNumSegments() * tagVector->getSegmentSize() * sizeof(TagHeapChunk);
    return sizeof(ItemTagHeap) + alloc;
}

/**
 * Returns a string with a usage description of this class.
 *
* @return the description.
 */
QString
ItemTagHeap::getUsageDescription() const
{
    QString result = "";
    result += "\tAllocated Items:\t" + QString::number(itemVector->getNumSegments()) + "\n";
    result += "\tItem Entries per Segment:\t" + QString::number(itemVector->getSegmentSize()) + "\n";
    result += "\tAllocated Tags:\t" + QString::number(tagVector->getNumSegments()) + "\n";
    result += "\tTag Entries per Segment:\t" + QString::number(tagVector->getSegmentSize()) + "\n";
    return result;
}

/**
 * Creats a string with the stats of this class.
 *
 * @return the stats.
 */
QString
ItemTagHeap::getStats() const
{
    QString result = "";
    //double usageProp = ((double)nextEntry/(double)numEntries)*100.0;
    //double hitProp = ((double)_getIdTagHitCnt/(double)_getIdTagCnt)*100.0;

    //result += "\t\tHeap Usage:\t\t"+QString::number(nextEntry)+" from "+QString::number(numEntries)+" ("+QString::number(usageProp)+"%)\n";
    //result += "\t\tCached Lookup Hit Rate:\t"+QString::number(hitProp)+"%\n";
    return (result);
}
