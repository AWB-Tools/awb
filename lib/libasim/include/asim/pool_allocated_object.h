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
 * @file
 * @author Chris Weaver
 * @brief A template for allocating objects from a pool instead of dynamically
 */

#ifndef _POOL_ALLOCATED_OBJECTS_
#define _POOL_ALLOCATED_OBJECTS_

// generic
#include <bitset>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/ioformat.h"
#include "asim/atomic.h"
#include "asim/smp.h"


// ***************************************************************************
//
// This module has a number of problems.  You should probably think twice
// before choosing to use it.  Among the problems:
//
// - Storage returned by Allocate() is uninitialized.  The default constructor
//   is called the first time the object is allocated but the old state from
//   the last Release() is returned when the storage is reused.  It would be
//   better to replace operator new and delete for heap objects instead of
//   using this code.  That would mean resturcturing this code and all users
//   of it.
//
// - The entire storage area is treated like a queue.  The allocator rotates
//   around the entire area instead of managing a free list.  A free list
//   implementation would likely improve the cache hit rate.
//
// - Pools are separate for each thread.  While all threads may access any
//   object, the thread calling Release() must be the same thread that
//   called Allocate().  There is an assertion to verify this.
//
// - The pool size grows in proportion to MAX_PTHREADS.  Older code used to
//   keep the pool size the same and spread it across all threads, but that
//   had its own problems.  The current code uses lazy allocation at the
//   thread level so no storage is allocated for threads that make no
//   allocation requests.
//
// ***************************************************************************


#if MAX_PTHREADS > 1
  #define POOL (ASIM_SMP_CLASS::GetRunningThreadNumber())
#else
  #define POOL 0
#endif


template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
class ASIM_POOL_ALLOCATED_OBJECT_CLASS
{
  public:
    ASIM_POOL_ALLOCATED_OBJECT_CLASS ();
    virtual ~ASIM_POOL_ALLOCATED_OBJECT_CLASS();
          
    POOL_OBJECT_TYPE * Allocate();
    void Release(POOL_OBJECT_TYPE * ptr);

  private:
    //
    // Each thread gets a private pool.  This works only if an objected is
    // allocated and freed by the same pool.  There is a check in the release
    // code to confirm this requirement.
    //
    class THREAD_POOL_CLASS {
      public:
        POOL_OBJECT_TYPE data[POOL_SIZE];

        //the last object that was allocated
        UINT32 last_allocated;

        //the flag to state whether an element is in use
        bitset<POOL_SIZE> inUse;

        THREAD_POOL_CLASS() : last_allocated(0) {};
        ~THREAD_POOL_CLASS() {};
    };

    typedef THREAD_POOL_CLASS *THREAD_POOL;
    THREAD_POOL thread_pools[MAX_PTHREADS];
};



template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
ASIM_POOL_ALLOCATED_OBJECT_CLASS<POOL_OBJECT_TYPE,POOL_SIZE>::ASIM_POOL_ALLOCATED_OBJECT_CLASS()
{
    for (UINT32 i = 0; i < MAX_PTHREADS; i++)
    {
        thread_pools[i] = NULL;
    }
}

template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
ASIM_POOL_ALLOCATED_OBJECT_CLASS<POOL_OBJECT_TYPE,POOL_SIZE>::~ASIM_POOL_ALLOCATED_OBJECT_CLASS()
{
    for (UINT32 i = 0; i < MAX_PTHREADS; i++)
    {
        if (thread_pools[i])
        {
            delete thread_pools[i];
        }
    }
}

template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
POOL_OBJECT_TYPE *
ASIM_POOL_ALLOCATED_OBJECT_CLASS<POOL_OBJECT_TYPE,POOL_SIZE>::Allocate ()
{
    if (thread_pools[POOL] == NULL)
    {
        thread_pools[POOL] = new THREAD_POOL_CLASS();
    }

    THREAD_POOL tPool = thread_pools[POOL];

    UINT32 index = tPool->last_allocated;
    index++;
    if(index==(POOL_SIZE) )
    {
        index=0;
    }

    while(tPool->inUse[index])
    {
        //make sure that we didn't wrap around and not find an element
        ASSERT(index != tPool->last_allocated, "Couldn't find a free element!\n");
        index++;
        if(index==(POOL_SIZE) )
        {
            index=0;
        }
    }
    tPool->last_allocated = index;
   
    tPool->inUse[index] = true;
    
    return &(tPool->data[index]);
}

template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
void
ASIM_POOL_ALLOCATED_OBJECT_CLASS<POOL_OBJECT_TYPE,POOL_SIZE>::Release (
    POOL_OBJECT_TYPE* ptr)  ///< pointer to object memory
{
    THREAD_POOL tPool = thread_pools[POOL];

    POOL_OBJECT_TYPE *base_ptr= &(tPool->data[0]); 
    UINT32 objIdx = ptr-base_ptr;

    ASSERT(tPool && (objIdx < POOL_SIZE), "Object does not belong to this pool.  Free by wrong thread?");
    ASSERT(tPool->inUse[objIdx], "Attempt to free object not in use");

    tPool->inUse[objIdx]=false;
}

#endif 
