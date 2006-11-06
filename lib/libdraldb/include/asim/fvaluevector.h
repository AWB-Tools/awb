// ==================================================
//Copyright (C) 2003-2006 Intel Corporation
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file fvaluevector.h
  */

#ifndef _FVALUEVECTOR_H_
#define _FVALUEVECTOR_H_

#include <stdlib.h>

/**
  * @brief
  * This is a very simple dynamic array implementation.
  * When size is exceeded a new vector is create with fourth times
  * of room.
  *
  * @description
  * This dynamic array just can be used to insert and consult
  * objects. The objects only can be removed all at once, but
  * never a subset of them. The access to an array position is
  * not asserted, so any out of bounds may raise a segmentation
  * fault. When a new object is appended and no space is left in
  * the vector, all the vector is reallocated in a new heap space
  * four times bigger than the old one.
  *
  * @author Federico Ardanaz
  * @date started at 2002-12-10
  * @version 0.1
  */
template <class T> class FValueVector
{
    public:

        /**
         * @typedef value_type
         * @brief
         * This struct is a value of the template class.
         */
        typedef T value_type;

        /**
         * @typedef pointer
         * @brief
         * This struct is a pointer of the template class.
         */
        typedef value_type* pointer;

        /**
         * @typedef iterator
         * @brief
         * This struct is a iterator of the template class.
         */
        typedef value_type* iterator;

        /**
         * @typedef reference
         * @brief
         * This struct is a reference of the template class.
         */
        typedef value_type& reference;

        /**
         * @typedef size_type
         * @brief
         * This struct is a size_type.
         */
        typedef size_t size_type;

        /**
         * Creator of this class. Creates an empty vector.
         *
         * @return new object.
         */
        inline FValueVector() : pstart( 0 ), pfinish( 0 ), pend( 0 )
        {
        }

        /**
         * Creator of this class. Creates a vector with an initial size of
         * size elements.
         *
         * @return new object.
         */
        inline FValueVector( size_type size )
        {
    	    if ( size > 0 )
    	    {
    	        pstart = new T[size];
    	        pfinish = pstart;
    	        pend = pstart + size;
    	    }
    	    else
    	    {
    	        pstart = 0;
    	        pfinish = 0;
    	        pend = 0;
    	    }
        }

        /**
         * Destructor of this class. If something is assigned to pstart
         * then is deleted.
         *
         * @return destroys the object.
         */
        inline ~FValueVector()
        {
            if (pstart!=0)
            {
                delete [] pstart;
            }
        }

        /**
         * Returns the object in the position idx.
         *
         * @return the object.
         */
        inline value_type operator[] (int idx) const
        {
            return pstart[idx];
        }

        /**
         * Returns the reference of the position idx.
         *
         * @return the reference.
         */
        inline reference operator[] (int idx)
        {
            return pstart[idx];
        }

        /**
         * Returns the object in the position idx.
         *
         * @return the object.
         */
        inline value_type at (int idx) const
        {
            return pstart[idx];
        }

        /**
         * Returns the pointer to the position idx.
         *
         * @return the pointer.
         */
        inline pointer ref (int idx) const
        {
            return pstart+idx;
        }

        /**
         * Returns the number of elements in the vector.
         *
         * @return the number of elements.
         */
        inline size_type count() const
        {
            return size();
        }

        /**
         * Returns the size of the vector.
         *
         * @return the size.
         */
        inline size_type size() const
        {
            return pfinish - pstart;
        }

        /**
         * Returns a boolean that is true if the vector is empty or false
         * if it has any object.
         *
         * @return the emptyness of the vector.
         */
        inline bool empty() const
        {
            return pstart == pfinish;
        }

        /**
         * Returns the number of slots in the vector that are still free.
         *
         * @return the number of free positions.
         */
        inline size_type capacity() const
        {
            return pend - pstart;
        }

        /**
         * Returns an iterator (pointer) to the first element of the
         * vector.
         *
         * @return the iterator.
         */
        inline iterator begin()
        {
            return pstart;
        }

        /**
         * Returns an iterator (pointer) to the last element of the
         * vector.
         *
         * @return the iterator.
         */
        inline iterator end()
        {
            return pfinish;
        }

        /**
         * Appends a new object in the vector. If no space is left in the
         * vector, then more space is allocated. Each time that more space
         * is allocated the vector size is multiplied by 4.
         *
         * @return void.
         */
        inline void append( const T& x )
        {
    	    if ( pfinish == pend )
    	    {
    	        reserve( (capacity()+1)*4 );
    	    }
    	    *pfinish = x;
    	    ++pfinish;
        }

        /**
         * All the content of the vector is erased.
         *
         * @return void.
         */
        inline void reset()
        {
            pfinish = pstart;
        }

    protected:
        /**
         * Allocs more space for the vector (n slots).
         *
         * @return void.
         */
        inline void reserve( size_t n )
        {
    	    const size_t lastSize = size();
    	    pointer tmp = growAndCopy( n, pstart, pend );
    	    pstart = tmp;
    	    pfinish = tmp + lastSize;
    	    pend = pstart + n;
        }

        /**
         * Allocs a new vector of size n and copies the content between
         * s and f (all the content of the old vector) to the new vector.
         * Finally the old vector is erased.
         *
         * @return the pointer to the new vector.
         */
        inline pointer growAndCopy( size_t n, pointer s, pointer f )
            {
                pointer newStart = new T[n];
    	        qCopy( s, f, newStart );
    	        delete [] pstart;
    	        return newStart;
            }

    private:
        pointer pstart; // Pointer to the beginning of data.
        pointer pfinish; // Pointer to the next bucket of data.
        pointer pend; // Pointer to the last bucket of data.
};

#endif
