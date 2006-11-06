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
 * @file dralClientBinary_v4.h
 * @author Federico Ardanaz 
 * @brief dral client binary defines for dral version 4, this is like version 3 without limiting strings to 256 bytes length
 */

#ifndef DRAL_CLIENT_BINARY_V4_H
#define DRAL_CLIENT_BINARY_V4_H

#include "asim/dralClientBinary_v3.h"

/**
 * BINARY client version 4 implementation class
 */
class DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS
    : public DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS
{
  public:

    DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS (
        DRAL_BUFFERED_READ dral_read, DRAL_LISTENER dralListener);
    
    virtual ~DRAL_CLIENT_BINARY_4_IMPLEMENTATION_CLASS ();

  protected:

    bool NewStringValue ();
    bool Comment ();
};

#endif /* DRAL_CLIENT_BINARY_V4_H */
