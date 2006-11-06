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
  * @file StrTable.cpp
  */

#include "asim/StrTable.h"
#include "asim/PrimeList.h"

/**
 * The instance is NULL at the beginning.
 */
StrTable* StrTable::_myInstance = NULL;

/**
 * Returns the instance of the class. Default size is used.
 *
 * @return the instance of the class.
 */
StrTable* 
StrTable::getInstance ()
{
    return getInstance(STRTBL_DEFAULT_SIZE);
}

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer. The size of the string table is defined by sz.
 *
 * @return the instance of the class.
 */
StrTable* 
StrTable::getInstance (INT32 sz)
{
    if (_myInstance==NULL)
    {
        _myInstance = new StrTable(sz);
    }

    Q_ASSERT(_myInstance!=NULL);
    return (_myInstance);
}

/**
 * Destroys the unique instance of the class. The instance is
 * destroyed if it was previously created.
 *
 * @return void.
 */
void
StrTable::destroy()
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
StrTable::StrTable(INT32 sz)
{
    init(sz);
}

/**
 * Resizes the string table.
 *
 * @return void.
 */
void
StrTable::resize(INT32 newsize)
{
    // Just deletes the hashes.
    delete strhash;
    delete idxhash;
    // Creates them with the new size.
    init(newsize);
}

/**
 * Initializes the class with a size of sz.
 *
 * @return void.
 */
void
StrTable::init(INT32 sz)
{
    Q_ASSERT(sz>0);
    // look for the nearest prime number
    primeSize = PrimeList::nearPrime(sz);

    // Creates the 2 hashes used.
    strhash = new QDict<INT32>(primeSize);
    idxhash = new QIntDict<QString>(primeSize);
    Q_ASSERT(strhash!=NULL);
    Q_ASSERT(idxhash!=NULL);
    // When the hashes are deleted their objects too.
    strhash->setAutoDelete(true);
    idxhash->setAutoDelete(true);
    nextIdx = 0;
}

/**
 * Deletes the object.
 *
 * @return void.
 */
StrTable::~StrTable()
{
    // Just deletes the hashes.
    if (strhash!=NULL)
    {
        delete strhash;
    }
    if (idxhash!=NULL)
    {
        delete idxhash;
    }
}

/**
 * Resets the object.
 *
 * @return void.
 */
void 
StrTable::reset()
{
    nextIdx=0;
    strhash->clear();
    idxhash->clear();
}

/**
 * Computes the total size of the object to know the memory used.
 *
 * @return the object size.
 */
INT64
StrTable::getObjSize() const
{
    INT64 result = sizeof(StrTable);
    result += strhash->count()*2*sizeof(void*);
    result += idxhash->count()*2*sizeof(void*);

    QIntDictIterator<QString> it( *idxhash );
    for ( ; it.current(); ++it )
    {
        QString* current = it.current();
        result += current->length()*sizeof(QChar);
    }

    return result;
}

/**
 * Returns a description of the table.
 *
 * @return the description.
 */
QString
StrTable::getUsageDescription() const
{
    QString result = "";
    double load = (double)nextIdx/(double)primeSize;
    
    result += "\t\tReserved Hash Buckets:\t"+QString::number(primeSize)+"\n";
    result += "\t\tUsed Entries:\t\t"+QString::number(nextIdx)+"\n";
    result += "\t\tLoad:\t\t\t"+QString::number(load);
    if (load>0.9) result += ". Increase Symbol Table Size in Preferences!";
    result += "\n";
    return (result);
}

/**
 * Bypasses the call to getUsageDescription.
 *
 * @return the object stats.
 */
QString
StrTable::getStats() const
{
    return getUsageDescription();
}
