/**************************************************************************
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
 * @file dralClientImplementation.cpp
 * @author Pau Cabre
 * @brief dral client implemntation base
 */
 
#include "asim/dralClientImplementation.h"

/**
 * Constructor with the file descriptor from where the data will be read,
 * the listener and the read buffer size
 */
DRAL_CLIENT_IMPLEMENTATION_CLASS::DRAL_CLIENT_IMPLEMENTATION_CLASS (
    DRAL_BUFFERED_READ dral_read, DRAL_LISTENER listener)
// Warning!!!
// This values aren't a bug. We need to assign big maximum values to this
// mappings.
    : tag_map(1024 * 1024 * 1024), str_val_map(1024 * 1024 * 1024)
{
    dralListener = listener;
    errorFound = false;
    dralRead = dral_read;
}

DRAL_CLIENT_IMPLEMENTATION_CLASS::~DRAL_CLIENT_IMPLEMENTATION_CLASS()
{
}

INT64
DRAL_CLIENT_IMPLEMENTATION_CLASS::GetFileSize(void)
{
    return dralRead->GetFileSize();
}

UINT64
DRAL_CLIENT_IMPLEMENTATION_CLASS::GetNumBytesRead(void)
{
    return dralRead->GetNumBytesRead();
}

UINT32
DRAL_CLIENT_IMPLEMENTATION_CLASS::getTagIndex(const char * tag, UINT16 tag_len)
{
    UINT32 index;

    if(tag_map.getMapping(tag, tag_len, &index))
    {
        dralListener->NewTag(index, tag, tag_len);
    }

    return index;
}

UINT32
DRAL_CLIENT_IMPLEMENTATION_CLASS::getStringIndex(const char * str, UINT16 str_len)
{
    UINT32 index;

    if(str_val_map.getMapping(str, str_len, &index))
    {
        dralListener->NewString(index, str, str_len);
    }

    return index;
}
