/*
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


#include <deque>

template <class T, unsigned N>
class CIRCULAR_QUEUE_CLASS {
    deque<T> queue;

    UINT32 max_size;

  public:
    CIRCULAR_QUEUE_CLASS();
    virtual ~CIRCULAR_QUEUE_CLASS() {}
    
    void Reset() { queue.clear(); }

    bool Add(T &d);
    bool Enqueue(T &d) { return Add(d); }

    bool Remove(T &d);
    bool Dequeue(T &d) { return Remove(d); }

    bool Peek(T &d);


    UINT32 Size()     { return queue.size(); }
    UINT32 Capacity() { return max_size; }

    bool IsFull()  { return (queue.size() == max_size); }
    bool IsEmpty() { return (queue.size() == 0); }

};

//
//  Constructor
//
template <class T, unsigned N>
CIRCULAR_QUEUE_CLASS<T,N>::
CIRCULAR_QUEUE_CLASS() 
{
    max_size = N;
    queue.resize(max_size);
}


template <class T, unsigned N>
bool
CIRCULAR_QUEUE_CLASS<T,N>::
Add(T &d)
{
    if (queue.size() < max_size) {
	queue.push_back(d);
    }
    else {
	return false;
    }

    return true;
}


template <class T, unsigned N>
bool
CIRCULAR_QUEUE_CLASS<T,N>::
Remove(T &d)
{
    if (!queue.empty()) {
	d = *(queue.begin());
	queue.pop_front();
    }
    else {
	return false;
    }

    return true;
}


template <class T, unsigned N>
bool
CIRCULAR_QUEUE_CLASS<T,N>::
Peek(T &d)
{
    if (!queue.empty()) {
	d = *(queue.begin());
    }
    else {
	return false;
    }

    return true;
}

