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
  * @file TagVecDictionaryNF.h
  */

#ifndef _DRALDB_TAGVECDICTIONARYNF_H
#define _DRALDB_TAGVECDICTIONARYNF_H

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
  * This implementation uses the Dict2064 class to map the keys
  * and values of the entries. The information is not compressed
  * in this class. Only values in the same cycle match.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */

// fwd reference of dense version
class TagVecDenseDictionaryNF;

class TagVecDictionaryNF : public TagVecDictionary
{
    public:
        TagVecDictionaryNF(INT32 bcycle);
        virtual ~TagVecDictionaryNF();

        inline bool getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle);
        inline bool getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle);

        inline TagVecEncodingType getType();

        ZipObject* compressYourSelf(INT32 cycle, bool last=false);
        void dumpCycleVector();

        friend class TagVecDenseDictionaryNF;
};

/**
 * Returns the type of vector.
 *
 * @return the type.
 */
TagVecEncodingType
TagVecDictionaryNF::getType()
{
    return TVEType_DICTIONARY_NF;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecDictionaryNF::getTagValue(INT32 cycle, UINT64* value, UINT32* atcycle)
{
    INT32 i=0;
    bool fnd = false ;
	UINT32 ccycle = 0;
    while ( (i<nextEntry) && !fnd && ((valvec[i].cycleOffset+baseCycle)<=(UINT32)cycle) )
    {
		ccycle = valvec[i].cycleOffset+baseCycle;
        fnd = ccycle == (UINT32)cycle ;
        *value = dict->getValueByCycle(valvec[i].key,ccycle);
        ++i;
    }
	if (fnd && (atcycle!=NULL))
    {
        *atcycle = ccycle;
    }
    return fnd;
}

/**
 * Gets the value of this tag in the cycle cycle.
 *
 * @return true if the value has been found.
 */
bool
TagVecDictionaryNF::getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle)
{
    /// @todo implement this.
    return false;
}

#endif
