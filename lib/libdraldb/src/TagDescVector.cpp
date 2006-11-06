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
  * @file TagDescVector.cpp
  */

#include "asim/TagDescVector.h"
#include "asim/PrimeList.h"

/**
 * The instance is NULL at the beginning.
 */
TagDescVector* TagDescVector::_myInstance = NULL;

/**
 * Just initialize the var with an invalid value.
 */
INT32 TagDescVector::itemId_TagId = -1;
INT32 TagDescVector::itemIdIn_TagId = -1;
INT32 TagDescVector::itemIdxIn_TagId = -1;

/**
 * Returns the instance of the class. Default size is used.
 *
 * @return the instance of the class.
 */
TagDescVector* TagDescVector::getInstance ()
{
    return getInstance(TAGDESCVEC_SIZE);
}

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer. The size of the vector is defined by sz.
 *
 * @return the instance of the class.
 */
TagDescVector* TagDescVector::getInstance (INT32 sz)
{
    if (_myInstance==NULL)
    {
        _myInstance = new TagDescVector(sz);
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
TagDescVector::destroy()
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
TagDescVector::TagDescVector(INT32 sz)
{
    init(sz);
}

/**
 * Initializes the class with a size of sz.
 *
 * @return void.
 */
void
TagDescVector::init(INT32 sz)
{
    Q_ASSERT(sz>=0);
    primeSize = PrimeList::nearPrime(sz);

    // Creates the hashes.
    strhash =  new QDict<INT32>(primeSize);
    idxhash =  new QIntDict<QString>(primeSize);
    deschash = new QIntDict<tagDescriptor>(primeSize);
    longHash = new QDict<QString>(primeSize);
    Q_ASSERT(strhash!=NULL);
    Q_ASSERT(idxhash!=NULL);
    Q_ASSERT(deschash!=NULL);
    Q_ASSERT(longHash!=NULL);
    // Sets to autodelete to avoid memory leaks.
    strhash->setAutoDelete(true);
    idxhash->setAutoDelete(true);
    deschash->setAutoDelete(true);
    longHash->setAutoDelete(true);
    strtbl = StrTable::getInstance();
    reset();
}

/**
 * Resizes the vector.
 *
 * @return void.
 */
void
TagDescVector::resize(INT32 sz)
{
    // First deletes the hashes.
    delete strhash;
    delete idxhash;
    delete deschash;
    delete longHash;
    // Then inits with the new size.
    init(sz);
}

/**
 * Deletes the object.
 *
 * @return void.
 */
TagDescVector::~TagDescVector()
{
    // Just deletes the hashes.
    delete strhash;
    delete idxhash;
    delete deschash;
    delete longHash;
}

/**
 * Adds a tag decription in the vector.
 *
 * @return the index of the tag description.
 */
INT32 
TagDescVector::addTagDescription (QString desc, TagValueType t, INT16 base)
{
    // look for the string to avoid redundant entries
    INT32* idx_ptr = strhash->find(desc);

    if (idx_ptr == NULL) // not found
    {
        idx_ptr = new INT32;
        Q_ASSERT(idx_ptr!=NULL);
        *idx_ptr = nextIdx;
        strhash->insert(desc,idx_ptr);
        QString *tmpstr = new QString(desc);
        Q_ASSERT(tmpstr!=NULL);
        idxhash->insert((long)nextIdx,tmpstr);
        tagDescriptor* tgdesc = new tagDescriptor;
        tgdesc->base=base;
        tgdesc->type=t;
        deschash->insert((long)nextIdx,tgdesc);
        nextIdx++;
    }
    return (nextIdx-1);
}

/**
 * Resets the contents of the different hash tables and enters the
 * special tags to the vector.
 *
 * @return void.
 */
void
TagDescVector::reset()
{
    strhash->clear();
    idxhash->clear();
    deschash->clear();
    longHash->clear();
    nextIdx=0;
    itemId_TagId = addTagDescription(ITEMID_STR_TAG, TagIntegerValue, 10);
    Q_ASSERT(itemId_TagId==0);

    itemIdIn_TagId = addTagDescription(ITEMIDIN_STR_TAG, TagIntegerValue, 10);
    Q_ASSERT(itemIdIn_TagId==1);

    itemIdxIn_TagId = addTagDescription(ITEMIDXIN_STR_TAG, TagIntegerValue, 10);
    Q_ASSERT(itemIdxIn_TagId==2);
}

/**
 * Returns a list of the known tags in the vector.
 *
 * @return a list with the known tags.
 */
QStrList
TagDescVector::getKnownTags()
{
    QStrList strlist(false);
    QDictIterator<INT32> it( *strhash );
    for( ; it.current(); ++it )
    {
        strlist.append(it.currentKey().latin1());
    }
    return strlist;
}

/**
 * Computes the total size of the object to know the memory used.
 *
 * @return the object size.
 */
INT64
TagDescVector::getObjSize() const
{
    INT64 result = sizeof(TagDescVector);
    result += strhash->count()*(sizeof(INT32)+   2*sizeof(void*));
    result += idxhash->count()*(sizeof(QString)+ 2*sizeof(void*));
    result += deschash->count()*(sizeof(tagDescriptor)+ 2*sizeof(void*));
    return (result);
}

/**
 * Returns a description of the vector.
 *
 * @return the description.
 */
QString
TagDescVector::getUsageDescription() const
{
    QString result = "\nTag Descriptor Statistics:\n";
    result += "Reserved Buckets:\t" + QString::number(primeSize) + "\n";
    result += "Used Entries:\t" + QString::number(nextIdx) + "\n";
    return result;
}

/**
 * Returns the stats of the object. Only the number of entries
 * is returned.
 *
 * @return the object stats.
 */
QString
TagDescVector::getStats() const
{
   QString  result = "\tNumber of entries:\t"+QString::number(nextIdx)+"\n";
   return result;
}
