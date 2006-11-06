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
  * @file Hash6431.cpp
  */

#include "asim/Hash6431.h"

/**
 * Creator of this class. Creates an array of _buckets objects.
 *
 * @return new object.
 */
Hash6431::Hash6431(INT32 _buckets)
{
    buckets = _buckets;
    bvector = new Hash6431Node[buckets];
    Q_ASSERT(bvector!=NULL);
}

/**
 * Destructor of this class. Deletes the array.
 *
 * @return destroys the object.
 */
Hash6431::~Hash6431()
{
    delete [] bvector;
}

/**
 * Clears all the contents of the hash.
 *
 * @return void.
 */
void
Hash6431::clear()
{
    for (int i=0;i<buckets;i++)
    {
        if ((bvector[i].used) && (bvector[i].next!=NULL))
        {
            Hash6431Node* cnode = bvector[i].next;
            Hash6431Node* nnode = NULL;
            // Go until the last collision node.
            while (cnode!=NULL)
            {
                nnode = cnode->next;
                delete cnode;
                cnode = nnode;
            }
            bvector[i].next = NULL;
        }
        bvector[i].used = 0;
    }
}
