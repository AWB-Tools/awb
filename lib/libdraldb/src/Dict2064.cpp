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
  * @file Dict2064.cpp
  */

#include "asim/Dict2064.h"

/**
 * The instance is NULL at the beginning.
 */
Dict2064* Dict2064::_myInstance = NULL;

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer.
 *
 * @return the instance of the class.
 */
Dict2064*
Dict2064::getInstance()
{
    if (_myInstance==NULL)
    {
        _myInstance = new Dict2064();
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
Dict2064::destroy()
{
    if (_myInstance!=NULL)
    {
        delete _myInstance;
    }
}

/**
 * Creator of this class. Creates a new hash and allocs the first
 * dictionary.
 *
 * @return new object.
 */
Dict2064::Dict2064()
{
    nextDict=0;
    currentDict = new Hash6431(DICT2064_BUCKETS);
    allocNewDict(0);
}

/**
 * Destructor of this class. Just deletes the hash.
 *
 * @return destroys the object.
 */
Dict2064::~Dict2064()
{
    delete currentDict;
}

/**
 * Just returns the number of dictionarues used.
 *
 * @return the number of dictionaries.
 */
INT32
Dict2064::getNumDictionaries()
{
    return nextDict;
}
