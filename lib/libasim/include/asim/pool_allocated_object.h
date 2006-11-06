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
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/ioformat.h"
#include "asim/atomic.h"
#include "asim/smp.h"

namespace iof = IoFormat;
using namespace iof;



#if NUM_PTHREADS > 1

  #define POOL get_asim_thread_id()
  
#else

  #define POOL 0
  
#endif


template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
class ASIM_POOL_ALLOCATED_OBJECT_CLASS
{
  private:
     //the actual pool data
     POOL_OBJECT_TYPE data[POOL_SIZE/NUM_PTHREADS][NUM_PTHREADS];
    
     //the flag to state whether an element is in use
     bool inUse[POOL_SIZE/NUM_PTHREADS][NUM_PTHREADS];
     
     //the last object that was allocated
     UINT32 last_allocated[NUM_PTHREADS];

  public:

    // constructors/destructors
    ASIM_POOL_ALLOCATED_OBJECT_CLASS ();
    /// Delete this MM object
    virtual ~ASIM_POOL_ALLOCATED_OBJECT_CLASS();
          
    POOL_OBJECT_TYPE * Allocate();
    void Release(POOL_OBJECT_TYPE * ptr);

};



template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
ASIM_POOL_ALLOCATED_OBJECT_CLASS<POOL_OBJECT_TYPE,POOL_SIZE>::ASIM_POOL_ALLOCATED_OBJECT_CLASS ()
{
    // nada
   memset(&inUse,0,sizeof(inUse));
   memset(&last_allocated,0,sizeof(last_allocated));
}

template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
ASIM_POOL_ALLOCATED_OBJECT_CLASS<POOL_OBJECT_TYPE,POOL_SIZE>::~ASIM_POOL_ALLOCATED_OBJECT_CLASS ()
{
    // nada
}

template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
POOL_OBJECT_TYPE *
ASIM_POOL_ALLOCATED_OBJECT_CLASS<POOL_OBJECT_TYPE,POOL_SIZE>::Allocate ()
{

    UINT32 index=last_allocated[POOL];
    index++;
    if(index==(POOL_SIZE/NUM_PTHREADS) )
    {
        index=0;
    }

    while(inUse[index][POOL])
    {
        //make sure that we didn't wrap around and not find an element
        ASSERT(index!=last_allocated[POOL], "Couldn't find a free element!\n");
        index++;
        if(index==(POOL_SIZE/NUM_PTHREADS) )
        {
            index=0;
        }
    }
    last_allocated[POOL]=index;
   
    inUse[index][POOL]= true;
    
    return &(data[index][POOL]);
 
}

template <class POOL_OBJECT_TYPE,UINT32 POOL_SIZE>
void
ASIM_POOL_ALLOCATED_OBJECT_CLASS<POOL_OBJECT_TYPE,POOL_SIZE>::Release (
    POOL_OBJECT_TYPE* ptr)  ///< pointer to object memory
{

   POOL_OBJECT_TYPE *my_ptr=ptr;
   POOL_OBJECT_TYPE *base_ptr= &(data[0][POOL]); 

//   UINT32 index = ((UINT64)my_ptr - (UINT64)base_ptr)/ sizeof(POOL_OBJECT_TYPE);
   inUse[my_ptr-base_ptr][POOL]=false;
}

#endif 
