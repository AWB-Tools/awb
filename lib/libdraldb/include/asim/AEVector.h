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
  * @file  AEVector.h
  */

#ifndef _AEVECTOR_H_
#define _AEVECTOR_H_

#include "qglobal.h"

#ifdef Q_OS_UNIX
  #include <string.h>
  #include <strings.h>
#endif

/**
  * @brief
  * This class implements a dynamic array that grows in chunks of
  * SEGMENTSIZE and has a maximum size of SEGMENTSIZE * MAXSEGMENTS.
  *
  * @description
  * This dynamic array just can be used to insert and consult
  * objects. The array uses segments to grow dynamically. Each
  * index is mapped to a segment and has a displacement within this
  * segment. When an object in an index is allocated or is appended
  * in the vector, the object checks if the segment is allocated and
  * in negative case the segment is allocated.
  *
  * @author Federico Ardanaz
  * @date started at 2002-12-10
  * @version 0.1
  */
template<class T> class AEVectorNode
{
    public:
        /**
         * Creator of this class. Alloc space for sz objects.
         *
         * @return new object.
         */
        AEVectorNode(int sz, int idx)
        {
            size = sz;
            index = idx;
            array = new T[sz];
            Q_ASSERT(array!=NULL);
        }

        /**
         * Destructor of this class. Deletes the allocated objectes.
         *
         * @return destroys the object.
         */
       ~AEVectorNode()
        {
            delete [] array;
        }

    public:
        int size; // Size of the node (in objects).
        int index; // Index of the node.
        T*  array; // Array sotring all the objects.
};

template<class T, int SEGMENTSIZE, int MAXSEGMENTS> class AEVector
{
    public:
        /**
         * Creator of this class. Sets to zero all the segment
         * pointers and get the nearest mod 2 value for segment
         * size.
         *
         * @return new object.
         */
        AEVector()
        {
            Q_ASSERT(SEGMENTSIZE>0);
            Q_ASSERT(MAXSEGMENTS>0);
            numSegments=0;

            // Set to NULL all the pointers to segments.
            bzero((char*)segvector,(size_t) (sizeof(segvector[0])*MAXSEGMENTS));

            // look for nearest (above) mod 2 value for segmentsize
            int popcnt=0;
            int logcnt=0;
            int osize = SEGMENTSIZE;
            while (osize>0)
            {
                if (osize&1) popcnt++;
                osize = osize >> 1;
                ++logcnt;
            }
            logcnt -= (popcnt==1) ? 1 : 0;
            m2SegmentSize = (int)(1)<<logcnt;
            segmentShift = logcnt;
            posMask = (0xffffffffU) >> ((sizeof(int)*8)-logcnt);

            reset();
            /*printf("segsize=%d, m2segsize=%d, segShift=%d, mask=0x%x\n",
                    SEGMENTSIZE,m2SegmentSize,segmentShift,posMask);*/
        }

        /**
         * Destructor of this class. Deletes the allocated objectes.
         *
         * @return destroys the object.
         */
        ~AEVector()
        {
            clear();
        }

        /**
         * Return the element on the position idx. To know which
         * segment the requested object is stored the idx is shifted
         * segmendShift bits. Finally to know which position within
         * the segment the object is stored the idx is masked with
         * posMask.
         *
         * @return the object in the position idx.
         */
        inline T operator[] (int idx) const
        {
            //int segment = idx >> segmentShift;
            //int pos     = idx &  posMask;
            //Q_ASSERT(segment<MAXSEGMENTS);
            //Q_ASSERT(segvector[segment]!=NULL);
            //return segvector[segment]->array[pos];
            return segvector[(idx>>segmentShift)]->array[(idx &  posMask)];
        }

        /**
         * Return a reference to the element on the position idx.
         * The access is asserted and maxUsed and minUsed Idx are
         * updated.
         *
         * @return the object in the position idx.
         */
        inline T& operator[] (int idx)
        {
            int segment = idx >> segmentShift;
            //int pos     = idx &  posMask;
            //Q_ASSERT(segment<MAXSEGMENTS);
            if (segvector[segment]==NULL)
            {
                allocateSegment(segment);
            }
            maxUsedIdx = QMAX(maxUsedIdx,idx);
            minUsedIdx = QMIN(minUsedIdx,idx);
            return (segvector[segment]->array[(idx &  posMask)]);
        }

        /**
         * Equal to [].
         *
         * @return the object in the position idx.
         */
        inline T at (int idx) const
        {
            //int segment = idx >> segmentShift;
            //int pos     = idx &  posMask;
            //Q_ASSERT(segment<MAXSEGMENTS);
            //Q_ASSERT(segvector[segment]!=NULL);
            //return segvector[segment]->array[pos];
            return segvector[(idx>>segmentShift)]->array[(idx &  posMask)];
        }

        /**
         * Equal to [].
         *
         * @return the object in the position idx.
         */
        inline T& ref (int idx) const
        {
            //int segment = idx >> segmentShift;
            //int pos     = idx &  posMask;
            //Q_ASSERT(segment<MAXSEGMENTS);
            //Q_ASSERT(segvector[segment]!=NULL);
            //return segvector[segment]->array[pos];
            return segvector[(idx>>segmentShift)]->array[(idx &  posMask)];
        }

        /**
         * Assigns an object after the last position in the vector.
         *
         * @return void.
         */
        inline void append(T value)
        {
            assign(nextIdx++,value);
        }

        /**
         * Assigns an object in the position idx. If the segment
         * where the object is mapped is NULL the segment is allocated
         * and after the object is written. maxUsedIdx and minUsedIdx
         * are updated.
         *
         * @return void.
         */
        inline void assign(int idx,T value)
        {
            int segment = idx >> segmentShift;
            //int pos     = idx &  posMask;
            //Q_ASSERT(segment<MAXSEGMENTS);
            if (segvector[segment]==NULL)
            {
                allocateSegment(segment);
            }
            segvector[segment]->array[(idx & posMask)] = value;
            maxUsedIdx = QMAX(maxUsedIdx,idx);
            minUsedIdx = QMIN(minUsedIdx,idx);
        }

        /**
         * Returns true if the segment where the idx position is
         * mapped is allocated. Returns false otherwise.
         *
         * @return true if the segment of the idx is allocated.
         */
        inline bool hasElement(int idx)
        {
            int segment = idx >> segmentShift;
            if(segment>=MAXSEGMENTS)
            {
                return false;
            }
            return (segvector[segment]!=NULL);
        }

        /**
         * If the segment where the object is mapped is NULL the
         * segment is allocated
         *
         * @return void.
         */
        inline void allocateElement(int idx)
        {
            int segment = idx >> segmentShift;
            Q_ASSERT(segment<MAXSEGMENTS);
            if (segvector[segment]==NULL)
            {
                allocateSegment(segment);
            }
        }

        /**
         * Clears all the allocated segments.
         *
         * @return void.
         */
        inline void clear()
        {
            for (int i=0;i<MAXSEGMENTS;i++)
            {
                if (segvector[i]!=NULL)
                {
                    delete segvector[i];
                    segvector[i]=NULL;
                }
            }
            reset();
        }

        /**
         * Resets the index fields of the class.
         *
         * @return void.
         */
        inline void reset()
        {
            maxUsedIdx = -1;
            minUsedIdx = 0x7fffffff;
            nextIdx = 0;
        }

    public:
        /**
         * The number of segments allocated.
         *
         * @return the num of segments.
         */
        inline int getNumSegments()
        {
            return numSegments;
        }

        /**
         * The segment size used in the vector.
         *
         * @return the segment size.
         */
        inline int getSegmentSize()
        {
            return m2SegmentSize;
        }

        /**
         * The maximum used index in the vector.
         *
         * @return the maximum used index.
         */
        inline int getMaxUsedIdx()
        {
            return maxUsedIdx;
        }

        /**
         * The minimum used index in the vector.
         *
         * @return the minimum used index.
         */
        inline int getMinUsedIdx()
        {
            return minUsedIdx;
        }

    protected:
        /**
         * Allocates the segment number segment.
         *
         * @return void.
         */
        inline void allocateSegment(int segment)
        {
            segvector[segment] = new AEVectorNode<T>(m2SegmentSize,segment);
            Q_ASSERT(segvector[segment] != NULL);
            ++numSegments;
        }

    protected:
        AEVectorNode<T>* segvector[MAXSEGMENTS]; // Pointer to the segments of the vector.
        int numSegments; // Number of allocated segments.
        int m2SegmentSize; // Segment size after computing segmentShift.
        int segmentShift; // Shift used to know to which segment an access must use.
        int posMask; // Mask used to know which position within a segment an access use.
        int maxUsedIdx; // Maximum index accessed.
        int minUsedIdx; // Minimum index accessed.
        int nextIdx; // Next index used to append data.
};

#endif
