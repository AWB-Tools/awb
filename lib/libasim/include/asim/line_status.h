/*****************************************************************************
*
* @brief Header file for Line Status
*
* @author Oscar Rosell created the file from a part of cache.h
*
* Copyright (C) 2005-2006 Intel Corporation
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
* 
*
*****************************************************************************/

#ifndef LINE_STATUS_H
#define LINE_STATUS_H

//
// Major Cache Line Coherency States
//
typedef enum
{ 
    S_PERFECT,
    S_WARM,
    S_MODIFIED, 
    S_EXCLUSIVE, 
    S_SHARED, 
    S_INVALID, 
    S_FORWARD, 
    S_LOCKED,
    S_RESERVED,
    S_MAX_LINE_STATUS 
} LINE_STATUS;

static char * LINE_STATUS_STRINGS[S_MAX_LINE_STATUS] =
{ 
    "Perfect",
    "Warm",
    "Modified", 
    "Exclusive", 
    "Shared", 
    "Invalid", 
    "Forward",
    "Locked",
    "Reserved"
};

#endif
