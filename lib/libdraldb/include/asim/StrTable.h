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
  * @file StrTable.h
  */

#ifndef _DRALDB_STRTABLE_H
#define _DRALDB_STRTABLE_H

// standard C
#include <stdio.h>
#include <stdlib.h>

// QT library
#include <qstring.h>
#include <qdict.h>
#include <qintdict.h>

#include "asim/draldb_syntax.h"
#include "asim/AMemObj.h"
#include "asim/DralDBDefinitions.h"
#include "asim/StatObj.h"

/**
  * @brief
  * This class holds a mapping between strings and integers.
  *
  * @description
  * We keep a huge amount of symbols (string) and this table avoid
  * repeated strings to spend memory space. This class has just one
  * object instanstiated, and the other classes have access to it
  * calling the getInstance function.
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */

class StrTable : public AMemObj, public StatObj
{
    public:
        // ---- AMemObj Interface methods
        virtual INT64 getObjSize() const;
        virtual QString getUsageDescription() const;
        // ---- StatObj Interface methods
        QString getStats() const;
        // -----------------------------------------------

    public:
        static StrTable* getInstance ();
        static StrTable* getInstance (INT32 sz);
        static void destroy();

    public:
        inline INT32 addString(QString);
        inline QString getString(INT32);
        inline bool hasString(QString);

        void reset();
        void resize(INT32 newsize);

    protected:
        StrTable(INT32 sz);
        virtual ~StrTable();
        void init(INT32 sz);

        inline INT32 lookForStr(QString);

    private:
        QDict<INT32>*      strhash; // Hash from string to int.
        QIntDict<QString>* idxhash; // Hash from int to string.

        INT32 nextIdx;
        INT32 primeSize;

    private:
        static StrTable* _myInstance;
};

/**
 * Returns the string mapped to the integer idx. If no string is
 * mapped to this integer a null is returned.
 *
 * @return the string.
 */
QString
StrTable::getString(INT32 idx)
{
    QString *str = idxhash->find((long)idx);
    if (str==NULL)
    {
        return (QString::null);
    }
    else
    {
        return (*str);
    }
}

/**
 * Adds a string to the table and returns the index mapped to the
 * string.
 *
 * @return the indexed mapped to the string.
 */
INT32 
StrTable::addString(QString str)
{
    // look for the string to avoid redundant entries
    INT32* idx_ptr = strhash->find(str);

    if (idx_ptr == NULL) // not found
    {
        idx_ptr = new INT32;
        Q_ASSERT(idx_ptr!=NULL);
        *idx_ptr = nextIdx;
        strhash->insert(str,idx_ptr);
        QString *tmpstr = new QString(str);
        Q_ASSERT(tmpstr!=NULL);
        idxhash->insert((long)nextIdx,tmpstr);
        nextIdx++;
    }
    return (*idx_ptr);
}

/**
 * Looks for the integer mapped to the string str.
 *
 * @return the index mapped to the string.
 */
INT32
StrTable::lookForStr(QString str)
{
   INT32* idx_ptr = strhash->find(str);
   if (idx_ptr == NULL)
   {
       return (-1);
   }
   else
   {
       return (*idx_ptr);
   }
}

/**
 * Looks if the string is already in the table.
 *
 * @return true if the index is in the table. False otherwise.
 */
bool
StrTable::hasString(QString str)
{
    return (lookForStr(str)>=0);
}

#endif
