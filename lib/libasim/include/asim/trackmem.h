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
 * @author Shubu Mukherjee
 * @brief Debugging support: track memory allocated by different routines.
 */

#ifndef _trackmem_h
#define _trackmem_h

// ASIM core
#include "asim/syntax.h"
#include "asim/stateout.h"

#if (defined(HOST_LINUX) && defined(__GNUC__))
// memory tracking works only with Tru64 because the strack tracing is OS-specific
// for some reason, this seg faults with the gem compiler in strlen (problem with 
// parsing mangled c++ names?), but works fine with gnuc (?) -Shubu
#else
// #define TRACK_MEMORY_ALLOCATION 	1
#endif

#ifdef TRACK_MEMORY_ALLOCATION

void StackTraceInit(char * filename); 
void DumpMemAllocInfo(STATE_OUT stateOut); 
char *StackTraceParentProc(); 

#else

#define StackTraceInit(filename)
#define DumpMemAllocInfo(stateOut)	

char *StackTraceParentProc(); 

#endif

#endif 
