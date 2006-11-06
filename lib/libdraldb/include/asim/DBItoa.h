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
  * @file DBItoa.h
  */

#ifndef _DRALDB_DBITOA_H_
#define _DRALDB_DBITOA_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "asim/draldb_syntax.h"

/**
  * @brief
  * Define some helper methods for 64 bits string to UINT64
  * conversion and vice-versa.
  *
  * @description
  * Implements the conversion instructions for 64 bit integer
  * and bit fields.
  *
  * @version 0.2
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class DBItoa
{
    public:
        /**
         * Converts a string to an unsigned int of 64 bits.
         *
         * @return the conversion of the string.
         */
        static inline UINT64 str2uint64(const char *str,bool& parseok)
        {
            UINT64 result;

            int ok=0;
            parseok = false;

            const char *pstr = str;
            while (isspace(*pstr)) ++pstr;
            if ( *pstr == '\0')
            {
                return 0;
            }

            // base 16 ?
            if ((pstr[0]=='0') && (toupper(pstr[1])=='X') )
            {
                ok = sscanf(pstr,FMT64X,&result);
            }

            // base 8 ?
            if ((ok==0) && (pstr[0]=='0') )
            {
                ok = sscanf(pstr,FMT64O,&result);
            }

            // base 10 ?
            if (ok==0)
            {
            ok = sscanf(pstr,FMT64U,&result);
            }
            parseok = (ok>0);
            return (UINT64)result;
        }

        /**
         * Converts unsigned int of 64 bits to a string.
         *
         * @return the conversion of the integer.
         */
        static inline char* uint642str(UINT64 value)
        {
            static char result[32];                    /// @todo This is not thread safe !!
            sprintf(result,FMT64U,value);
            return result;
        }

        /**
         * Returns a string with the binary content of value.
         *
         * @return the conversion.
         */
        static char* bitstring(UINT64 value, int count, int width)
        {
            static char result[128];                      /// @todo This is not thread safe !!
            if (width>127)
            {
                width=127;
            }

            int cnt=0;
            while (--count >= 0)
            {
                result[cnt++] = ((value >> count) & 1) + '0';
            }

            for (int i = cnt; i < width; i++)
            {
                result[cnt++] = ' ';
            }

            result[cnt] = '\0';
            return (result);
        }
};

#endif
