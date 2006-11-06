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
  * @file  Dict2064.h
  */

#ifndef _DRALDB_DICT2064_H
#define _DRALDB_DICT2064_H

// dbg
#include <stdio.h>

#include <qstring.h>

#include "asim/draldb_syntax.h"
#include "asim/Hash6431.h"

#define DICT2064_MAXDICT 128
#define DICT2064_MAXKEY  1048575
#define DICT2064_BUCKETS 1048589  // must be a prime number bigger than DICT2064_MAXKEY
// Therefore we support a max of 128*1048575= 134,217,600 different values

/**
  * @brief
  * This class maps 64 bits values with 32 bit keys.
  *
  * @description
  * The main use of this class is reduce the memory usage of the
  * program using 32 bit keys to map 64 bit values. To improve the
  * performance the mapping is stored in mutiple dictionaries of
  * DICT2064_MAXKEY entries. The problem having multiple dictionaries
  * is that the keys are repeated. To solve this the cycle is used
  * to distinguish between keys of different dictionaries (each
  * dict stores values starting at one cycle to another one).
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class Dict2064;

class Dict2064Node
{
    public:
        /**
         * Creator of this class. Sets the fields of the class.
         *
         * @return new object.
         */
        Dict2064Node()
        {
            nextKey=0;
            dict=NULL;
            cycle=-1;
        };

        /**
         * Free the dictionary array if it was allocated.
         *
         * @return destroys the object.
         */
        ~Dict2064Node()
        {
            if (dict!=NULL)
            {
                delete [] dict;
            }
        };

    protected:
        INT32   cycle; // First cycle 
        UINT64* dict; // Holds the values of this dictinary.
        INT32   nextKey; // Next entry in the dict array.

    friend class Dict2064;
};

class Dict2064
{
    public:
        static Dict2064* getInstance();
        static void destroy();

    public:
        inline INT32  getKeyFor(UINT64 value, INT32 cycle);
        inline UINT64 getValueByCycle (INT32 key, INT32 cycle);
        inline UINT64 getValueByDict  (INT32 key, INT32 dictNum);

        INT32  getNumDictionaries();

    protected:
        Dict2064();
        virtual ~Dict2064();

        inline void allocNewDict(INT32 cycle);

    private:
        INT32 nextDict; // First free position in the dicvec array.
        Dict2064Node dicvec[DICT2064_MAXDICT]; // The dictionary array.
        Hash6431 *currentDict; // I keep a hash-like dictionary for O(1) value look-up for the "current" only (last created) dictionary.

    private:
        static Dict2064* _myInstance; // Pointer to the instance of this class.
};

/**
 * Inserts a new 64 bit integer in the dictionary. When the number
 * of entries in the actual dictionary gets DICT2064_MAXKEY a new
 * one is used. This is done to avoid too many collisions inside
 * the hash. To distinguish between the same key of different
 * dictionaries the cycle is used.
 *
 * @return the key for the value.
 */
INT32
Dict2064::getKeyFor(UINT64 value, INT32 cycle)
{
    INT32 key = currentDict->find(value);
    // get a new entry then
    if (key<0) 
    {
        // 1) check for room in the current dict
        if (dicvec[nextDict-1].nextKey>DICT2064_MAXKEY)
        {
            allocNewDict(cycle);
        }

        // 2) put it, be aware that if allocNewDict() is called, nextDict is incremented!
        key = dicvec[nextDict-1].nextKey++;
        dicvec[nextDict-1].dict[key]=value;
        currentDict->insert(value,key);
    }
    return key;
}

/**
 * This functions searches the dictionary that has the values for
 * the cycle cycle and then returns the value of the key inside
 * the dictionary.
 *
 * @return the value mapped to the key key.
 */
UINT64
Dict2064::getValueByCycle (INT32 key, INT32 cycle)
{
    INT32 dictNum = 0;
    bool fnd = false;
    while ( !fnd && dictNum<nextDict )
    {
        fnd = dicvec[dictNum].cycle > cycle;
        ++dictNum;
    }
    // recover extra inc
    --dictNum; 
    if (fnd)
    { 
        --dictNum;
    }
    return dicvec[dictNum].dict[key];
}

/**
 * Simply returns the value mapped to the key key in the dictionary
 * dictNum. Asserts that the dictionary exists.
 *
 * @return the value mapped to the key key in the dict dictNum.
 */
UINT64
Dict2064::getValueByDict (INT32 key, INT32 dictNum)
{
    Q_ASSERT(dictNum<nextDict);
    return dicvec[dictNum].dict[key];
}

/**
 * Allocs a new dictionary that holds values that are set after
 * the cycle cycle.
 *
 * @return void.
 */
void
Dict2064::allocNewDict(INT32 cycle)
{
    //printf("Dict2064::allocNewDict called on cycle=%d\n",cycle);
    dicvec[nextDict].cycle = cycle;
    dicvec[nextDict].dict  = new UINT64[DICT2064_MAXKEY+1];
    Q_ASSERT(dicvec[nextDict].dict != NULL);
    ++nextDict;
    currentDict->clear();
}

#endif
