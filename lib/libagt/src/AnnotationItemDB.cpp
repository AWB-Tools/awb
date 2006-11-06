/*
 *Copyright (C) 2003-2006 Intel Corporation
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
  * @file  AnnotationItemDB.cpp
  */

#include "AnnotationItemDB.h"

AnnotationItemDB::AnnotationItemDB()
{
    hash = new QPtrDict<AnnotationItem>(137);
    Q_ASSERT(hash!=NULL);
}

AnnotationItemDB::~AnnotationItemDB()
{
    delete hash;
}

void 
AnnotationItemDB::reset()
{
    hash->clear();
}

void
AnnotationItemDB::add (AnnotationItem* item)
{
    //printf("AnnotationItemDB::add called with 0x%x\n",(void*)item);fflush(stdout);
    hash->insert(item,item);
}

bool
AnnotationItemDB::remove (AnnotationItem* item, bool destroy)
{
    bool ok = hash->remove(item);
    if (destroy && ok)
    {
        delete item;
    }
    return (ok);
}


QPtrDictIterator<AnnotationItem>
    AnnotationItemDB::getIterator()
{
    return QPtrDictIterator<AnnotationItem>( *hash );
}

