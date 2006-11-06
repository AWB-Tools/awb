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
  * @file TagIdVec.h
  */

#ifndef _DRALDB_TAGIDVEC_H
#define _DRALDB_TAGIDVEC_H

#include "asim/DRALTag.h"
#include "asim/StrTable.h"
#include "asim/AEVector.h"
#include "asim/TagVec.h"
#include "asim/ZipObject.h"

#include "asim/TagVecDictionary.h"
#include "asim/TagVecDictionaryNF.h"
#include "asim/TagVecItemIdx.h"

#define TAGIDVECNODE_AEPAGESIZE 32
#define TAGIDVECNODE_AENUMPAGES 8
#define TAGIDVECNODE_MAXCHUNKS  (TAGIDVECNODE_AEPAGESIZE*TAGIDVECNODE_AENUMPAGES)
#define TAGIDVECNODE_MAXCYCLES (TAGIDVECNODE_MAXCHUNKS*CYCLE_CHUNK_SIZE)

/**
  * @brief
  * Class that stores the tracks of all the tags of a node or item.
  *
  * @description
  * This class has an array of 4096 entries that track the values
  * of the tags of an element. Each entry is a tag vector that
  * holds the values for this tag.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */

/** @typedef TagCycleVector
  * @brief
  * Array that holds all the pointers to tag vectors for a given
  * tag. 8 * 32 * 4096 = 1 million cycles.
  */
typedef AEVector<TagVec*,TAGIDVECNODE_AEPAGESIZE,TAGIDVECNODE_AENUMPAGES> TagCycleVector;

class TagIdVecNode : public ZipObject
{
    public:
        inline TagIdVecNode();
        inline virtual ~TagIdVecNode();

        // ---- ZibObject Interface methods
        ZipObject* compressYourSelf(INT32 cycle, bool last=false);
        // -----------------------------------------------

        inline void setFwd(bool value);
        inline void setUseDictionary(bool value);

        bool getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle);
        inline bool getTagValue(INT32 cycle, QString* value, UINT32* atcycle);
        bool getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle);

        inline bool addTagValue(INT32 cycle, UINT64   value);
        inline bool addTagValue(INT32 cycle, QString  value);
        inline bool addTagValue(INT32 cycle, SOVList* value);

        inline bool hasData();
        inline void checkCycleChunk(INT32 cycleChunk);

        void dumpTagIdVector();

        inline void incPendingCnt(INT32 cycle);
        inline void decPendingCnt(INT32 cycle);
        void setWriteEnabled(bool);

    protected:
        TagCycleVector* cycleVec; // Pointer to the tag vectors with values.
        INT32 lastCycle; // Last cycle with contens.
        INT32 firstCycle; // First cycle with contents.
        // state flags
        UINT8 isFwd         : 1; // A value is forwarded when search.
        UINT8 useDictionary : 1; // Use dictionary.

        // optimization flags...

    private:
        static StrTable* strtbl; // Pointer to the string table.
};

/** @typedef TagIdVector
  * @brief
  * Array that holds the track of all the tags of a node or item.
  * A maximum of 4096 tags per element is allowed.
 */
typedef AEVector<TagIdVecNode,32,128> TagIdVector;

/**
 * Creator of this class. Gets the static instances and sets the
 * default values to the fields of the class.
 *
 * @return new object.
 */
TagIdVecNode::TagIdVecNode()
{
    if (strtbl==NULL)
    {
        strtbl = StrTable::getInstance();
    }
    firstCycle = 2147483647;
    lastCycle  = -1;
    isFwd      =  0;
    useDictionary = 1;
    cycleVec      = NULL;
}

/**
 * Destructor of this class. Frees the array if was allocated.
 *
 * @return destroys the object.
 */
TagIdVecNode::~TagIdVecNode()
{
    
    if (cycleVec!=NULL)
    {
        delete cycleVec;
    }
}

/**
 * Adds a value to the tag in the cycle cycle. Uses the cycle to
 * know which is the tag vector where the value must be stored.
 *
 * @return true if everything is ok.
 */
bool
TagIdVecNode::addTagValue(INT32 cycle, UINT64 value)
{

    // ensure cycle ordered insertion
    //if (cycle<lastCycle) return false;

    lastCycle = QMAX(cycle,lastCycle);
    firstCycle = QMIN(firstCycle,cycle);
    INT32 cycleChunk = cycle >> CYCLE_OFFSET_BITS;

    //printf ("TagIdVecNode::addTagValue cycle=%d, chunk=%d\n",cycle,cycleChunk);

    // check if there is a vector allocated for that...
    checkCycleChunk(cycleChunk);
    return (*cycleVec)[cycleChunk]->addTagValue(cycle,value);
}

/**
 * Adds a value to the tag in the cycle cycle. Uses the cycle to
 * know which is the tag vector where the value must be stored.
 *
 * @return true if everything is ok.
 */
bool
TagIdVecNode::addTagValue(INT32 cycle, QString  value)
{
    // ensure cycle ordered insertion
    //if (cycle<lastCycle) return false;

    lastCycle = QMAX(cycle,lastCycle);
    firstCycle = QMIN(firstCycle,cycle);
    INT32 cycleChunk = cycle >> CYCLE_OFFSET_BITS;
    UINT64 nvalue = (UINT64)(strtbl->addString(value));
    // check if there is a vector allocated for that...
    checkCycleChunk(cycleChunk);
    return (*cycleVec)[cycleChunk]->addTagValue(cycle,nvalue);
}

/**
 * Adds a value to the tag in the cycle cycle. Uses the cycle to
 * know which is the tag vector where the value must be stored.
 *
 * @return true if everything is ok.
 */
bool
TagIdVecNode::addTagValue(INT32 cycle, SOVList* value)
{
    /// @todo implement this
    return false;
}

/**
 * Checks that a cycle chunk is allocated. First looks if the
 * array has been allocated. Then looks if the chunk is allocated.
 * In negative case is allocated and all the pointers are set to
 * NULL. Finally allocates the correct type of tag vector.
 *
 * @return void.
 */
void
TagIdVecNode::checkCycleChunk(INT32 cycleChunk)
{
    // first time we use this tagId?
    if (cycleVec==NULL)
    {
        cycleVec = new TagCycleVector();
    }

    // since TagCycleVector is a vector of pointers I need to
    // make sure I initialize it with NULLs but, at the same time
    // I'd like to avoid initialize (and therefore allocate)
    // uneeded pages on the AEVector so:

    if ( !(cycleVec->hasElement(cycleChunk)) ) // no allocated page?
    {
        cycleVec->allocateElement(cycleChunk);    // allocate it

        // clear the AE page to avoid dangling/trash pointers.
        INT32 aepage = cycleChunk / TAGIDVECNODE_AEPAGESIZE;
        INT32 aepageElem0 = aepage*TAGIDVECNODE_AEPAGESIZE;  // improve this with a mask
        for (int i=0;i<TAGIDVECNODE_AEPAGESIZE;i++)
        {
            (*cycleVec)[aepageElem0+i]=NULL;
        }
    }

    if ((*cycleVec)[cycleChunk]==NULL)
    {
        // get some TagVec implementation,
        if (useDictionary)
        {
            if (isFwd)
            {
                (*cycleVec)[cycleChunk] = new TagVecDictionary(cycleChunk*CYCLE_CHUNK_SIZE);
            }
            else
            {
                (*cycleVec)[cycleChunk] = new TagVecDictionaryNF(cycleChunk*CYCLE_CHUNK_SIZE);
            }
        }
        else
        {
            // by now only itemidx-like slot do not use dictionary so:
            (*cycleVec)[cycleChunk] = new TagVecItemIdx(cycleChunk*CYCLE_CHUNK_SIZE);
        }
    }
}

/**
 * Checks if the array with the values has already been allocated.
 *
 * @return if the array has been allocated.
 */
bool
TagIdVecNode::hasData()
{
    return (cycleVec!=NULL);
}

/**
 * Sets to forward the behaviour of the vector.
 *
 * @return void.
 */
void
TagIdVecNode::setFwd(bool value)
{
    // you cannot change behavior AFTER infomation has been put on the vector.
    Q_ASSERT(cycleVec==NULL);
    isFwd = (UINT16)value;
}

/**
 * Sets to dictionary the behaviour of the vector.
 *
 * @return void.
 */
void
TagIdVecNode::setUseDictionary(bool value)
{
    // you cannot change behavior AFTER infomation has been put on the vector.
    Q_ASSERT(cycleVec==NULL);
    useDictionary = (UINT16)value;
}

/**
 * Gets the value of the tag in the cycle cycle.
 *
 * @return true if a value is found.
 */
bool
TagIdVecNode::getTagValue(INT32 cycle, QString* value, UINT32* atcycle)
{
    UINT64 nvalue;
    bool ok = getTagValue(cycle,&nvalue,atcycle);
    if (!ok)
    {
        return false;
    }
    *value = strtbl->getString((INT32)nvalue);
    return true;
}

/**
 * Increments the pending count.
 *
 * @return void.
 */
void
TagIdVecNode::incPendingCnt(INT32 cycle)
{
    INT32 cycleChunk = cycle >> CYCLE_OFFSET_BITS;
    checkCycleChunk(cycleChunk);
    (*cycleVec)[cycleChunk]->incPendingCnt();
}

/**
 * Decrements the pending count.
 *
 * @return void.
 */
void
TagIdVecNode::decPendingCnt(INT32 cycle)
{
    INT32 cycleChunk = cycle >> CYCLE_OFFSET_BITS;
    checkCycleChunk(cycleChunk);
    (*cycleVec)[cycleChunk]->decPendingCnt();
}

#endif
