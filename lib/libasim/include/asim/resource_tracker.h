/*****************************************************************************
 *
 *
 * @author Steven Raasch
 *
 * @brief Class to track usage of a resource
 *
 *Copyright (C) 2005-2006 Intel Corporation
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

#ifndef _RESOURCE_TRACKER_
#define _RESOURCE_TRACKER_

#include <sstream>

// ASIM core
#include "asim/syntax.h"
#include "asim/trace.h"
#include "asim/mesg.h"

// Change this to enable/disable debug messages
#define MY_DEBUG 0


enum TRACKER_MODE {
    TRACKER_MODE_NO_ID = 0,
    TRACKER_MODE_SEQUENTIAL,
    TRACKER_MODE_SEQUENTIAL_WITH_WRAP,
    TRACKER_MODE_FREE_LIST,

    TRACKER_MODE_NUMBER_OF_MODES
};


static const char * modeNames[TRACKER_MODE_NUMBER_OF_MODES] = {
    "No ID tracking",
    "Sequential",
    "Sequential with wrap",
    "Free List"
};


typedef class RESOURCE_TRACKER_CLASS * RESOURCE;
class RESOURCE_TRACKER_CLASS
{
  private:
    UINT32 size;
    UINT32 alloc_map_size;
    UINT32 total_used;
    UINT32 * num_used;
    UINT32 num_threads;
    UINT32 * per_thread_limit;

    bool   partitioned_resource;

    UINT32 num_alloc_maps;
    UINT32 ** alloc_map;

    const char * name;
    
    TRACKER_MODE  tracking_mode;

    bool track_ids;
    bool sequential_allocation;

    UINT32 * next_id;

  public:
    RESOURCE_TRACKER_CLASS(const char * _name, 
			   UINT32 _size, 
			   UINT32 _num_threads,
			   TRACKER_MODE   _tracking_mode = TRACKER_MODE_NO_ID,
			   UINT32 * _per_thread_limit = 0,
	                   bool   _partitioned = false);
    ~RESOURCE_TRACKER_CLASS();

    void Clear();
    void Clear(UINT32 thread);

    void SetNextID(UINT32 id, UINT32 thread);

    bool IsEmpty(UINT32 thread);

    void Dump(const TRACEABLE traceable);

    bool IsAvailable(UINT32 thread, UINT32 count);
    UINT32 Reserve(UINT32 thread);

    void ReleaseID(UINT32 thread, UINT32 start_id, UINT32 count);
    void ReleaseCount(UINT32 thread, UINT32 count);

    UINT32 Size() { return size; }

    UINT32 NumAvailable() { return size - total_used; }
    UINT32 NumAvailable(UINT32 thread) { return per_thread_limit[thread] - num_used[thread]; }

    UINT32 NumUsed() { return total_used; }
    UINT32 NumUsed(UINT32 thread ) { return num_used[thread]; }

    TRACKER_MODE   WhichTrackingMode() { return tracking_mode; }
};




/*****************************************************************************
 *   Constructor & Destructor
 *
 *****************************************************************************/
RESOURCE_TRACKER_CLASS::RESOURCE_TRACKER_CLASS(
    const char *  _name,
    UINT32   _size,
    UINT32   _num_threads,
    TRACKER_MODE _tracking_mode,
    UINT32 * _per_thread_limit,
    bool     _partitioned) :
    partitioned_resource(_partitioned),
    name(_name)
{
#if MY_DEBUG
    cout << "\tRESOURCE TRACKER Constructor:" << endl;
    cout << "\t\tName: " << _name << endl;
    cout << "\t\tSize: " << _size << endl;
    cout << "\t\tThreads: " << _num_threads << endl;
    cout << "\t\tMode: " << modeNames[_tracking_mode] << endl;
    if (_per_thread_limit) {
	cout << "\t\tPer-thread limits are set" << endl;
    }
#endif

    tracking_mode = _tracking_mode;
    num_threads = _num_threads;

    size = _size;

    alloc_map_size = size * 2;
    num_alloc_maps = 1;

    if (partitioned_resource)
    {
	num_alloc_maps = num_threads;
	alloc_map_size = alloc_map_size / num_threads;
    }

    total_used = 0;

    if (size == 0) {
	//
	//  Infinite resources
	//

	alloc_map_size = 1;
    }
    else
    {
	//
	//  Finite resources, use alloc_map_size as given
	//
    }


    ASSERTX(alloc_map_size > 0);
    alloc_map = new UINT32 * [num_alloc_maps];

    //  Allocate memory
    num_used         = new UINT32[num_threads];
    next_id          = new UINT32[num_threads];
    per_thread_limit = new UINT32[num_threads];

    for (uint t=0; t<num_alloc_maps; ++t)
    {
	alloc_map[t]  = new UINT32[alloc_map_size];
	for (UINT32 i=0; i<alloc_map_size; ++i) {
	    alloc_map[t][i] = num_threads;  // flag value indicates unused
	}
    }

    for (UINT32 i=0; i<num_threads; ++i) {

	num_used[i] = 0;

	if (_per_thread_limit != 0) {
	    ASSERTX(_per_thread_limit[i] <= size);
	    per_thread_limit[i] = _per_thread_limit[i];
	}
	else {
	    per_thread_limit[i] = size / num_alloc_maps;
	}
    }

    Clear();

#if MY_DEBUG
    cout << "\t\t# Alloc Maps: " << num_alloc_maps << endl;
    cout << "\t\tAlloc Map Size: " << alloc_map_size << endl;
#endif

}

RESOURCE_TRACKER_CLASS::~RESOURCE_TRACKER_CLASS()
{
    delete[] num_used;
    delete[] next_id;
    delete[] per_thread_limit;

    for (uint t=0; t<num_alloc_maps; ++t)
    {
	delete[] alloc_map[t];
    }
    delete[] alloc_map;
}

/*****************************************************************************
 *   Clear the resource
 *
 *****************************************************************************/
void
RESOURCE_TRACKER_CLASS::Clear()
{
#if MY_DEBUG
    cout << "\tRESOURCE TRACKER (" << name << "): Clear()" << endl;
#endif

    for (UINT32 t=0; t<num_threads; ++t) {
	Clear(t);

	// Per-thread clear man not do this
	next_id[t] = 0;
    }

}

void
RESOURCE_TRACKER_CLASS::Clear(
    UINT32 thread)
{
#if MY_DEBUG
    cout << "\tRESOURCE TRACKER (" << name << "): Clear(" << thread << ")" << endl;
#endif

    total_used = total_used - num_used[thread];

    num_used[thread] = 0;

    UINT32 map = partitioned_resource ? thread : 0;
    if (partitioned_resource)
    {
	next_id[thread] = 0;
    }


    for (UINT32 i=0; i<alloc_map_size; ++i) {
	if (alloc_map[map][i] == thread) {
	    alloc_map[map][i] = num_threads;
	}
    }
}


/*****************************************************************************
 *   Return true if there are no occupied entries
 *
 *****************************************************************************/
bool
RESOURCE_TRACKER_CLASS::IsEmpty(
    UINT32 thread)
{
    return (num_used[thread] == 0);
}


/*****************************************************************************
 *   Set the next_id field (Check for validity first)
 *
 *****************************************************************************/
void
RESOURCE_TRACKER_CLASS::SetNextID(
    UINT32 id,
    UINT32 thread)
{
    next_id[thread] = id;
    if (id == alloc_map_size)
    {
        next_id[thread] = 0;
    }
}


/*****************************************************************************
 *   
 *
 *****************************************************************************/
bool
RESOURCE_TRACKER_CLASS::IsAvailable(
    UINT32 thread,
    UINT32 count)
{
#if MY_DEBUG
    cout << "\tRESOURCE TRACKER (" << name << ") IsAvaialble: " 
	 << "total available=" << size-total_used 
	 << " thread available=" << per_thread_limit[thread]-num_used[thread];
#endif

    //  Special case for infinite resources
    if (size == 0) {
#if MY_DEBUG
    cout << " TRUE" << endl;
#endif
	return true;
    }

    if ((size - total_used >= count) && 
	(num_used[thread] + count <= per_thread_limit[thread]))
    {
	if (tracking_mode == TRACKER_MODE_SEQUENTIAL) {

	    UINT32 map = partitioned_resource ? thread : 0;

	    //  For sequential allocation, we must check that the next 'count'
	    //  id's are available
	    UINT32 id = next_id[map];
	    for (UINT32 i=0; i<count; ++i) {
		if (alloc_map[map][id] != num_threads) {
#if MY_DEBUG
    cout << " FALSE" << endl;
#endif
		    return false;
		}

		// point to next id to test (handle wrapping)
		if (++id == alloc_map_size) {
		    id = 0;
		}
	    }

	    //  If all id's are free, it's available
	}

#if MY_DEBUG
    cout << " TRUE" << endl;
#endif
	return true;
    }
    
#if MY_DEBUG
    cout << " FALSE" << endl;
#endif
    return false;
}


//
//  Reserve a single unit.
//
//  Reserving one unit per call eliminates the complexity of returning a set of ID
//  numbers which is the un-common case
//
UINT32
RESOURCE_TRACKER_CLASS::Reserve(UINT32 thread)
{
    uint map = partitioned_resource ? thread : 0;
    UINT32 id = next_id[map];

#if MY_DEBUG
    cout << "\tRESOURCE TRACKER (" << name << "): Reserve(" << thread << ")" << endl;
#endif

    //  If we're not configured for infinite resources
    if (size != 0) {

	//  Let's just double-check...
	ASSERTX(IsAvailable(thread,1));
	
	switch (tracking_mode) {
	  case TRACKER_MODE_NO_ID:
	    id = 0;
	    break;
	    
	  case TRACKER_MODE_SEQUENTIAL:
	  case TRACKER_MODE_SEQUENTIAL_WITH_WRAP:
	    //
	    //  Simply allocate the next id
	    //
	    id = next_id[map]++;

	    if (next_id[map] == alloc_map_size) {
		next_id[map] = 0;
	    }
	    ASSERT(alloc_map[map][id] == num_threads, "RESOURCE TRACKER (" << name <<
		                                      "): Tried to allocate id #" << id);
	    alloc_map[map][id] = thread;
	    break;

	  case TRACKER_MODE_FREE_LIST:
	    //
	    //  Non-sequential allocation means that the next
	    //  id may not be available. Search for an id starting
	    //  at 'next_id'
	    //
	    for (UINT32 i=0; i<alloc_map_size; ++i) {
		if (alloc_map[map][id] == num_threads) {
		    break;   // use this one
		}
		
		// point to next id to test (handle wrapping)
		if (++id == alloc_map_size) {
		    id = 0;
		}
	    }
	    ASSERTX(alloc_map[map][id] == num_threads);
	    alloc_map[map][id] = thread;
	    break;
	    
	  default:
	    ASSERT(false, "You shouldn't be here!");
	    break;
	}

	++num_used[thread];
	++total_used;
    }
    else {
	id = 0;
    }

#if MY_DEBUG
    cout << "\tRESOURCE TRACKER (" << name << "): "
	 << "reserved unit #" << id 
	 << " for thread " << thread;
    if (size != 0) {
	cout << " [avail: " << size-total_used << "(" 
	     << per_thread_limit[thread]-num_used[thread] << ")] "
	     << "(Next ID = " << next_id[map] << ")" << endl;
    }
    else {
	cout << " [infinite resource]" << endl;
    }
#endif

    return id;
}


/*--------------------------------------------------------------------
 *
 *  Release the specified resource(s)
 *
 *--------------------------------------------------------------------*/
void
RESOURCE_TRACKER_CLASS::ReleaseID(UINT32 thread, UINT32 start_id, UINT32 _count)
{

#if MY_DEBUG
    cout << "\tRESOURCE TRACKER (" << name << "): Release(" << 
	thread << "," << start_id << "," <<  _count << ")" << endl;
#endif

    if (size != 0) {

	uint map = partitioned_resource ? thread : 0;

	//  Pick up the thread ID of the first ID
	thread = alloc_map[map][start_id];


	//  Can't do this, since we may deallocate something twice
	//  Sanity checks
	//      ASSERTX(thread != num_threads);
        //	ASSERTX(num_used[thread] >= _count);
	
	ASSERTX(tracking_mode != TRACKER_MODE_NO_ID);
	    
	// Free 'count' units starting at 'start_id'
	    
	UINT32 id = start_id;
	UINT32 count = 0;
	// we only want to check 'size' elements here, since that's the
	// 'actual' size of the id-space
	for (UINT32 i=0; i<size; ++i) {  
	    
	    if (alloc_map[map][id] == thread) {
		// found one.. free it
		alloc_map[map][id] = num_threads;
		
		// did we get them all?
		if (++count == _count) {
		    break;
		}
	    }
	    
	    if (++id == alloc_map_size) {
		id = 0;
	    }
	}

	num_used[thread] -= _count;
	total_used -= _count;
    }

#if MY_DEBUG
    cout << "\tRESOURCE TRACKER (" << name << "): "
	 << "released unit " << start_id << " (len " << _count << ") "
	 << "for thread " << thread;
    if (size != 0) {
	cout << " [avail: " << size-total_used << "(" 
	     << per_thread_limit[thread]-num_used[thread] << ")]" << endl;
    }
    else {
	cout << " [infinite resource]" << endl;
    }
#endif

}


/*--------------------------------------------------------------------
 *
 *  Release the specified resource(s)
 *
 *--------------------------------------------------------------------*/
void
RESOURCE_TRACKER_CLASS::ReleaseCount(UINT32 thread, UINT32 _count)
{
    if (size != 0) {

	//  Sanity checks
        ASSERTX(num_used[thread] >= _count);
	
	ASSERTX(tracking_mode == TRACKER_MODE_NO_ID);
	    
	num_used[thread] -= _count;
	total_used -= _count;
    }

#if MY_DEBUG
    cout << "\tRESOURCE TRACKER (" << name << "): "
	 << "released unit  (len " << _count << ") "
	 << "for thread " << thread;
    if (size != 0) {
	cout << " [avail: " << size-total_used << "(" 
	     << per_thread_limit[thread]-num_used[thread] << ")]" << endl;
    }
    else {
	cout << " [infinite resource]" << endl;
    }
#endif

}


/*--------------------------------------------------------------------*/

void
RESOURCE_TRACKER_CLASS::Dump(
    const TRACEABLE traceable)
{
    T2_AS(traceable, "\tResource Tracker (" << name << "):");
    T2_AS(traceable, "\t\tSize: " << size);

    if (size != 0) {
	T2_AS(traceable, "\t\tTotal Used: " << total_used);
	T2_AS(traceable, "\t\tTracking Mode: " << modeNames[tracking_mode]);

	for (UINT32 i=0; i<num_threads; ++i) {
	    T2_AS(traceable, "\t\tNum Used (thread " << i << "): " << num_used[i]);
	}
	for (UINT32 i=0; i<num_threads; ++i) {
	    T2_AS(traceable, "\t\tThread Limit (thread " << i << "): " << per_thread_limit[i]);
	}

	if (tracking_mode != TRACKER_MODE_NO_ID) {
	    for (uint map=0; map<num_alloc_maps; ++map)
	    {
		T2_AS(traceable, "\t\tNext ID: " << next_id[map]);
		T2_AS(traceable, "\t\tAllocation Map " << map << ":");
		UINT32 index = 0;
		do {
		    ostringstream s;
		    s << "\t\t\t";
		    for (UINT32 c=0; (index<alloc_map_size) && (c<20); ++index, ++c) {
			if ((c>0) && (c % 5 == 0)) {
			    s << " ";
			}
			if (alloc_map[map][index]==num_threads) {
			    s << "_";
			}
			else {
			    s << alloc_map[map][index];
			}
		    }
		    T2_AS(traceable, s.str());
		} while (index < alloc_map_size);
	    }
	}
    }
    else {
	// Infinite resource
    }
}

#endif  // _RESOURCE_TRACKER_
