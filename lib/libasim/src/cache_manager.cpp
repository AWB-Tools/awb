/*****************************************************************************
*
* @brief Source file for Cache Manager
*
* @author Oscar Rosell
*
* Copyright (C) 2005-2006 Intel Corporation
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
* 
*
*****************************************************************************/

#include "asim/cache_manager.h"
#include "asim/mesg.h"
#include <iostream>

using namespace std;

LINE_STATUS
CACHE_MANAGER::LINE_MANAGER::LINE_STATE::GetStatus() const
{
    ASSERTX(state_.size() > 0);

    // FIXME I choose to stay on the safe side
    if (state_.size() == 1)
    {
        return S_EXCLUSIVE;
    }
    else
    {
        return S_SHARED;
    }
}

LINE_STATUS
CACHE_MANAGER::LINE_MANAGER::LINE_STATE::GetStatus(UINT32 owner) const
{
    map<UINT32, LINE_STATUS>::const_iterator it = state_.find(owner);
    if (it != state_.end())
    {
        return it->second;
    }
    else
    {
        return S_INVALID;
    }
}

void 
CACHE_MANAGER::LINE_MANAGER::LINE_STATE::SetStatus(UINT32 owner, LINE_STATUS status)
{
    state_[owner] = status;
    // FIXME The line deallocation is deactivated by default and the setClearLines() cache_manager
    // method should be called to activate it. However, with it activated we have race problems
    // on the clients when running with asim/cache.h warming activated.
    if ( ((status == S_INVALID) || (status == S_RESERVED)) &&
         CACHE_MANAGER::GetInstance().getClearLines() )
    {
        state_.erase(owner);
    }
    
}

CACHE_MANAGER::LINE_MANAGER::LINE_MANAGER()
{ }

CACHE_MANAGER::LINE_MANAGER::~LINE_MANAGER()
{ }

LINE_STATUS
CACHE_MANAGER::LINE_MANAGER::GetStatus(ADDRESS address) const
{
    map<ADDRESS, LINE_STATE>::const_iterator it = addr2status_.find(address);

    LINE_STATUS ret_status = S_INVALID;
    if (it != addr2status_.end())
    {
        // Found
        ret_status = it->second.GetStatus();
    }

    return ret_status;
}


LINE_STATUS
CACHE_MANAGER::LINE_MANAGER::GetStatus(UINT32 owner, ADDRESS address) const
{
    std::map<ADDRESS, LINE_STATE>::const_iterator it = addr2status_.find(address);

    LINE_STATUS ret_status = S_INVALID;
    if (it != addr2status_.end())
    {
        // Found
        ret_status = it->second.GetStatus(owner);
    }

    return ret_status;
}

void
CACHE_MANAGER::LINE_MANAGER::SetStatus(UINT32 owner, ADDRESS address, LINE_STATUS status)
{
    // Look for it. Implicitly create it if it doesn't exist
    LINE_STATE& state = addr2status_[address];
    state.SetStatus(owner, status);
    if (!state.IsValid())
    {
        // No cache has it in a valid state -> remove the entry
        addr2status_.erase(address);
    }    
}

CACHE_MANAGER::CACHE_MANAGER():
    clear_lines(false),
    activated(true)
{}

CACHE_MANAGER::~CACHE_MANAGER()
{}

CACHE_MANAGER&
CACHE_MANAGER::GetInstance()
{
    static CACHE_MANAGER the_manager;
    return the_manager;
}

void
CACHE_MANAGER::Register(std::string level)
{
    // Create new manager if it doesn't exist
    str2manager_[level];
}

LINE_STATUS
CACHE_MANAGER::GetStatus(std::string level, UINT32 index, UINT64 tag) const
{
    map<std::string, LINE_MANAGER>::const_iterator it = str2manager_.find(level);
    if (it != str2manager_.end())
    {
        return it->second.GetStatus(std::pair<UINT32, UINT64>(index, tag));
    }
    else
    {
        return S_INVALID;
    }
}


LINE_STATUS
CACHE_MANAGER::GetStatus(std::string level, UINT32 owner, UINT32 index, UINT64 tag) const
{
    map<std::string, LINE_MANAGER>::const_iterator it = str2manager_.find(level);
    if (it != str2manager_.end())
    {
        return it->second.GetStatus(owner, std::pair<UINT32, UINT64>(index, tag));
    }
    else
    {
        return S_INVALID;
    }
}

void
CACHE_MANAGER::SetStatus(std::string level, UINT32 owner, UINT32 index, UINT64 tag, LINE_STATUS status)
{
    if(!activated) return;
    
    map<std::string, LINE_MANAGER>::iterator it = str2manager_.find(level);
    if (it != str2manager_.end())
    {
        it->second.SetStatus(owner, std::pair<UINT32, UINT64>(index,tag), status);
    }
}

