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
  * @file TagVecDenseDictionaryNF.cpp
  */

#include "asim/TagVecDenseDictionaryNF.h"

/**
 * The static variables are set to NULL.
 */
Dict2064* TagVecDenseDictionaryNF::dict   = NULL;
StrTable* TagVecDenseDictionaryNF::strtbl = NULL;

/**
 * Creator of this class. Gets the static instances and allocates
 * space for the entries.
 *
 * @return new object.
 */
TagVecDenseDictionaryNF::TagVecDenseDictionaryNF(TagVecDictionaryNF* ovec)
{
    if (dict==NULL)
    {
        dict = Dict2064::getInstance();
    }
    if (strtbl==NULL)
    {
        strtbl = StrTable::getInstance();
    }
    baseCycle = ovec->baseCycle;
    nextEntry = ovec->nextEntry;
    // Allocates space just for the number of entries.
    valvec = new TagVecDictionaryNode[nextEntry];
    // Copies all the entries.
    for (int i=0;i<nextEntry;i++)
    {
        valvec[i].cycleOffset = ovec->valvec[i].cycleOffset;
        valvec[i].key = ovec->valvec[i].key;
    }
}

/**
 * Destructor of this class. Deletes the array if was allocated.
 *
 * @return destroys the object.
 */
TagVecDenseDictionaryNF::~TagVecDenseDictionaryNF()
{
    delete [] valvec;
}

/**
  * Dumps the content of the vector.
  *
  * @return void.
  */
void
TagVecDenseDictionaryNF::dumpCycleVector()
{
    printf ("Dumping cycle vector base cycle = %d. Encoding type is TVEType_DENSE_DICTIONARY_NF\n",baseCycle);
    for (int i=0;i<nextEntry;i++)
    {
        printf ("<%d,%llu> ",
                baseCycle+valvec[i].cycleOffset,
                dict->getValueByCycle(valvec[i].key,(valvec[i].cycleOffset+baseCycle))
               );
    }
    printf("\n");
}
