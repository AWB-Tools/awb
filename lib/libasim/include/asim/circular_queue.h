/*****************************************************************************
 *
 *
 * @author Oscar Rosell based on Julio Gago's fifo.h
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


/* The differences with Julio's fifo are:
    - Size dynamically determined
    - Closer to stl structures syntax
*/

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

// ASIM core
#include "asim/syntax.h"
#include "asim/utils.h"

template<class T> 
class circular_queue
{
  public:

    //
    // Constructor.
    //
    circular_queue(UINT32 size);
        
    //
    // Add an object to the tail of the FIFO (there MUST be space available).
    //
    void        push(const T& obj)	{ ASSERTX( count < data.size() ); data[tail] = obj; tail = UP_1_WRAP(tail, data.size()); ++count; }
        
    //
    // Get (but do not remove) an object from the head of the FIFO (there MUST be something in the FIFO).
    //
    T&          front()         { ASSERTX ( count > 0 ); return data[head]; }
    const T&    front() const   { ASSERTX ( count > 0 ); return data[head]; }

    T&          back()          { ASSERTX ( count > 0 ); return data[DOWN_1_WRAP(head + count, data.size())]; }
    const T&    back() const    { ASSERTX ( count > 0 ); return data[DOWN_1_WRAP(head + count, data.size())]; }
        
    //
    // Remove an object from the head of the FIFO (there MUST be something in the FIFO).
    //
    void        pop()           { ASSERTX( count > 0 ); head = UP_1_WRAP(head,data.size()); --count;}

    //
    // Get information about the state of the FIFO: how many objects? is it empty? is it full?
    //
    UINT32      size() const    { return count; }
    bool        empty() const   { return (count == 0);	}
    bool        full() const    { return (count == data.size());	}

  private: 
    std::vector<T> data;

    UINT32 head;
    UINT32 tail;
    UINT32 count;
};

template<class T> 
circular_queue<T>::circular_queue(UINT32 size):
    data(size),
    head(0),
    tail(0),
    count(0)
{
	//
	// empty the queue
	//
}

#endif // CIRCULAR_QUEUE_H
