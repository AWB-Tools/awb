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
  * @file  ZipObject.h
  */

#ifndef _ZIPOBJECT_H
#define _ZIPOBJECT_H

/**
  * @brief
  * This interface defines a method used to compress objects.
  *
  * @description
  * The method defined is used to compress the object that inherits
  * from this interface.
  *
  * @version 0.1
  * @date started at 2003-06-05
  * @author Federico Ardanaz
  */

class ZipObject
{
    public:
        virtual ZipObject* compressYourSelf(INT32 cycle, bool last=false) = 0;
};

#endif
