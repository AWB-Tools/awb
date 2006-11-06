/*****************************************************************************
 *
 * @brief Singleton to keep tracking of the R$ lines. 
 *
 * @author Oscar Rosell
 *
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

#ifndef MEMORY_LINE_MANAGER_H 
#define MEMORY_LINE_MANAGER_H
#include "asim/item.h"
#include <map>

class MEMORY_LINE_MANAGER
{
  public:
    static MEMORY_LINE_MANAGER& GetInstance()
    {
        static MEMORY_LINE_MANAGER the_manager;
        return the_manager;
    }

    static ASIM_ITEM 
    GetItem(UINT64 address)
    {
        std::map<UINT64, ASIM_ITEM>::const_iterator it = GetInstance().addr2item_.find(address);
        if (it != GetInstance().addr2item_.end())
        {
            // Found
            return it->second;
        }
        else
        {
            // Create it
            GetInstance().addr2item_[address] = new ASIM_ITEM_CLASS();
            return GetInstance().addr2item_[address];
        }
    }
  private:
    MEMORY_LINE_MANAGER()
    {
    };
    ~MEMORY_LINE_MANAGER()
    {
    };

    std::map<UINT64, ASIM_ITEM> addr2item_;
};
#endif
