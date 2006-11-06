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
 * @brief Main ASIM initialiaztion and driver - interface between
 * ASIM's controller, performance model, and visualizer.
 */

#ifndef _ARGS_
#define _ARGS_

// ASIM core
#include "asim/syntax.h"

extern void PartitionArgs (INT32 argc, char **argv);
extern void Usage (char *exec, FILE *file);
extern bool ParseEvents (INT32 argc, char **argv);

// Arguments partitioned into awb's, system's and feeder's

extern UINT32 origArgc, awbArgc, sysArgc, fdArgc;
extern char **origArgv, **awbArgv, **sysArgv, **fdArgv;

#endif
