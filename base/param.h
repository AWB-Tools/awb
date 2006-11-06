/**************************************************************************
 *Copyright (C) 2002-2006 Intel Corporation
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
 * @author Artur Klauser
 * @brief support for dynamic parameters
 */

#ifndef _PARAM_
#define _PARAM_ 1

// generic C++
#include <string>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"

/// Set a dynamic parameter to a new value.
bool               /// @returns true if parameter was found
SetParam (
    char * name,   ///< parameter name to set
    char * value); ///< value to set param to

/// List the dynamic parameters that are available.
void ListParams (void);

#endif // _PARAM_
