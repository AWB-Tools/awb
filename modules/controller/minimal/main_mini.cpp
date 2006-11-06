/*
 * *****************************************************************
 * *                                                               *
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
 * @author David Goodwin & Joel Emer
 * @brief ASIM Main
 */

// Generic
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/stackdump.h"
#include "asim/trackmem.h"
#include "asim/ioformat.h"
#include "asim/event.h"

// ASIM public modules
#include "asim/provides/controller.h"
#include "asim/provides/system.h"


// ASIM local module
#include "args_mini.h"

// Global declaration of control variables - BAD
UINT64 MaxCycles = 100;
char *StatsFileName = "stats.xml";

static void PrintInfo();


int
main (INT32 argc, char *argv[], char *envp[])
/*
 * Main program for standalone version of asim.
 */
{
    ASIM_SYSTEM system;

    // force line buffering for reasonable synchronization of stderr and
    // stdout in batch runs
    setlinebuf(stdout);
    setlinebuf(stderr);

    // shared buffers between C-style stdio and C++-style streamio
    cin.sync_with_stdio();
    cout.sync_with_stdio();
    cerr.sync_with_stdio();

    PrintInfo();

    //
    // Initailized events
    //
    EVENT(ASIM_DRAL_EVENT_CLASS::InitEvent());

    //
    // Partition arguments for awb, system, and feeder.
    //
    PartitionArgs(argc, argv);
    
    //
    // Let the system execute until our scheduled options are complete.
    //
    system = SYS_Init(0, 0, 0);
    system->SYS_Execute(MaxCycles, UINT64_MAX, UINT64_MAX);

    system->PrintModuleStats(StatsFileName);
    return(0);
}


void
PrintInfo()
{
    cout << "ASIM, Copyright (c) 1999 - 2006, Intel Corporation" << endl
         << "Developed by VSSAD, Maintained and Enhanced by the AMI Group" << endl << endl;
}
