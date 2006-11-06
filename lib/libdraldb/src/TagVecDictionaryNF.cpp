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
  * @file TagVecDictionaryNF.cpp
  */

#include "asim/TagVecDictionaryNF.h"
#include "asim/TagVecDenseDictionaryNF.h"

/**
 * Creator of this class. Just calls the TagVecDictionary creator.
 *
 * @return new object.
 */
TagVecDictionaryNF::TagVecDictionaryNF(INT32 bcycle) : TagVecDictionary(bcycle)
{
}

/**
 * Destructor of this class. Nothing is done.
 *
 * @return destroys the object.
 */
TagVecDictionaryNF::~TagVecDictionaryNF()
{
}

/**
  * Compresses the vector to a dense vector.
  *
  * @return the compressed vector.
  */
ZipObject*
TagVecDictionaryNF::compressYourSelf(INT32 cycle, bool last)
{
    ZipObject* result = this;
    if (last || (cycle>=(baseCycle+CYCLE_CHUNK_SIZE)))
    {
        double density = ((double)nextEntry/(double)CYCLE_CHUNK_SIZE);
        // Only is compressed if is a low densed vector.
        if (density<TAGVECDICTIONARY_MIN_DENSITY)
        {
            result = new TagVecDenseDictionaryNF(this);
        }
    }
    return result;
}

/**
  * Dumps the content of the vector.
  *
  * @return void.
  */
void
TagVecDictionaryNF::dumpCycleVector()
{
    printf ("Dumping cycle vector base cycle = %d. Encoding type is TVEType_DICTIONARY_NF\n",baseCycle);
    printf ("TVEType_DICTIONARY_NF efficiency = %g \n",((double)nextEntry/(double)CYCLE_CHUNK_SIZE*100.0));
    for (int i=0;i<nextEntry;i++)
    {
        printf ("<%d,%llu> ",
                baseCycle+valvec[i].cycleOffset,
                dict->getValueByCycle(valvec[i].key,(valvec[i].cycleOffset+baseCycle))
               );
    }
    printf("\n");
}
