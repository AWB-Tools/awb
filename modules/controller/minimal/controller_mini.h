/*
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
 * @author David Goodwin
 * @brief
 */

#ifndef _CONTROLER_MINI_
#define _CONTROLLER_MINI_


// ASIM core
#include "asim/syntax.h"

// ASIM modules 
#include "asim/provides/system.h"

// ASIM local module


/*******************************************************************/

/*
 * Print command-line usage information for feeder and system.
 */

extern void CMD_Usage (FILE *file);


/********************************************************************
 *
 * Global system object.
 *
 ********************************************************************/

extern ASIM_SYSTEM asimSystem;


#endif /* _CMD_ */
