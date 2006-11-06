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
 * @file dralClientImplementation.h
 * @author Pau Cabre
 * @brief dral client implementation interface
 */


#ifndef DRAL_CLIENT_IMPLEMENTATION_H
#define DRAL_CLIENT_IMPLEMENTATION_H

#include <sstream>
#include <string.h>
#include <assert.h>
#include "asim/dral_syntax.h"
#include "asim/dralRead.h"
#include "asim/dralStringMapping.h"

using namespace std;

/**
 * implementation class interface
 * The client may have different implementations, according
 * to the type of the data read (ASCII or BINARY)
 */
class DRAL_CLIENT_IMPLEMENTATION_CLASS
{
  public:
    DRAL_CLIENT_IMPLEMENTATION_CLASS(DRAL_BUFFERED_READ dral_read, DRAL_LISTENER dralListener);
    virtual ~DRAL_CLIENT_IMPLEMENTATION_CLASS();

    virtual UINT16 ProcessNextEvent (bool blocking, UINT16 num_events)=0;

    INT64 GetFileSize(void);
    
    UINT64 GetNumBytesRead(void);

  protected:

    /*
     * a pointer to the class that performs buffered read
     */
    DRAL_BUFFERED_READ dralRead;

    /*
     * pointer to the listener
     */
    DRAL_LISTENER dralListener;
    

    /**
     * boolean used to know if an error has been found
     */
    bool errorFound;

    /*
     * stores the tags.
     */
    DRAL_STRING_MAPPING_CLASS tag_map;

    /*
     * stores the strings.
     */
    DRAL_STRING_MAPPING_CLASS str_val_map;

    /*
     * functions that map between strings and indexes.
     */
    UINT32 getTagIndex(const char * tag, UINT16 tag_len);
    UINT32 getStringIndex(const char * str, UINT16 str_len);

};

typedef DRAL_CLIENT_IMPLEMENTATION_CLASS * DRAL_CLIENT_IMPLEMENTATION;


#endif /* DRAL_CLIENT_IMPLEMENTATION_H */
