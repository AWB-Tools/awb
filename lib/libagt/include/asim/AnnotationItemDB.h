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
  * @file  AnnotationItemDB.h
  * @brief AnnotationItemDB keeps a "list" of annotation items for current DB
  */
  
#ifndef _ANNOTATIONITEMDB__H
#define _ANNOTATIONITEMDB__H

#include <qptrdict.h>

#include "agt_syntax.h"
#include "AnnotationItem.h"

/**
  * This class keeps a "list" of annotation items for current DB
  * Put long explanation here
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class AnnotationItemDB
{
    public:
       /**
        * Function description
        */
        AnnotationItemDB();

       /**
        * Function description
        */
        ~AnnotationItemDB();

       /**
        * Reset DBase
        */
        void reset();

       /**
        * Function description
        */
        void
        add (
            AnnotationItem* item
            );

       /**
        * Function description
        */
        bool
        remove (
               AnnotationItem* item,
               bool destroy
               );

       /**
        * Function description
        */
        QPtrDictIterator<AnnotationItem>
        getIterator();

        // saveToFile(FILE* f)

    protected:
        QPtrDict<AnnotationItem>* hash;
};

#endif

