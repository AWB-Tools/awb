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
 * @author Julio Gago 
 * @brief
 */

#ifndef _FIFO_H
#define _FIFO_H

// generic
#include <stdio.h>

// ASIM core
#include "asim/syntax.h"
#include "asim/utils.h"

/**
 * This class provides elemental and fast FIFO support for fixed-size queues
 * by using a static array and a few indexes. Although it may seem too much
 * simple to deserve a base class, it is used very often (the CBOX has tons
 * of simple FIFOs) both for object fifo's and free lists. Furthermore, the
 * universally known GET/PUT FIFO mechanisms are a potential source of bugs,
 * so it's always safer to encapsulate them.
 */
template<class T, UINT32 N> class fifo
{
  private: 

	UINT32 head;
	UINT32 tail;
	UINT32 count;
	  
	T data[N];

  public:

	//
	// Constructor.
	//
	fifo();
	
	//
	// Add an object to the tail of the FIFO (there MUST be space available).
	//
	void			Put(T obj)		{ ASSERTX( count < N ); data[tail] = obj;	tail = (tail + 1) % N; count++; }
	
	//
	// Get (and remove) an object from the head of the FIFO (there MUST be something in the FIFO).
	//
	T			Get()			{ ASSERTX( count > 0 ); T ret = data[head];	head = (head + 1) % N; count--; return ret; }
	
	//
	// Get (but do not remove) an object from the head of the FIFO (there MUST be something in the FIFO).
	//
	T			Peek()			{ ASSERTX ( count > 0 ); return data[head]; }
	
	//
	// Remove an object from the head of the FIFO (there MUST be something in the FIFO).
	//
	void			Drop()			{ ASSERTX( count > 0 ); 			head = (head + 1) % N; count--; return; }

	//
	// Get information about the state of the FIFO: how many objects? is it empty? is it full?
	//
	UINT32			Count()			{ return count; 	}
	bool			Empty()			{ return (count == 0);	}
	bool			Full()			{ return (count == N);	}
};

template<class T, unsigned N> 
fifo<T,N>::fifo()
{
	//
	// empty the queue
	//
	count = 0;
	head = 0;
	tail = 0;
}

#endif // _FIFO_H
