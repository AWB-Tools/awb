/*
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
 
/**
 * @file
 * @author David Goodwin
 * @brief Asynchronous exception delivery (multicast).
 */

// ASIM core
#include "asim/syntax.h"
#include "asim/except.h"

// _exceptrefcount_
// ASIM_MM_DEFINE(ASIM_EXCEPTDATA_CLASS, MAX_EXCEPTIONS); 

UINT64 ASIM_EXCEPTDATA_CLASS::uniqueExceptId = 0;

