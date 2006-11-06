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
  * @file PrimeList.h
  */

#ifndef _DRALDB_PRIMELIST_H
#define _DRALDB_PRIMELIST_H

#include "asim/draldb_syntax.h"

/**
  * @brief
  * This class is used to find primes near a value.
  *
  * @description
  * The only content in this class is a static function that search
  * the first bigger prime in the list. All the primes are defined
  * in a list inside the function.
  *
  * @link http://www.mathematical.com/primelist1to100kk.html
  * @author Federico Ardanaz
  * @date started at 2002-12-1
  * @version 0.1
  */
class PrimeList
{
    public:
       static inline INT32 nearPrime(INT32 size);
};

/**
 * This function finds a prime that is bigger than the parameter
 * size. If no bigger prime is found a 9999991 is returned.
 *
 * @return a near primer.
 */
INT32
PrimeList::nearPrime(INT32 size)
{
    INT32 primeArray[] =
    {
        17,59,97,151,197,257,317,419,521,617,709,811,911,
        1009,1499,2003,2503,3001,3499,
        4001,4507,5003,5501,6007,6521,
        7001,7499,8009,8501,9001,9497,
        10007,20011,30011,
        40009,50021,60013,
        70001,80021,90001,
        100003,150001,
        200003,250007,
        300089,350003,
        400009,450001,
        500009,550007,
        600011,650011,
        700001,750019,
        800011,850009,
        900001,950009,
        1000003,1200007,1300021,
        1400017,1500007,1600033,
        1700021,1800017,1900009,
        2000003,
        3000017,
        4500007,4600003,4700021,4800007,4900001,
        5000011,5500003,
        6000011,
        7000003,
        8000009,
        9000011,
        9900047,
        -1
    };
    INT32 result = -1;
    bool fnd = false;
    INT32 idx = 0;

    while (!fnd && primeArray[idx]>0)
    {
        fnd = primeArray[idx]>=size;
        ++idx;
    }
    if (fnd)
    {
        result = primeArray[idx-1];
    }
    else
    {
        // a very big one!
        result = 9999991;
    }

    //printf ("nearPrime called width %d result=%d\n",size,result);fflush(stdout);
    return (result);
}

#endif
