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
  * @file TagDescVector.h
  */

#ifndef _DRALDB_TAGDESCVECTOR_H
#define _DRALDB_TAGDESCVECTOR_H

#include <stdio.h>

// Qt library
#include <qstrlist.h>
#include <qdict.h>
#include <qintdict.h>

#include "asim/draldb_syntax.h"
#include "asim/AMemObj.h"
#include "asim/StatObj.h"
#include "asim/DRALTag.h"
#include "asim/DralDBDefinitions.h"
#include "asim/StrTable.h"
#include "asim/DBItoa.h"

typedef struct
{
    INT16 base;
    TagValueType type;
} tagDescriptor;

/**
  * @brief
  * Vector holding tag descriptors (name, value type, etc).
  *
  * @description
  * This singleton class holds all the information of the user
  * defined tags. The name, value and type is stored.
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class TagDescVector : public AMemObj, public StatObj
{
    public:
        // ---- AMemObj Interface methods
        virtual INT64 getObjSize() const;
        virtual QString getUsageDescription() const;
        // ---- StatObj Interface methods
        QString getStats() const;

    public:
        static TagDescVector* getInstance ();
        static TagDescVector* getInstance (INT32 sz);
        static void destroy();

        inline static INT32 getItemId_TagId() {return itemId_TagId;}
        inline static INT32 getItemIdIn_TagId() {return itemIdIn_TagId;}
        inline static INT32 getItemIdxIn_TagId() {return itemIdxIn_TagId;}

    public:
        inline INT32   tryToAlloc (QString desc, TagValueType t, INT16 base=10);
        inline QString getTagDescription (INT32 id);
        inline INT16 getTagValueBase   (INT32 id);
        inline TagValueType getTagValueType (INT32 id);
        inline INT32 scanTagName(QString);
        inline bool hasTagName(QString);

        inline QString getTagLongDesc(QString tgName);
        inline void    setTagLongDesc(QString tgName, QString ldesc); 

        void reset();
        QStrList getKnownTags();
        void resize(INT32 sz);

        inline QString getFormatedTagValue(UINT16 tagId, UINT64 value);

    protected:
        TagDescVector(INT32 sz);
        virtual ~TagDescVector();
        void    init(INT32 sz);
        INT32   addTagDescription (QString desc, TagValueType t, INT16 base=10);

    private:
        QDict<INT32>*      strhash; // Hash used to know the index indexing with a tag.
        QIntDict<QString>* idxhash; // Hash used to know the tag indexing with an index.
        QIntDict<tagDescriptor>* deschash; // Contains the type and base of the tag description.
        INT32 nextIdx; // Next index entry.
        INT32 primeSize; // Primer near to the requested size.
        StrTable* strtbl; // Instance of the string table.
        QDict<QString>* longHash; // Holds the long description of the tag.

    private:
        static TagDescVector* _myInstance; // Pointer to the instance of the class.
        static INT32    itemId_TagId; // Value of the itemid.
        static INT32    itemIdIn_TagId; // Value of the itemidin.
        static INT32    itemIdxIn_TagId; // Value of the itemidxin.
};

/**
 * Returns the tag with id id.
 *
 * @return the tag.
 */
QString
TagDescVector::getTagDescription (INT32 id)
{
    Q_ASSERT(id<nextIdx);
    Q_ASSERT(id>=0);
    QString* str = idxhash->find((long)id);
    if (str==NULL)
    { return QString::null; }
    else
    { return *str; }
}

/**
 * Returns the tag type of the tag description.
 *
 * @return the tag type.
 */
TagValueType
TagDescVector::getTagValueType (INT32 id)
{
    Q_ASSERT(id<nextIdx);
    Q_ASSERT(id>=0);
    tagDescriptor* tg = deschash->find((long)id);
    Q_ASSERT(tg!=NULL);
    return tg->type;
}

/**
 * Returns the base of the tag description with id id.
 *
 * @return the base.
 */
INT16
TagDescVector::getTagValueBase(INT32 id)
{
    Q_ASSERT(id<nextIdx);
    Q_ASSERT(id>=0);
    tagDescriptor* tg = deschash->find((long)id);
    Q_ASSERT(tg!=NULL);
    return tg->base;
}

/**
 * Looks for the tag name in the vector and returns the index where
 * is stored.
 *
 * @return the index of the tag.
 */
INT32
TagDescVector::scanTagName(QString name)
{
    INT32 *idx = strhash->find(name);
    if (idx==NULL)
    {
        return (-1);
    }
    return (*idx);
}

/**
 * Tries to alloc a new tag description in the vector.
 *
 * @return id of the tag description.
 */
INT32
TagDescVector::tryToAlloc (QString desc, TagValueType t, INT16 base)
{
    // 1) check if this tag is already on
    INT32 id = scanTagName(desc);
    if (id>=0)
    {
        return (id);
    }

    // 2) if not put it now
    id = addTagDescription(desc,t,base);
    //printf("&&& tryToAlloc called on %s, id=%d\n",desc.latin1(),id);fflush(stdout);fflush(stderr);
    return (id);
}

/**
 * Returns the long description of the tag.
 *
 * @return the description.
 */
QString 
TagDescVector::getTagLongDesc(QString tgName)
{
    QString* result = longHash->find(tgName);
    if (result==NULL)
    {
        return QString::null;
    }
    else
    {
        return *result;
    }
}

/**
 * Sets a long description of the tag description.
 *
 * @return void.
 */
void    
TagDescVector::setTagLongDesc(QString tgName, QString ldesc)
{
    longHash->insert(tgName,new QString(ldesc));
}

/**
 * Returns the value in a string with the format of the tag.
 *
 * @return the formatted value.
 */
QString
TagDescVector::getFormatedTagValue(UINT16 tagId, UINT64 value)
{
    static char tmpstr[128];

    TagValueType tgvtype = getTagValueType(tagId);
    INT16 tgbase = getTagValueBase(tagId);

    if (tgvtype==TagStringValue)
    {
        return strtbl->getString(value);
    }

    if (tgbase==16)
    {
        sprintf(tmpstr,FMTP64X,value);
        return QString(tmpstr);
    }

    if (tgbase==10)
    {
        sprintf(tmpstr,FMT64U,value);
        return QString(tmpstr);
    }

    if (tgbase==2)
    {
        char* tmp = DBItoa::bitstring(value,64,64);
        return QString(tmp)+"B";
    }

    if (tgbase==8)
    {
        sprintf(tmpstr,FMTP64O,value);
        return QString(tmpstr);
    }

    return (QString("Invalid base or tag type"));
}

#endif
