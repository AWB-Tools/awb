/**************************************************************************
 *Copyright (C) 2004-2006 Intel Corporation
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
 * @file dralStringMapping.cpp
 * @author Guillem Sole
 * @brief A mapping of strings.
 */

#include "asim/dralStringMapping.h"
#include <stdlib.h>
#include <iostream>
#include <assert.h>

DRAL_STRING_MAPPING_CLASS::DRAL_STRING_MAPPING_CLASS(UINT32 size)
{
    assert(size > 0);
    max_strs = size;
    commands = 0;
    conflicts = 0;
}

DRAL_STRING_MAPPING_CLASS::~DRAL_STRING_MAPPING_CLASS()
{
}

bool
DRAL_STRING_MAPPING_CLASS::getMapping(const char * str, UINT16 strlen, UINT32 * index)
{
    StringMappingIterator it_map;

    // Internal statistic update.
    commands++;

    // Tries to find the string in the mapping.
    it_map = mapping.find(str);

    if(it_map == mapping.end())
    {
        UINT32 new_index;         ///< Index that will be used.
        StringMappingEntry entry; ///< New entry of the mapping.

        // If not found, checks if we've space in the mapping.
        if(mapping.size() == max_strs)
        {
            LRUListIterator it_lru;
            StringMappingIterator it_map2;

            // Internal statistic update.
            conflicts++;

            // Gets the entry to remove.
            it_lru = lru.begin();
            it_map2 = mapping.find(* it_lru);

            // Recycles the index.
            new_index = it_map2->second.index;

            // Removes it from the mapping.
            mapping.erase(it_map2);
            lru.pop_front();
        }
        else
        {
            // The index set is equal to the size of the mapping.
            new_index = mapping.size();
        }

        // Adds the new entry in the lru and the mapping.
        lru.push_back(str);
        entry.index = new_index;
        entry.it = --lru.end();

        mapping[str] = entry;
        * index = new_index;

        return true;
    }
    else
    {
        // If found, just sets the index and returns that no new callback is needed and also
        // updates the lru state.
        lru.erase(it_map->second.it);
        lru.push_back(it_map->first);
        it_map->second.it = --lru.end();
        * index = it_map->second.index;
        return false;
    }
}

void
DRAL_STRING_MAPPING_CLASS::dump()
{
    StringMappingIterator it;
    const char ** temp;

    temp = (const char **) malloc(max_strs * sizeof(const char *));

    cout << "Dumping string map:" << endl;

    for(UINT32 i = 0; i < max_strs; i++)
    {
        temp[i] = NULL;
    }

    for(it = mapping.begin(); it != mapping.end(); it++)
    {
        temp[it->second.index] = it->first.c_str();
    }

    for(UINT32 i = 0; i < max_strs; i++)
    {
        cout << "\tEntry " << i << " is: " << temp[i] << endl;
    }
    cout << endl;

    free(temp);
}
