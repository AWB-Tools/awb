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
  * @file DefaultSize.h
  * @brief Default size for some fundamental structures
  */

#ifndef _DRALDBDEFINITIONS_H_
#define _DRALDBDEFINITIONS_H_

#define DRALDB_MAJOR_VERSION 0
#define DRALDB_MINOR_VERSION 7
#define DRALDB_STR_MAJOR_VERSION "0"
#define DRALDB_STR_MINOR_VERSION "7"

// -------------------------------------------------
// TagDescVector definitions
// -------------------------------------------------
#define TAGDESCVEC_SIZE 4096

// -------------------------------------------------
// StrTable definitions
// -------------------------------------------------
#define STRTBL_DEFAULT_SIZE     10000

// -------------------------------------------------
// LiveDB definitions
// -------------------------------------------------
#define LIVEDB_DEFAULT_MAXITEMCNT 100000

// -------------------------------------------------
// Cycle bitfield-related definitions
// -------------------------------------------------
#define CYCLE_CHUNK_SIZE 4096
#define CYCLE_OFFSET_BITS 12
#define CYCLE_OFFSET_MASK 0x00000FFFU

// -------------------------------------------------
// DBGraph definitions
// -------------------------------------------------
/**
  * @def DEFAULT_DGN_SIZE
  * @brief This is default size used for hashed list of Dral Nodes.
  * Bear in mind that this MUST be a prime number.
  */
#define DEFAULT_DBGN_SIZE 2111

/**
  * @def DEFAULT_DGE_SIZE
  * @brief This is default size used for hashed list of Dral Edges.
  * Bear in mind that this MUST be a prime number.
  */
#define DEFAULT_DBGE_SIZE 99991


#define CACHED_ITEMIDS 10000

// -------------------------------------------------
// Global definitions
// -------------------------------------------------

/** @def used as a "canonical" item id string */
#define ITEMID_STR_TAG "ITEMID"

/** @def used as a "canonical" item id IN string */
#define ITEMIDIN_STR_TAG "_ITEMID_IN"

/** @def used as a "canonical" item id IN string */
#define ITEMIDXIN_STR_TAG "_ITEMIDX_IN"

/** @def used as special internal use pointer to ItemTagHeap */
#define ITEMIDX_STR_TAG "_DRALDB_ITEMIDX_ON_ITEMTAGHEAP"

#define AGEPURGE_STR_TAG "_AGEPURGED"
#define EOFPURGE_STR_TAG "_EOFPURGED"

#endif


