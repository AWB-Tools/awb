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

#ifndef DRAL_CLIENT_ASCII_V0_1_H
#define DRAL_CLIENT_ASCII_V0_1_H

#include <string.h>
using namespace std;

#include "asim/dralClientImplementation.h"

#define DRAL_NO_FLAGS       0 /* 0000b */
#define DRAL_FLG_IMMUTABLE  1 /* 0001b */
#define DRAL_FLG_PAST       2 /* 0010b */
#define DRAL_FLG_CURRENT    4 /* 0100b */
#define DRAL_FLG_FUTURE     8 /* 1000b */

/**
 * ASCII client specific implementation class
 * It works with all the version numbers, since the ascii format has not
 * changed
 */
class DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS
    : public DRAL_CLIENT_IMPLEMENTATION_CLASS
{
  public:

    DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS (
        DRAL_BUFFERED_READ dral_read, DRAL_LISTENER dralListener);

    UINT16 ProcessNextEvent (bool blocking, UINT16 num_events);

  private:

    /**
     * Private method used to fill the buffer with a line read
     * from the file descriptor
     * All the lines with a command have to end with a '\n' character
     * It returns false if any error found
     */
    bool Get_line (char ** buffer, bool * end_of_file);

    /**
     * Private method used to convert the time_span string
     * format from the string format to the unsigned byte format
     */
    unsigned char TimeSpanStrToByte(char * s);


    /**
     * Private methods to proccess theses specific events
     */
    void SetTagSingleValue (istringstream * s);
    void SetTagString (istringstream * s);
    void SetTagSet (istringstream * s, char * b);
    void MoveItems (istringstream * s, char * b);
    void SetCapacity (istringstream * s, char * b);



    /**
     * This private method  counts how many times the character c is
     * found in the string.
     * It is used to know how many elements are in a command
     */
    UINT16 HowManyElements(char * buffer, char c);

    bool firstCycle;

};



#endif /* DRAL_CLIENT_ASCII_V0_1_H */
