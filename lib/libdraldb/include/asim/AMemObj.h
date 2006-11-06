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
  * @file  AMemObj.h
  */

#ifndef _AMEMOBJ_H
#define _AMEMOBJ_H

// QT library
#include <qstring.h>

#include "asim/draldb_syntax.h"

/**
  * @brief
  * This interface defines a few methods used to gather information
  * about memory usage.
  *
  * @description
  * Any (non size critical) dynamically allocated object in an AGT
  * application (2Dreams,etc) must inherit from AMemObj. This is a
  * simple mechanism to be able to know (in detail) the amount of
  * dynamically allocated memory used at any moment.
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class AMemObj
{
    public:

        /**
         * A simple object would just return sizeof(this)a composite
         * one is responsable of adding the size of its subcomponents.
         *
         * @return the size of the object.
         */
        virtual INT64 getObjSize() const = 0;

        /**
         * To know if all internal dynamically allocated objects were
         * sucessfully allocated:
         *
         * @return success allocate.
         */
        //virtual bool sucessfullAllocated() const = 0;

        /**
         * Textual (detailed) description of the memory usage this
         * make sense mainly for big composite objects.
         *
         * @return the description .
         */
        virtual QString getUsageDescription() const;
};

#endif
