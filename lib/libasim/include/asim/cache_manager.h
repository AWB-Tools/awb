/*****************************************************************************
*
* @brief Header file for Cache Manager
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

/*
*   This class is to avoid ending with the same line in 2+ caches 
*   under a multisocket environment.
*   The cache manager should be used for tracking line state in all the caches.
*  
*/

#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include "asim/line_status.h"
#include "asim/syntax.h"
#include <map>
#include <string>
#include <utility>

class CACHE_MANAGER
{
  protected:
    class LINE_MANAGER
    {
        struct LINE_STATE
        {
            LINE_STATUS GetStatus() const;
            LINE_STATUS GetStatus(UINT32 owner) const;
            void SetStatus(UINT32 owner, LINE_STATUS status);
            bool IsValid() const
            {
                return !state_.empty();
            };

            std::map<UINT32 /* Owner */, LINE_STATUS /* Status */> state_;
        };

      public:
        typedef std::pair<UINT32 /* Index */, UINT64 /* Tag */> ADDRESS;

        LINE_MANAGER();
        ~LINE_MANAGER();

        LINE_STATUS GetStatus(ADDRESS address) const;
        LINE_STATUS GetStatus(UINT32 owner, ADDRESS address) const;
        void SetStatus(UINT32 owner, ADDRESS address, LINE_STATUS status);
        
      private:
        std::map<ADDRESS, LINE_STATE> addr2status_;
        std::string level_;
    };

    typedef LINE_MANAGER::ADDRESS ADDRESS;

    CACHE_MANAGER();
    virtual ~CACHE_MANAGER();

  public:
    static CACHE_MANAGER& GetInstance();
    virtual void Register(std::string level);

    LINE_STATUS GetStatus(std::string level, UINT32 index, UINT64 tag);
    LINE_STATUS GetStatus(std::string level, UINT32 owner, UINT32 index, UINT64 tag);
    void SetStatus(std::string level, UINT32 owner, UINT32 index, UINT64 tag, LINE_STATUS status);
    
  private:
    std::map<std::string, LINE_MANAGER> str2manager_;
    
    bool clear_lines;
    bool activated;
  
  protected:
    virtual LINE_MANAGER *find_line_manager(std::string level);

  public:
    void setClearLines() { clear_lines = true; }
    const bool getClearLines() { return clear_lines; }
    
    void deactivate() { activated = false; }

};

#endif
