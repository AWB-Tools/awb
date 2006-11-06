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
  * @file TagVecDictionary.h
  */

#ifndef _DRALDB_TAGVECDENSEDICTIONARYNF_H
#define _DRALDB_TAGVECDENSEDICTIONARYNF_H

#include "asim/DralDBDefinitions.h"
#include "asim/TagVec.h"
#include "asim/StrTable.h"
#include "asim/Dict2064.h"
#include "asim/TagVecDictionaryNF.h"

/**
  * @brief
  * This class is a dense tag vector that uses a dictionary.
  *
  * @description
  * This implementation just alloc the necessary space to store
  * all the values. When a value is requested for one cycle, the
  * function only returns match if the set of the value is in the
  * same cycle.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class TagVecDenseDictionaryNF : public TagVec
{
    public:
        TagVecDenseDictionaryNF(TagVecDictionaryNF* original);
        virtual ~TagVecDenseDictionaryNF();

        inline bool getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle);
        inline bool getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle);

        inline TagVecEncodingType getType();

        inline bool addTagValue(INT32 cycle, UINT64   value);
        inline bool addTagValue(INT32 cycle, QString  value);
        inline bool addTagValue(INT32 cycle, SOVList* value);

        inline ZipObject* compressYourSelf(INT32 cycle, bool last=false);

        void dumpCycleVector();

    private:
        TagVecDictionaryNode* valvec; // Pointer to the array of data.
        INT32 nextEntry; // Position where the next entry will be added.
        INT32 baseCycle; // Base cycle of the vector.

    private:
        static Dict2064* dict; // Pointer to the dictionary of values.
        static StrTable* strtbl; // Pointer to the string table.
};

/**
 * Adds a new tag in the vector.
 *
 * @return always false.
 */
bool
TagVecDenseDictionaryNF::addTagValue(INT32 cycle, UINT64   value)
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
TagVecDenseDictionaryNF::addTagValue(INT32 cycle, QString  value)
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
TagVecDenseDictionaryNF::addTagValue(INT32 cycle, SOVList* value)
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
TagVecDenseDictionaryNF::getType()
{
    return TVEType_DENSE_DICTIONARY_NF;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecDenseDictionaryNF::getTagValue(INT32 cycle, UINT64* value, UINT32* atcycle)
{
    INT32 i=0;
    bool fnd = false ;
	UINT32 ccycle = 0;
    while ( (i<nextEntry) && !fnd && ((valvec[i].cycleOffset+baseCycle)<=(UINT32)cycle) )
    {
		ccycle = valvec[i].cycleOffset+baseCycle;
        fnd = ccycle == (UINT32)cycle;
        *value = dict->getValueByCycle(valvec[i].key,ccycle);
        ++i;
    }
	if (fnd && (atcycle!=NULL) ) { *atcycle = ccycle; }
    return fnd;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecDenseDictionaryNF::getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle)
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
TagVecDenseDictionaryNF::compressYourSelf(INT32 cycle, bool last)
{
    return this;
}

#endif
