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
  * @file DBGraphNode.cpp
  */

#include "asim/DBGraphNode.h"

/**
 * Creator of this class. Just assigns all the parameters to the
 * fields of this class.
 *
 * @return new object.
 */
DBGraphNode::DBGraphNode (QString pname,UINT16 pnode_id,UINT16 pparent_id,UINT16 pinstance)
{
    this->name = pname;
    this->node_id = pnode_id;
    this->parent_id = pparent_id;
    this->instance = pinstance;
    this->dimensions=0;
    this->capacities=NULL;
    inputbw=0;
    outputbw=0;
    totalCapacity=0;
}

/**
 * Destructor of this class. If the capacities vector is set then
 * is freed.
 *
 * @return destroys the object.
 */
DBGraphNode::~DBGraphNode()
{
	if (capacities!=NULL)
	{
		delete capacities;
	}
}

/**
 * Sets the layout of the node. Each node have a different layout
 * used to represent different types of hardware structures.
 * The only parameters needed is the number of dimensions and the
 * capacity of each dimension. totalCapacity is computed as the
 * mul of the capacity of all the dimensions.
 *
 * @return void.
 */
void
DBGraphNode::setLayout(UINT16 _dimensions, UINT32* _capacities)
{
    Q_ASSERT(_dimensions>=1);
    Q_ASSERT(_capacities!=NULL);

    dimensions=_dimensions;
    capacities = new UINT32[_dimensions];
    Q_ASSERT(capacities!=NULL);
    totalCapacity = 1;
    for (int i=0;i<_dimensions;i++)
    {
        capacities[i] = _capacities[i];
        totalCapacity *= capacities[i];
    }
}
