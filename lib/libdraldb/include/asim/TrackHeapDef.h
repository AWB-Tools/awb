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
  * @file  TrackHeapDef.h
  */

#ifndef _DRALDB_TRACKHEAPDEF_H_
#define _DRALDB_TRACKHEAPDEF_H_

#include "asim/draldb_syntax.h"
#include "asim/AEVector.h"
#include "asim/TagVec.h"
#include "asim/DRALTag.h"

/** @typedef enum TrackIdType
  * @brief
  * Enumeration of the different types of track.
  */
typedef enum
{
    TRACKIDTYPE_MOVEITEM = 0,
    TRACKIDTYPE_NODETAG,
    TRACKIDTYPE_CYCLETAG,
    TRACKIDTYPE_ENTERNODE,
    TRACKIDTYPE_EXITNODE
} TrackIdType;

/** @class TrackIdInfo
  * @brief
  * Holds the information to track the information of the different
  * types of track : move item and node tag.
  */
class TrackIdInfo
{
    public:
        UINT16   node_edge_id; // Id of the track.
        NodeSlot slot; // Node slot of the track.
} ;

/** @typedef TagIDList
  * @brief
  * Dynamic vector of tag id's.
  */
typedef AEVector<UINT16,32,128> TagIDList;

#endif
