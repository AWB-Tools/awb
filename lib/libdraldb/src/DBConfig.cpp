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
  * @file  DBConfig.cpp
  */

#include "asim/DBConfig.h"

/**
 * The instance is NULL at the beginning.
 */
DBConfig* DBConfig::_myInstance = NULL;

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer.
 *
 * @return the instance of the class.
 */
DBConfig*
DBConfig::getInstance()
{
    if (_myInstance==NULL)
    {
        _myInstance = new DBConfig();
    }
    return _myInstance;
}

/**
 * Destroys the unique instance of the class. The instance is
 * destroyed if it was previously created.
 *
 * @return void.
 */
void
DBConfig::destroy()
{
    if (_myInstance!=NULL)
    {
        delete _myInstance;
    }
}

/**
 * Creator of this class. Just call reset.
 *
 * @return new object.
 */
DBConfig::DBConfig()
{
    reset();
}

/**
 * Destructor of this class. Nothing is done.
 *
 * @return destroys the object.
 */
DBConfig::~DBConfig()
{
}

/**
 * Sets the default configuration values.
 *
 * @return void.
 */
void
DBConfig::reset()
{
    autoPurge=true;
    compressMutable=true;
    incrementalPurge=false;
    maxIFIEnabled=false;
    tagBackPropagate=false;
    guiEnabled=false;
    itemMaxAge=0;
    maxIFI=0;
}
