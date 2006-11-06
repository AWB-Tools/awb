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
  * @file  StatObj.h
  */

#ifndef _StatObj_H
#define _StatObj_H

// QT library
#include <qstring.h>

#include "asim/draldb_syntax.h"

/**
  * @brief
  * This interface defines a few methods used to gather statistic
  * information about system components.
  *
  * @description
  * Just a virtual function is defined inside this class. When is
  * called the class that inherits from this interface must return
  * the statistics with a QString.
  *
  * @version 0.1
  * @date started at 2003-03-11
  * @author Federico Ardanaz
  */
class StatObj
{
    public:
        virtual QString getStats() const;
};

#endif
