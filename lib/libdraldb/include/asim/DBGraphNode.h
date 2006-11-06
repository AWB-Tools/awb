// ==================================================
// Copyright (C) 2003-2006 Intel Corporation
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file DBGraphNode.h
  */

#ifndef _DRALDB_DBGRAPHNODE_H
#define _DRALDB_DBGRAPHNODE_H

// Qt library
#include <qstring.h>
#include <qvaluelist.h>
#include <qdict.h>
#include <qintdict.h>

#include "asim/draldb_syntax.h"
#include "asim/DralDBDefinitions.h"

/**
  * @brief
  * This class represents a DRAL graph node.
  *
  * @description
  * A DRAL graph node is used to store all the information that
  * a DRAL trace gives us about this node.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class DBGraphNode
{
    public:
        DBGraphNode (
               QString name,
               UINT16 node_id,
               UINT16 parent_id,
               UINT16 instance
               );

        ~DBGraphNode();

        inline QString getName();
        inline UINT16 getNodeId();

        void   setLayout(UINT16 dimensions, UINT32 capacities []);

        inline UINT16 getInputBW();
        inline UINT16 getOutputBW();
        inline UINT32 getCapacity();
        inline UINT16 getDimensions();
        inline UINT32* getCapacities();

    private:
        QString name; // Name of the node. This name is a unique string for identifying the node : node_name[instance]{node_slot}.  
        UINT32  node_id; // Holds the id of the node. 
        UINT16  parent_id; // Contains the id of this node's parent.
        UINT16  instance; // Instance of the node. Needed due the existence of different nodes with the same name.
        UINT16  dimensions; // Contains the number of dimensions of the node layout.
        UINT32* capacities; // Contains the value of each dimension of the node layout.
        UINT32  totalCapacity; // Contains the number of slots of the node.
        UINT16  inputbw; // Contains the input bandwidth of the node. Not used.
        UINT16  outputbw; // Contains the output bandwidth of the node. Not used.
};

/**
  * @typedef DGNList
  * @brief
  * This struct is a dictionary of nodes indexed by integers.
  */
typedef QIntDict<DBGraphNode> DBGNList;

/**
  * @typedef DGNListByName
  * @brief
  * This struct is a dictionary of nodes indexed by name (field name of this class).
  */
typedef QDict<DBGraphNode> DBGNListByName;

/**
 * Returns the name of the node. 
 *
 * @return the name.
 */
QString
DBGraphNode::getName()
{
	return name;
}

/**
 * Returns the id of the node. 
 *
 * @return the id.
 */
UINT16
DBGraphNode::getNodeId()
{
	return node_id;
}

/**
 * Returns the input bandwidth of the node. 
 *
 * @return the input bandwidth.
 */
UINT16 
DBGraphNode::getInputBW()
{
	return inputbw;
}

/**
 * Returns the output bandwidth of the node. 
 *
 * @return the output bandwidth.
 */
UINT16
DBGraphNode::getOutputBW()
{
	return outputbw;
}

/**
 * Returns the capacity of the node. 
 *
 * @return the capacity.
 */
UINT32
DBGraphNode::getCapacity()
{
	return totalCapacity;
}

/**
 * Returns the dimensions of the node. 
 *
 * @return the dimensions.
 */
UINT16
DBGraphNode::getDimensions()
{
	return dimensions;
}

/**
 * Returns the capacities of the node. 
 *
 * @return the capacities.
 */
UINT32*
DBGraphNode::getCapacities()
{
	return capacities;
}

#endif
