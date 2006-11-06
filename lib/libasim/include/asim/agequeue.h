/*
 *Copyright (C) 2003-2006 Intel Corporation
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
 * @author Roger Espasa
 */

#ifndef _AGEQUEUE_H
#define _AGEQUEUE_H

// generic
#include <stdio.h>

// ASIM core
#include "asim/syntax.h"
#include "asim/utils.h"

// To reduce SelfCheck when runnig in optimized mode we define the following:
#ifdef ASIM_ENABLE_TRACE
#define SELF_CHECK ASSERTX(SelfCheck())
#else
#define SELF_CHECK
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////
//// QUEUE OF OBJECTS SORTED BY AGE (where AGE is defined by entry time in the 
//// queue).
////
////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//
// This class is an improvement on the ACKQUEUE class implemented in ackqueue.h.
// More precisely, this class is a generalisation of the former. 
// 
// The goal of this class is to provide a data type for describing queue-like
// data structures where instructions/data-misses/system-requests stay for a
// while until they are serviced/resolved. 
// 
// The typical pattern of usage of such a class is as follows:
//
// An object (instruction/d-cache-miss/scache-miss/etc...) is Entered into
// the queue. This is done using routine Enter(). Some time later, the
// logic controlling the queue decides to send the oldest object in
// the queue to some other box for further processing. This step is
// called the 'LAUNCH' of the object. After being launched, an object stays
// in the queue (thus consuming a valuable resource) until:
//
//  - its servicing is fully accomplished or
//  - the service process failed at some point and the object must be re-launched
//    again.
//
// If the object is considered to be fully serviced, it can be removed from the queue
// using routine Remove(). If the object is not fully serviced, it must be considered 
// for being re-launched again. This can be accomplished using routine Rearm().
//
// An object inside the queue can also be put in "SUSPENDED" mode; this can be useful
// when the queue is also used as a scoreboard mechanism (in conjunction with scoreboard.h).
// For example, think of the store queue in the VBOX, or the DIFT in the ZBOX. In both
// cases if a 'dependency' is found between two objects in the queue, then the younger object
// is not allowed to proceed. We support this notion using the Suspend()/UnSuspend() interface.
// An object can be put into SUSPENDED mode. In this mode, it will not be considered for
// LAUNCH.
//
// The class also provides an interface to "WALK" through it; the interface allows
// walks from youngest to oldest and from oldest to youngest.
//
//

typedef enum { 
		 ENTRY_FREE,
  		 ENTRY_READY_TO_LAUNCH,
		 ENTRY_LAUNCHED,
		 ENTRY_SUSPENDED
	       } EntryState;

template<class T, UINT32 N> class agequeue
{
  private: 

  //
  // Each entry in the AgeQueue has a 2-bit associated state. An entry follows a very simple
  // automaton through its life:
  //
  //      
  //                                   +-------------+
  //                                   |  ENTRY_FREE |
  //                                   +-------------+
  //                                     /\       \.
  //                                     /         \.
  //            Entry deallocated and   /           \  Entry allocated in Enter() routine
  //            forgotten              /             \.
  //                                  /               \.
  //                                 /                \/
  //                                /             +------------------------+ Suspend()  +------------------+
  //                               /              |  ENTRY_READY_TO_LAUNCH |----------->| ENTRY_SUSPENDED  |
  //                               \              +------------------------+            +------------------+
  //                                \                  /\        /       /\                   /
  //                                 \                 /        /         \__________________/
  //                         Remove() \      Rearm()  /        /              UnSuspend()
  //                                   \             /        / Launch()
  //                                    \           /        \/
  //                                   +-----------------------+
  //                                   |  ENTRY_LAUNCHED       |
  //                                   +-----------------------+
  //
  //
  //
  // Each entry in the queue contains an 'object', state information, and ``pointers'' linking this entry
  // to the previous and next entries in the queue
  //
  struct {
   EntryState	state;
   int		next;
   int		prev;
   T		obj;
  } queue[N];

  //
  // Number of items in queue
  //
  INT32	count;

  //
  // Index into the array indicating first (oldest) element in queue (must be 'int' to support
  // having a -1 as indicator of 'empty list')
  //
  INT32	first;

  //
  // Index into array indicating last (youngest) element in queue (must be 'int' to support
  // having a -1 as indicator of 'empty list')
  //
  INT32	last;

  //
  // Index of the first item that is on the free list (must be 'int' to support
  // having a -1 as indicator of 'empty list')
  //
  INT32	free;

  //
  // This index indicates the first element in the queue (from oldest to youngest) that is 
  // marked as  ENTRY_READY_TO_LAUNCH. (must be 'int' to support
  // having a -1 as indicator of 'empty list')
  //
  INT32	firstReadyToLaunch;

  private:

	// Self-diagnosis of the state of the queue
	bool		SelfCheck() const;


  public:

	//
	// Find the index of a given object by presenting the pointer to the object itself.
	// This interface works assuming that 'obj' can be uniquely identified. I.e, you better instantiate
	// this class only with small data types (pointers, essentially). It is unlikely that you need this 
	// interface at all, and that is the reason why we put it somewhat separated from the other public
	// functions.
	//
	INT32		Find(T obj);

  public:

	//
	// Constructor
	//
	agequeue();

	//
	// Add an item to the queue. The new item is considered to be 'younger' than any
	// item in the queue
	//
	UINT32		Enter(T obj);

	//
	// Consider the object presented (either by index or by pointer) 'launched'. It must be true that 
	// previous to invoking Launch() the object was in state ENTRY_READY_TO_LAUNCH. After inovking this
	// routine, the object will be in state ENTRY_LAUNCHED and will remain in that state until either
	// Remove() or Rearm() is invoked on the object.
	//
	void		Launch(UINT32 idx);
	void		Launch(T obj)		{ Launch(Find(obj)); }

	//
	// Eliminates an object from the queue. The second version of the function provides a 'SEARCHING' interface. That is,
	// you need not know the exact index. By presenting the pointer to the object we search the queue and remove the object.
	//
	void		Remove(UINT32 idx);
	void		Remove(T obj)		{ Remove(Find(obj)); }

	//
	// The object being identified has suffered some problem (a NACK or an error condition) and has to be launched again from
	// the queue. The guarantee here is that the next time you execute 'OldestReady' the object being rearmed will be considered
	// as a candidate (of course, only if it's the oldest candidate it will be actually selected). Again, we also provide
	// a searching interface where the class scans the queue looking for the object you present.
	//
	void		Rearm(UINT32 idx);
	void		Rearm(T obj)		{ Rearm(Find(obj)); }

	//
	// The object being indicated must be SUSPENDED and not considered for launching until further notice (i.e., until UnSuspended()
	// by the user himself). The Object must be in state ENTRY_READY_TO_LAUNCH in order to be eligible to be suspended. It is meaningless
	// (and incorrect) to try to suspend a LAUNCHED object
	//
	void		Suspend(UINT32 idx)	{ ASSERTX(idx < N && queue[idx].state == ENTRY_READY_TO_LAUNCH); queue[idx].state = ENTRY_SUSPENDED; }
	void		Suspend(T obj)		{ Suspend(Find(obj)); }
	void		UnSuspend(UINT32 idx);
	void		UnSuspend(T obj)	{ UnSuspend(Find(obj)); }

	//
	// Functions that return state of queue
	//
  	UINT32		GetOccupancy() const	{ SELF_CHECK; return count; }
  	UINT32		GetCapacity() const	{ SELF_CHECK; return N; }
  	UINT32		GetFreeSpace() const	{ SELF_CHECK; return (N - count); }

	//
	// Functions to perform a WALK on the list (akin to the 'ITERATORS' in Nate's classes
	// but not as fancy). They are based on integers instead of an iterator class due to
	// my limited abilities with C++ :-). Well, that is not entirely true. There are good
	// reasons to make the iterators UINT32: in particular, you can pass the index to an object
	// to the next BOX without much fuss (as opposed to using real STL iterators).
	//

	//
	// Returns the oldest object in the queue. Can return objects in state ENTRY_READY_TO_LAUNCH
	// or in state  ENTRY_LAUNCHED. Returns -1 if list empty. We need different names for both
	// routines because C++ sucks and is not able to distinguish their signature based on the 
	// return type (why use such a complicated language if it fails miserably in the most simple
	// requirement ? ;-) -- roger)
	//
	INT32		Oldest() const		{ SELF_CHECK; return first; }
	T		OldestObj() const	{ SELF_CHECK; return (first != -1) ? queue[first].obj : static_cast<T>(NULL); }

	//
	// Returns the youngest object in the queue. Can return objects in state ENTRY_READY_TO_LAUNCH
	// or in state ENTRY_LAUNCHED. Returns -1 if list empty.
	//
	INT32		Youngest() const	{ SELF_CHECK; return last; }
	T		YoungestObj() const	{ SELF_CHECK; return (last != -1) ? queue[last].obj : static_cast<T>(NULL); }

	//
	// Returns the oldest object in state ENTRY_READY_TO_LAUNCH in the queue. 
	//
	INT32		OldestReady();
	T		OldestReadyObj()	{ SELF_CHECK; INT32 i = OldestReady(); return ((i != -1) ? queue[i].obj : static_cast<T>(NULL)); }

	//
	// The two following functions return the next object in the queue (either younger or older) ignoring
	// its state (i.e., they can return objects both in state ENTRY_READY_TO_LAUNCH and in state ENTRY_LAUNCHED).
	// If the end of the queue is reached, they both return -1.
	// NEXT      always means going from older to younger
	// PREVIOUS  always means going from younger to older
	//
	INT32		Next(UINT32 idx) const;
	INT32		Previous(UINT32 idx) const;

	//
	// The two following functions return the next object in the queue (either younger or older) but skip
	// over all elements that are NOT in state ENTRY_READY_TO_LAUNCH. Thus, the object returned is always
	// in state ENTRY_READY_TO_LAUNCH. If the end of the queue is reached, they both return -1.
	// NEXT      always means going from older to younger
	// PREVIOUS  always means going from younger to older
	//
	INT32		NextReady(UINT32 idx) const;
	INT32		PreviousReady(UINT32 idx) const;

	//
	// Retrieve an object from the queue given its index. The index is obtained either from the
	// Enter() function or from any of the ``WALKING'' functions above.
	//
	T		GetObject(UINT32 idx) const { ASSERTX(idx < N); ASSERTX(queue[idx].state != ENTRY_FREE); return queue[idx].obj; }

	// 
	// Retrieve the status of an object.
	//
	EntryState	GetObjectState(UINT32 idx) const { ASSERTX(idx < N); ASSERTX(queue[idx].state != ENTRY_FREE); return queue[idx].state; }

};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////
/////
/////
/////  IMPLEMENTATION SECTION
/////
/////  (note: GCC seems to have trouble when having the implementation code in a separate .cpp
/////         file. Therefore, I have moved the agequeue.cpp file in here. Yet another reason not
/////         to use GCC ).
/////
/////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
// constRUCTOR
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<class T, UINT32 N> 
agequeue<T,N>::agequeue()
{
 UINT32 i;

 // Clear all entries and link them onto free list
 for (i = 0; i < N; i++ ) {
  queue[i].state = ENTRY_FREE;
  queue[i].obj = static_cast<T>(NULL);
  queue[i].prev = -1;
  queue[i].next = (i == (N-1)) ? -1 : INT32(i + 1);
 }
 // Free list starts at element 0
 free = 0;

 // We have no items, so first and last are invalid
 first = -1;
 last = -1;
 firstReadyToLaunch = -1;

 // Again, no items
 count = 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
// PRIVATE FUNCTIONS
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//
// Searches for an object starting at the first element of the list (oldest element)
// This could be sped up by using a hash function, but I don't have the time for
// that now. Feel free to improve it if your queues are long enough to warrant another
// data structure.
//
template<class T, UINT32 N> 
int
agequeue<T,N>::Find(T obj)
{
 int i;

 SELF_CHECK;
 for ( i = first; i != -1; i = queue[i].next ) {
  if ( queue[i].obj == obj ) return i;
 }

 return -1; 
}

//
// This function does a thorough checking on the consistency of the data strucutre.
// It is invoked by the other functions to help detect bugs as soon as possible
//
template<class T, UINT32 N> 
bool
agequeue<T,N>::SelfCheck() const
{
 int i, in_free, last_visited, in_list;
 bool ok  = true;

 ok &= (count >= 0);
 ok &= (count <= INT32(N));
 ASSERTX(ok);

 //
 // Check state of first and last pointers w.r.t the number of items
 // If we have items (count > 0), then both first and last must be defined
 // If we do not have items (count == 0), then the three pointers must be -1
 //
 ok &= (count > 0 ? (first != -1 && last != -1) : (first == -1 && last == -1 && firstReadyToLaunch == -1));
 ASSERTX(ok);

 //
 // Check that linked list of objects is in consistent state
 //
 for ( i = first, last_visited = -1, in_list = 0; i != -1; i = queue[i].next ) { 
  in_list++;
  last_visited = i;
  ok &= (queue[i].state != ENTRY_FREE);
  if ( queue[i].prev != -1 ) ok &= (queue[queue[i].prev].next == i);
  if ( queue[i].next != -1 ) ok &= (queue[queue[i].next].prev == i);
  ASSERTX(ok);
 }
 ok &= (last == last_visited);
 ok &= (in_list == count);
 ASSERTX(ok);

 // 
 // Check Status of free list. Walk it, counting how many entries are in it and compare to the
 // number of declared objects in the queue. The number of allocated entries PLUS the number of
 // free entries must match the total queue size.
 //
 for ( i = free, in_free = 0; i != -1; i = queue[i].next ) {
  in_free++;
  ok &= (queue[i].state == ENTRY_FREE);
  ASSERTX(ok);
 }
 ok &= ((in_free + count) == N);
 ASSERTX(ok);

 return ok;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
// PUBLIC FUNCTIONS
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
// The main four routines in this class are:
//
//  1. Enter
//  2. Launch
//  3a. Remove
//  3b. Rearm
//
// All of them offer two interfaces, searching and non-searching, but their underlying
// functionality is the same. The searching versions are built on top of the non-searching
// ones.
//
///////////////////////////////////////////////////////////////////////////////////////
//
// Enter a new object at the end of the queue. The new object entered is considered to 
// be younger than all previous objects in the queue.
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
UINT32
agequeue<T,N>::Enter(T obj)
{
 UINT32 idx;

 ASSERTX(count < INT32(N));
 ASSERTX(free != -1);
 ASSERTX(queue[free].state == ENTRY_FREE);

 // Get slot from free list and advance free list pointer
 idx = free;
 free = queue[idx].next;

 // Initialize slot
 queue[idx].state = ENTRY_READY_TO_LAUNCH;
 queue[idx].obj = obj;
 queue[idx].prev = last;
 queue[idx].next = -1;

 // Update 'first' and 'firstReadyToLaunch' pointers if necessary 
 if ( first == -1 ) first = idx;
 if ( firstReadyToLaunch == -1 ) firstReadyToLaunch = idx;

 // Update 'last' pointer and link the new element to end of list
 if ( last != -1 ) queue[last].next = idx;
 last = idx;

 // Increase total number of elements
 count++;

 SELF_CHECK;

 return idx;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// The User has requested to mark a given entry as LAUNCHED. Most likely, this entry
// will be the oldest entry, but we can not readily assume so (since we provide the user
// with walking functions over the queue, he might as well launch some younger object
// if he/she so wishes). However, we do try to advance the 'firstReadyToLaunch' pointer in
// order to speed up following invocations of OldestReady().
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
void
agequeue<T,N>::Launch(UINT32 idx)
{
 int i;

 ASSERTX(idx < N);
 ASSERTX(queue[idx].state == ENTRY_READY_TO_LAUNCH);

 // Mark entry to indicate that it has been sent out and we are waiting for a response
 queue[idx].state = ENTRY_LAUNCHED;

 //
 // Try to advance the 'firstReadyToLaunch' pointer so that the next invocation of
 // OldestReady() is a faster. Note that if the user has chosen to launch an object
 // that is not the oldest, then the 'firstReadyToLaunch' pointer might not advance (i.e.,
 // it might be pointing to some older object not yet launched
 //
 while ( firstReadyToLaunch != -1 && queue[firstReadyToLaunch].state != ENTRY_READY_TO_LAUNCH ) {
  firstReadyToLaunch = queue[firstReadyToLaunch].next;
 }

 SELF_CHECK;
}


///////////////////////////////////////////////////////////////////////////////////////
//
// The object indicated by the user must be removed from the queue. Most likely, this object
// has been correctly processed by some later box or has received the appropriate ACK from
// some downstream box.
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
void
agequeue<T,N>::Remove(UINT32 idx)
{

 ASSERTX(idx < N);
 ASSERTX(count > 0);
 ASSERTX(first != -1);

 // Check state of object in queue
 ASSERTX(queue[idx].state == ENTRY_LAUNCHED);

 // Clean object
 queue[idx].state = ENTRY_FREE;
 queue[idx].obj = static_cast<T>(NULL);

 // Remove object from queue list and add it to free list
 if ( queue[idx].prev != -1 ) queue[queue[idx].prev].next = queue[idx].next;
 if ( queue[idx].next != -1 ) queue[queue[idx].next].prev = queue[idx].prev;

 // Update first and last pointers
 if ( last == INT32(idx) ) last = queue[idx].prev;
 if ( first == INT32(idx) ) first = queue[idx].next;

 // Check for consitency with firstReadyToLaunch . It could happen that
 // the object being removed is the 'firstReadyToLaunch' since in routine
 // NACK we rewind this pointer to the begginning of the list
 //
 if ( firstReadyToLaunch == INT32(idx) ) firstReadyToLaunch = queue[idx].next;


 // Add to free list
 queue[idx].next = free;
 free = idx;

 // Update count
 count--;

 SELF_CHECK;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// We have received a NACK or something when wrong for an object that we launched several
// cycles ago. Find the object, check that it is in appropriate state and mark it
// so that it is launched again as soon as possible. 
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
void
agequeue<T,N>::Rearm(UINT32 idx)
{

 ASSERTX(idx < N);
 ASSERTX(count > 0);
 ASSERTX(first != -1);

 // Check state of object in queue
 ASSERTX(queue[idx].state == ENTRY_LAUNCHED);
 
 // Mark it as ready to launch again
 queue[idx].state = ENTRY_READY_TO_LAUNCH;

 //
 // Rewind the 'firstReadyToLaunch' entry so that, at next search, we start
 // again from beginning and we hit the entry that we just changed. Note that you can not
 // be smart and make 'firstReadyToLaunch' point to 'idx', since 'idx' might not be the
 // oldest object in the queue. Hence, you don't really know what to do and must be 
 // conservative and compeletely rewind firstReadyToLaunch.
 //
 firstReadyToLaunch = first; 

 SELF_CHECK;
}

template<class T, UINT32 N> 
void
agequeue<T,N>::UnSuspend(UINT32 idx)
{
 ASSERTX(idx < N);
 ASSERTX(queue[idx].state == ENTRY_SUSPENDED); 
 
 // Re-arm entry
 queue[idx].state = ENTRY_READY_TO_LAUNCH; 

 // Rewind pointer to make sure this entry will be 'seen' next time the user invoked
 // OldestReady()
 //
 firstReadyToLaunch = first;

 SELF_CHECK;
}


///////////////////////////////////////////////////////////////////////////////////////
//
//
// WALKING FUNCTIONS. 
//
//
///////////////////////////////////////////////////////////////////////////////////////
//
// The following set of functions allow the user to walk around the agequeue from older
// to youngest and viceversa.
//
///////////////////////////////////////////////////////////////////////////////////////
//
// Find the oldest object in queue that is marked as ENTRY_READY_TO_LAUNCH. The search can
// be sped up by using the hint that 'firstReadyToLaunch' provides
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
INT32
agequeue<T,N>::OldestReady()
{
 int i;

 SELF_CHECK;

 if ( count == 0 ) return -1;

 //
 // Start walking the list at the 'ReadyToLaunch' index and look for an
 // entry that has not yet been sent out
 //
 for ( i = firstReadyToLaunch; i != -1; i = queue[i].next ) {
  if ( queue[i].state == ENTRY_READY_TO_LAUNCH ) break;
 }

 if ( i != -1 ) {
  // We found an entry. Update the pointer so that next search starts at the entry we found.
  firstReadyToLaunch = i;

  // Return index of object found.
  return i;
 } 
 else {
  // Either the list is empty or all objects on the list have already been sent out
  // and are waiting FOR an ACK
  firstReadyToLaunch = -1;
  return -1;
 }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Return the next item that is older than 'idx'. This means going back on the 'prev' link.
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
inline INT32
agequeue<T,N>::Previous(UINT32 idx) const
{
 ASSERTX(idx < N);
 ASSERTX(queue[idx].state != ENTRY_FREE);
 return queue[idx].prev;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Return the next item that is younger than 'idx'. This means going forward on the 'next' link.
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
inline INT32
agequeue<T,N>::Next(UINT32 idx) const
{
 ASSERTX(idx < N);
 ASSERTX(queue[idx].state != ENTRY_FREE);
 return queue[idx].next;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Return the next READY item that is older than 'idx'. This means going back on the 'prev' link.
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
inline INT32
agequeue<T,N>::PreviousReady(UINT32 idx) const
{
 int i;
 ASSERTX(idx < N);
 ASSERTX(queue[idx].state != ENTRY_FREE);

 for ( i = queue[idx].prev; i != -1; i = queue[i].prev ) {
  if ( queue[i].state == ENTRY_READY_TO_LAUNCH ) break;
 }
 return i;
}
///////////////////////////////////////////////////////////////////////////////////////
//
// Return the next READY item that is younger than 'idx'. This means going forward on the 'next' link.
//
///////////////////////////////////////////////////////////////////////////////////////
template<class T, UINT32 N> 
inline INT32
agequeue<T,N>::NextReady(UINT32 idx) const
{
 int i;
 ASSERTX(idx < N);
 ASSERTX(queue[idx].state != ENTRY_FREE);

 for ( i = queue[idx].next; i != -1; i = queue[i].next ) {
  if ( queue[i].state == ENTRY_READY_TO_LAUNCH ) break;
 }
 return i;
}

#endif // _AGEQUEUE_H
