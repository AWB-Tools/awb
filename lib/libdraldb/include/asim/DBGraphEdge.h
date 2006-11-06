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
  * @file DBGraphEdge.h
  */

#ifndef _DRALDB_DBGRAPHEDGE_H
#define _DRALDB_DBGRAPHEDGE_H

// Qt library
#include <qstring.h>
#include <qintdict.h>

#include "asim/draldb_syntax.h"
#include "asim/DBGraphNode.h"
#include "asim/DralDBDefinitions.h"

/**
  * @brief
  * This class represents a DRAL graph edge.
  *
  * @description
  * A DRAL graph edge is used to store all the information of  
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class DBGraphEdge
{
    public:
        DBGraphEdge (
                      DBGraphNode* source_node,
                      DBGraphNode* destination_node,
                      UINT16 edge_id,
                      UINT32 bandwidth,
                      UINT32 latency,
                      QString name
                      );

        ~DBGraphEdge();

        inline DBGraphNode* getSourceNode();
        inline DBGraphNode* getDestinationNode();
        inline QString getSourceNodeName();
        inline QString getDestinationNodeName();
        inline UINT16 getEdgeId();
        inline UINT32 getBandwidth();
        inline UINT32 getLatency();
        inline QString  getName();
        inline QString  getCompleteName();

    private:

        // DRAL edge attributes
        UINT32 bandwidth; // Maximum bandwidth of the egdes (items per cycle).
        UINT32 latency; // Latency of the edge.
        DBGraphNode* source_node; // Source node of the edge.
        DBGraphNode* destination_node; // Destination node of the edge.
        UINT16 edge_id; // Holds the id of the edge. Unique value.
        QString edgeName; // The name of the edge.
};

/**
  * @typedef DBGEList
  * @brief
  * This struct is a dictionary of edges indexed by integers.
  */
typedef QIntDict<DBGraphEdge> DBGEList;

/**
 * Returns the source node of the edge.
 *
 * @return the source node.
 */
DBGraphNode*
DBGraphEdge::getSourceNode()
{
	return source_node;
}

/**
 * Returns the destination node of the edge.
 *
 * @return the destination node.
 */
DBGraphNode*
DBGraphEdge::getDestinationNode() 
{
	return destination_node;
}

/**
 * Returns the source node name of the edge.
 *
 * @return the source node name.
 */
QString
DBGraphEdge::getSourceNodeName()
{
	return (source_node->getName());
}

/**
 * Returns the destination node name of the edge.
 *
 * @return the destination node name.
 */
QString
DBGraphEdge::getDestinationNodeName()
{
	return (destination_node->getName());
}

/**
 * Returns the id of the edge.
 *
 * @return the id.
 */
UINT16
DBGraphEdge::getEdgeId()
{
	return edge_id;
}

/**
 * Returns the bandwidth of the edge.
 *
 * @return the bandwidth.
 */
UINT32
DBGraphEdge::getBandwidth()
{
	return bandwidth;
}

/**
 * Returns the latency of the edge.
 *
 * @return the latency.
 */
UINT32
DBGraphEdge::getLatency()
{
	return latency;
}

/**
 * Returns the name of the edge.
 *
 * @return the name.
 */
QString
DBGraphEdge::getName()
{
	return edgeName;
}

/**
 * Returns the name of the edge followed by the name of the source
 * node plus the name of the destination node.
 *
 * @return the complete name.
 */
QString
DBGraphEdge::getCompleteName()
{
	return edgeName+";"+source_node->getName()+";"+destination_node->getName();
}

#endif
