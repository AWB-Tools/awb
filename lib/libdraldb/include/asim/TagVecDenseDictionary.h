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
  * @file TagVecDenseDictionary.h
  */

#ifndef _DRALDB_TAGVECDENSEDICTIONARY_H
#define _DRALDB_TAGVECDENSEDICTIONARY_H

#include "asim/DralDBDefinitions.h"
#include "asim/TagVec.h"
#include "asim/StrTable.h"
#include "asim/Dict2064.h"
#include "asim/TagVecDictionary.h"

/**
  * @brief
  * This class is a tag vector that uses a dictionary.
  *
  * @description
  * This is an implementation that just allocates the necessary
  * space for the entries that it stores. Has the same functions
  * than the non dense version. All the write functions are
  * disabled.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class TagVecDenseDictionary : public TagVec
{
    public:
        TagVecDenseDictionary(TagVecDictionary* original);
        virtual ~TagVecDenseDictionary();

        inline bool getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle);
        inline bool getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle);

        inline bool addTagValue(INT32 cycle, UINT64   value);
        inline bool addTagValue(INT32 cycle, QString  value);
        inline bool addTagValue(INT32 cycle, SOVList* value);

        inline ZipObject* compressYourSelf(INT32 cycle, bool last=false);
        inline TagVecEncodingType getType();

        void dumpCycleVector();

    private:
        TagVecDictionaryNode* valvec; // Pointer to the entries.
        INT32 nextEntry; // Position of the next value.
        INT32 baseCycle; // Cycle where the data starts.

    private:
        static Dict2064* dict; // Pointer to the dictionary.
        static StrTable* strtbl; // Pointer to the string table.
};

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecDenseDictionary::addTagValue(INT32 cycle, UINT64   value)
{
    // a posteriori additions not supported
    return false;
}

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecDenseDictionary::addTagValue(INT32 cycle, QString  value)
{
    // a posteriori additions not supported
    return false;
}

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecDenseDictionary::addTagValue(INT32 cycle, SOVList* value)
{
    // a posteriori additions not supported
    return false;
}

/**
 * Returns the type of vector.
 *
 * @return the type.
 */
TagVecEncodingType
TagVecDenseDictionary::getType()
{
    return TVEType_DENSE_DICTIONARY;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecDenseDictionary::getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle)
{
    INT32 i=0;
	UINT32 ccycle = valvec[i].cycleOffset+baseCycle;
    bool somethingfnd = ccycle <= (UINT32)cycle ;
    while ( (i<nextEntry) && ((valvec[i].cycleOffset+baseCycle)<= (UINT32)cycle) )
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
TagVecDenseDictionary::getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle)
{
    /// @todo implement this.
    return false;
}

/**
  * Object already compressed.
  *
  * @return itself.
  */
ZipObject*
TagVecDenseDictionary::compressYourSelf(INT32 cycle, bool last)
{
    return this;
}

#endif
