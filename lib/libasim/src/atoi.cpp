/*
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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

// ASIM core
#include "asim/syntax.h"

#include "asim/atoi.h"


INT64 
atoi_general(const char* buf, int mul)
{

    /*
      mul should be 1000 or 1024
     */

    const char* p = buf;
    while(*p && isspace(*p))
    {
        p++;
    }
    // exclude hex; octal works just fine
    const char* q = p;
    if (*q == '-' || *q == '+')
    {
        q++;
    }
    if (*q=='0' && (q[1]=='x' || q[1]=='X'))
    {
        return strtoll(buf,0,0);
    }

    INT64 b = strtoll(buf,0,0);


    if (p)
    {
        while(*p && (*p == '-' || *p == '+'))
        {
            p++;
        }
        while(*p && isdigit(*p))
        {
            p++;
        }

        if (*p != 0)
        {
            if (*p == 'k' || *p == 'K')
            {
                b = b * mul;
            }
            else if (*p == 'm' || *p == 'M')
            {
                b = b * mul * mul;
            }
            else if (*p == 'g' || *p == 'G' || *p == 'b' || *p == 'B')
            {
                b = b * mul * mul * mul;
            }
        }
    }
    return b;
}

UINT64 
atoi_general_unsigned(const char* buf, int mul)
{

    /*
      mul should be 1000 or 1024
     */

    const char* p = buf;
    while(*p && isspace(*p))
    {
        p++;
    }
    // exclude hex; octal works just fine
    const char* q = p;
    if ( *q == '+')
    {
        q++;
    }
    if (*q=='0' && (q[1]=='x' || q[1]=='X'))
    {
        return strtoull(buf,0,0);
    }

    UINT64 b = strtoull(buf,0,0);

    if (p)
    {
        while(*p && ( *p == '+'))
        {
            p++;
        }
        while(*p && isdigit(*p))
        {
            p++;
        }

        if (*p != 0)
        {
            if (*p == 'k' || *p == 'K')
            {
                b = b * mul;
            }
            else if (*p == 'm' || *p == 'M')
            {
                b = b * mul * mul;
            }
            else if (*p == 'g' || *p == 'G' || *p == 'b' || *p == 'B')
            {
                b = b * mul * mul * mul;
            }
        }
    }
    return b;
}

INT64 
atoi_general(const char* buf)
{
  return atoi_general(buf, 1024);
}
INT64 
atoi_general(const std::string& str)
{
  return atoi_general(str.c_str(), 1024);
}

UINT64 
atoi_general_unsigned(const char* buf)
{
  return atoi_general_unsigned(buf, 1024);
}
UINT64 
atoi_general_unsigned(const std::string& str)
{
  return atoi_general_unsigned(str.c_str(), 1024);
}
//Local Variables:
//pref: "../include/asim/atoi.h"
//End:
