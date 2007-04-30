/*
 * Copyright (C) 2003-2006 Intel Corporation
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
 */


/*****************************************************************************
*
* @brief Header file for thread safe Cache Manager
*
* @author Carl Beckmann
*
*****************************************************************************/

/*
*   This class is to avoid ending with the same line in 2+ caches 
*   under a multisocket environment.
*   The cache manager should be used for tracking line state in all the caches.
*
*   This version is thread safe, i.e. you can call it from modules running on
*   different threads on the simulator host machine.  This is typically the case
*   if you run a multisocket model, and have different threads assigned to each socket.
*
*   This implementation assumes that although the cache manager might be accessed
*   by different threads, individual lines are always accessed only by a single thread.
*   There is no locking around cache line data structures, only around the cache
*   manager accesses that get you to the line.  (To minimize lock holding time)
*/

#ifndef CACHE_MANAGER_SMP_H
#define CACHE_MANAGER_SMP_H

#include "asim/cache_manager.h"
#include "asim/smp.h"

class CACHE_MANAGER_SMP : public CACHE_MANAGER
{
  protected:
    pthread_mutex_t mutex;       ///< a lock for thread-safe access to cache manager data structures

    CACHE_MANAGER_SMP();         ///< constructor and destructor are private since this is a singleton
    ~CACHE_MANAGER_SMP();
  public:
    // this returns the singleton instance of the thread-safe cache manager
    static CACHE_MANAGER& GetInstance();
    
    // the following functions are reimplemented for thread safety
    virtual void Register(std::string level);
  protected:
    virtual LINE_MANAGER *find_line_manager(std::string level);
};

#endif
