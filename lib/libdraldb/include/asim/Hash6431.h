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
  * @file  Hash6431.h
  */

#ifndef _DRALDB_HASH6431_H
#define _DRALDB_HASH6431_H

#include <qnamespace.h>

#include "asim/draldb_syntax.h"

class Hash6431;

class Hash6431Node
{
    public:
        /**
         * Creates a void hash node.
         *
         * @return the new object.
         */
        Hash6431Node()
        {
            next=NULL;
            key=0;
            value=0;
            used=0;
        }

        /**
         * Nothing done.
         *
         * @return destroys the object.
         */
        ~Hash6431Node()
        {
        }

    protected:
        Hash6431Node* next; // Pointer to the next collision node.
        UINT64 key; // Key of the entry.
        INT32  value : 31; // Value of the entry.
        INT32  used  : 1; // If the node is occupied or not.

    friend class Hash6431;
};

/**
  * @brief
  * Integer hash table that maps 64 bit integers to 32 bits.
  *
  * @description
  * The hash is implemented using a collision list when two
  * different entries map to the same bucket. The hashing funciton
  * is just a mod.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class Hash6431
{
    public:
        Hash6431(INT32 buckets);
        ~Hash6431();

        inline INT32 find(UINT64 key);
        inline void  insert(UINT64 key, INT32 value);

        void clear();
    private:
        Hash6431Node* bvector; // Pointer to the hash nodes.
        INT32         buckets; // Number of buckets of the hash.
};

/**
 * This function hashes the key and searches through the collision
 * list of the hashed value until it finds the node that has the
 * value for the key. If the key is not found a -1 is returned.
 *
 * @return the value mapped to the key.
 */
INT32
Hash6431::find(UINT64 key)
{
    INT32 lastValue=-1;
    INT32 bk = key % buckets;
    Hash6431Node* current = &(bvector[bk]);
    if (! (current->used) )
    {
        return lastValue;
    }

    bool fnd = false;
    // Until the key is found or the end of the list is reached.
    while (!fnd && (current !=NULL))
    {
        fnd = (current->used) && (current->key == key);
        lastValue = current->value;
        current = current->next;
    }
    if (!fnd)
    {
        lastValue = -1;
    }
    return lastValue;
}

/**
 * Inserts a value associated to the key in the hash. If the bucket
 * of the hashed key is already occupied the entry is stored at the
 * end of the list.
 *
 * @return void.
 */
void
Hash6431::insert(UINT64 key, INT32 value)
{
    INT32 bk = key % buckets;
    Hash6431Node* current = &(bvector[bk]);
    Hash6431Node* prev = current;
    // If the bucket is already used.
    if (current->used)
    {
        // Goes to the end of the collision list.
        while (current!=NULL)
        {
            prev = current;
            current = current->next;
        }
        // Allocate a new node.
        current = new Hash6431Node;
        prev->next = current;
    }
    current->used = 1;
    current->key = key;
    current->value = value;
}

#endif
