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
  * @file TagVecDictionary.h
  */

#ifndef _DRALDB_TAGVECDICTIONARY_H
#define _DRALDB_TAGVECDICTIONARY_H

#include "asim/DralDBDefinitions.h"
#include "asim/TagVec.h"
#include "asim/StrTable.h"
#include "asim/Dict2064.h"

#define TAGVECDICTIONARY_MIN_DENSITY 0.75

/**
  * @brief
  * This class is a tag vector that uses a dictionary.
  *
  * @description
  * This implementation uses the Dict2064 class to map the keys
  * and values of the entries. The information is not compressed
  * in this class.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */

/**
  * @typedef TagVecDictionaryNode
  * @brief
  * Entries of a tag vector dictionary.
  */
typedef struct
{
    UINT32 cycleOffset: 12; // Offset with the base cycle.
    UINT32 key :        20; // Key used to know the value of the entry using the Dict2064.
} TagVecDictionaryNode;

// fwd reference of dense version
class TagVecDenseDictionary;

class TagVecDictionary : public TagVec
{
    public:
        TagVecDictionary(INT32 bcycle);
        virtual ~TagVecDictionary();

        inline  bool getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle);
        inline  bool getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle);

        inline bool addTagValue(INT32 cycle, UINT64   value);
        inline bool addTagValue(INT32 cycle, QString  value);
        inline bool addTagValue(INT32 cycle, SOVList* value);

        void dumpCycleVector();
        inline TagVecEncodingType getType();

        ZipObject* compressYourSelf(INT32 cycle, bool last=false);

        friend class TagVecDenseDictionary;

    protected:
        TagVecDictionaryNode valvec[CYCLE_CHUNK_SIZE]; // fixed size 4096 entries...
        INT32 nextEntry; // Position of the next value.
        INT32 baseCycle; // Cycle where the data starts.

    protected:
        static Dict2064* dict; // Pointer to the dictionary.
        static StrTable* strtbl; // Pointer to the string table.
};

/**
 * Adds a new tag in the vector.
 *
 * @return true if everything is ok.
 */
bool
TagVecDictionary::addTagValue(INT32 cycle, UINT64 value)
{
    //printf ("TagVecDictionary::addTagValue(cycle=%d,value=%llu\n",cycle,value);fflush(stdout);

    INT32 offset = cycle - baseCycle;
    Q_ASSERT ((offset>=0)&&(offset<CYCLE_CHUNK_SIZE));
    Q_ASSERT (nextEntry<CYCLE_CHUNK_SIZE);
    INT32 dkey = dict->getKeyFor(value,cycle);

    // check if is the same value than previous set
    if ((nextEntry>0) && ((UINT32)dkey==valvec[nextEntry-1].key) )
    {
        return true;
    }

    // Inserts the value.
    valvec[nextEntry].key = (UINT32)dkey;
    valvec[nextEntry].cycleOffset = (UINT32)offset;
    ++nextEntry;
    return true;
}

/**
 * Adds a new tag in the vector.
 *
 * @return true if everything is ok.
 */
bool
TagVecDictionary::addTagValue(INT32 cycle, QString  value)
{
    //printf ("TagVecDictionary::addTagValue(cycle=%d,value=%s\n",cycle,value.latin1());fflush(stdout);

    INT32 offset = cycle-baseCycle;
    Q_ASSERT ((offset>=0)&&(offset<CYCLE_CHUNK_SIZE));
    Q_ASSERT(nextEntry<CYCLE_CHUNK_SIZE);

    UINT64 nvalue = (UINT64)(strtbl->addString(value));
    INT32 dkey = dict->getKeyFor(nvalue,cycle);

    // check if is the same value than previous set
    if ((nextEntry>0) && ((UINT32)dkey==valvec[nextEntry-1].key) )
    {
        return true;
    }

    valvec[nextEntry].key = (UINT32)dkey;
    valvec[nextEntry].cycleOffset = (UINT32)offset;
    ++nextEntry;
    return true;
}

/**
 * Adds a new tag in the vector.
 *
 * @return true if everything is ok.
 */
bool
TagVecDictionary::addTagValue(INT32 cycle, SOVList* value)
{
    // TODO.
    return false;
}

/**
 * Returns the type of vector.
 *
 * @return the type.
 */
TagVecEncodingType
TagVecDictionary::getType()
{
    return TVEType_DICTIONARY;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecDictionary::getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle)
{
    //if (!fwd) return getTagValueNoFw(cycle,value);
    INT32 i=0;
	UINT32 ccycle = valvec[i].cycleOffset+baseCycle;
    bool somethingfnd = ccycle <= (UINT32)cycle ;
    while ( (i<nextEntry) && ((valvec[i].cycleOffset+baseCycle) <= (UINT32)cycle) )
    {
		ccycle = valvec[i].cycleOffset+baseCycle;
        *value = dict->getValueByCycle(valvec[i].key,ccycle);
        ++i;
    }
	if (atcycle!=NULL)
    {
        *atcycle = ccycle;
    }
    return somethingfnd;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecDictionary::getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle)
{
    /// @todo implement this.
    return false;
}

#endif
