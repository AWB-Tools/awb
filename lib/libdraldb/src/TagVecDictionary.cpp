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
  * @file TagVecDictionary.cpp
  */

#include "asim/TagVecDictionary.h"
#include "asim/TagVecDenseDictionary.h"

/**
 * The static variables are set to NULL.
 */
Dict2064* TagVecDictionary::dict   = NULL;
StrTable* TagVecDictionary::strtbl = NULL;

/**
 * Creator of this class. Gets the static instances.
 *
 * @return new object.
 */
TagVecDictionary::TagVecDictionary(INT32 bcycle)
{
    if (dict==NULL)
    {
        dict = Dict2064::getInstance();
    }
    if (strtbl==NULL)
    {
        strtbl = StrTable::getInstance();
    }
    baseCycle = bcycle;
    nextEntry=0;
}

/**
 * Destructor of this class. Nothing is done.
 *
 * @return destroys the object.
 */
TagVecDictionary::~TagVecDictionary()
{
}

/**
  * Compresses the vector to a dense vector.
  *
  * @return the compressed vector.
  */
ZipObject*
TagVecDictionary::compressYourSelf(INT32 cycle, bool last)
{
    ZipObject* result = this;
    if (last || (cycle>=(baseCycle+CYCLE_CHUNK_SIZE)))
    {
        double density = ((double)nextEntry/(double)CYCLE_CHUNK_SIZE);
        // Only is compressed if is a low densed vector.
        if (density<TAGVECDICTIONARY_MIN_DENSITY)
        {
            result = new TagVecDenseDictionary(this);
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
TagVecDictionary::dumpCycleVector()
{
    printf ("Dumping cycle vector base cycle = %d. Encoding type is TVEType_DICTIONARY\n",baseCycle);
    printf ("TVEType_DICTIONARY efficiency = %g \n",((double)nextEntry/(double)CYCLE_CHUNK_SIZE*100.0));
    for (int i=0;i<nextEntry;i++)
    {
        printf ("<%d,%llu> ",
                baseCycle+valvec[i].cycleOffset,
                dict->getValueByCycle(valvec[i].key,(valvec[i].cycleOffset+baseCycle))
               );
    }
    printf("\n");
}
