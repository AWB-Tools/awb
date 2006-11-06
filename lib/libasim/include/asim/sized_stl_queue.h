/*****************************************************************************
 *
 * @brief Header file for SIZED_STL_QUEUE
 *
 * @author Pritpal Ahuja
 *
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

#ifndef _SIZED_STL_QUEUE_
#define _SIZED_STL_QUEUE_

#include <queue>

template<class T>
class SIZED_STL_QUEUE_CLASS : public queue<T>
{
    const UINT32 myMaxSize;

  public:
    SIZED_STL_QUEUE_CLASS(const UINT32 max_size);

    bool push(T t);
    bool full();
};


template<class T>
SIZED_STL_QUEUE_CLASS<T>::SIZED_STL_QUEUE_CLASS(const UINT32 max_size)
    : queue<T>(), myMaxSize(max_size)
{
}


template<class T> 
inline bool
SIZED_STL_QUEUE_CLASS<T>::push(T t)
{
    if (this->size() < myMaxSize)
    {
        queue<T>::push(t);
        return true;
    }

    return false;
}


template<class T>
inline bool
SIZED_STL_QUEUE_CLASS<T>::full()
{
    return (this->size() == myMaxSize);
}


#endif // _SIZED_STL_QUEUE_
