/**************************************************************************
 *Copyright (C) 2005-2006 Intel Corporation
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
 * @file dralClientBinary_v4.cpp
 * @author Pau Cabre
 * @brief dral client binary version 1 implementation
 */

#include "asim/dralClientBinary_v4.h"

/**
 * Constructor of the ascii specific implementation of the dral client
 * it only invokes the generic implementation constructor
 */
DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS::DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS(
    DRAL_BUFFERED_READ dral_read,
    DRAL_LISTENER listener)
    : DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS (dral_read,listener)
{
}

DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS::~DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS()
{
}

bool
DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS::Comment()
{
    char * buffer2;
    UINT32 magic_num;
    UINT32 length;

    struct commentFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT32 strlen;
        UINT32 magic_num;
    } * commandComment;

    commandComment = (commentFormat *) ((char *) ReadBytes(sizeof(commentFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    if (commandComment->strlen == 0)
    {
        dralListener->NonCriticalError(
            "Found a comment command with comment length == 0");
        return true;
    }

    magic_num=commandComment->magic_num;
    length=commandComment->strlen;
    buffer2 = (char *) ReadBytes(commandComment->strlen);
    if(EOS)
    {
        return false;
    }

    dralListener->Comment(magic_num,buffer2);

    return true;
}

bool
DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS::NewStringValue()
{
    struct newStringEntryFormat
    {
        UINT8 commandCode : 6;
        UINT8 reserved    : 2;
        UINT16 str_len;
        UINT16 str_id;
    } * stringEntry;

    stringEntry = (newStringEntryFormat *) ((char *) ReadBytes(sizeof(newStringEntryFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT16 str_len = stringEntry->str_len;
    UINT16 str_id = stringEntry->str_id;

    const char * str = (const char *) ReadBytes(str_len);

    if(EOS)
    {
        return false;
    }

    strings[str_id] = getStringIndex(str, str_len);

    return true;
}
