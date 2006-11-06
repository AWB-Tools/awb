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
  * @file DRALTag.h
  */

#ifndef _DRALTAG_H
#define _DRALTAG_H

#include "asim/draldb_syntax.h"
#include "asim/fvaluevector.h"

#include "qglobal.h"
#include "qstring.h"

/**
  * @brief
  * This is a small enumeration for the different types of values
  * a tag can hold on.
  *
  * Put long explanation here
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
typedef enum
{
    TagUnknownType = 0,
    TagStringValue,
    TagIntegerValue,        ///< 64 bits UINT64
    TagFPValue,             ///< 64 bits IEEE double
    TagSetOfValues          ///< list of values (INT64)
} TagValueType;

/** @typdef SOVList
  * @brief
  * Set of values is a vector of unsigned integers.
  */
typedef FValueVector<UINT64> SOVList;

/**
  * @brief
  * This class represents the capacity of a node with a defined
  * layout.
  * WARNING!!! WARNING!!! WARNING!!!
  * Be careful when dealing with this class: when the object is
  * destroyed it automatically deletes the vector. Don't have
  * various elements pointing to the same allocated array of
  * integers!!!
  */
class NodeSlot
{

    public :
        /**
         * Creator of this class. Sets default values.
         *
         * @return new object.
         */
        NodeSlot()
        {
            dimVec = NULL;
            specDimensions = 0;
        }

        /**
         * Copy creator. Makes a copy of a previous NodeSlot.
         *
         * @return new object.
         */
        NodeSlot(const NodeSlot& slot)
        {
            copyVector(slot.specDimensions, slot.dimVec);
        }

        /**
         * Creator with dimension parameter
         *
         * @return new object.
         */
        NodeSlot(UINT16 numDim)
        {
            specDimensions = numDim;
            dimVec = new UINT32[numDim];
        }

        /**
         * Creator with vector parameter
         *
         * @return new object.
         */
        NodeSlot(UINT16 numDim, UINT32 * dim)
        {
            copyVector(numDim, dim);
        }

        /**
         * Destructor of this class. Frees the vector if necessary.
         *
         * @return destroys the object.
         */
        ~NodeSlot()
        {
            if(dimVec != NULL)
            {
                delete [] dimVec;
            }
        }

        /**
         * Assignation operator of this class.
         *
         * @return void.
         */
        void
		operator=(const NodeSlot& orig)
        {
            if(dimVec != NULL)
            {
                delete [] dimVec;
            }
            copyVector(orig.specDimensions, orig.dimVec);
        }

        /**
         * Comparison of two node slots.
         *
         * @return if the two node slots are equal.
         */
        bool
		operator==(const NodeSlot& cmp)
        {
            if(specDimensions != cmp.specDimensions)
            {
                return false;
            }
            else
            {
                for(UINT16 i = 0; i < specDimensions; i++)
                {
                    if(dimVec[i] != cmp.dimVec[i])
                    {
                        return false;
                    }
                }
                return true;
            }
        }

        /**
         * Clears the content of the slot.
         *
         * @return void.
         */
		void
		clearSlot()
		{
			for(UINT16 i = 0; i < specDimensions; i++)
			{
				dimVec[i] = 0;
			}
		}

		/**
         * Dumps the content of the node slot.
         *
         * @return a string with the dump.
         */
        QString
		dump()
        {
            QString ret = "";

            ret = "Dim " + QString::number(specDimensions);
            if(specDimensions > 0)
            {
                ret += ": ";
                for(UINT16 i = 0; i < specDimensions - 1; i++)
                {
                    ret += QString::number(dimVec[i]) + ", ";
                }
                ret += QString::number(dimVec[specDimensions - 1]);
            }
            return ret;
        }

    private :
        /**
         * Copies a integer vector to the NodeSlot.
         *
         * @return void.
         */
        void
		copyVector(UINT16 numDim, UINT32 * dim)
        {
            specDimensions = numDim;
            if(specDimensions != 0)
            {
                dimVec = new UINT32[specDimensions];
                for(UINT32 i = 0; i < specDimensions; i++)
                {
                    dimVec[i] = dim[i];
                }
            }
            else dimVec = NULL;
        }

    public :
        UINT16 specDimensions; ///< Number dimensions of the slot.
        UINT32 * dimVec;       ///< Capacity of each dimension.
} ;

#endif
