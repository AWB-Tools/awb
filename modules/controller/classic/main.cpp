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


// ASIM public modules
#include "asim/provides/controller.h"

// ASIM local module
#include "args.h"


static void PrintInfo();


int
main (INT32 argc, char *argv[], char *envp[])
/*
 * Main program for standalone version of asim.
 */
{
    // force line buffering for reasonable synchronization of stderr and
    // stdout in batch runs
    setlinebuf(stdout);
    setlinebuf(stderr);

    // shared buffers between C-style stdio and C++-style streamio
    cin.sync_with_stdio();
    cout.sync_with_stdio();
    cerr.sync_with_stdio();

    PrintInfo();

    // Initialize awb
    //
    AWB_Initialize();

    //
    // Initialize for stack dumping on segfaults
    StackDumpInit(argv[0]);

    // 
    // Initialize memory tracking routine
    StackTraceInit(argv[0]); 

    //
    // Partition arguments for awb, system, and feeder.
    PartitionArgs(argc, argv);

    //
    // Initialize by calling the controller,
    // it will initialize system and feeder.
    //
    if (!CMD_Init(fdArgc, fdArgv, sysArgc, sysArgv, envp)) {
        cout << "Performance model failed to initialize" << endl;
        return(1);
    }

    //
    // Parse awb's arguments.
    //
    if (!ParseEvents(awbArgc, awbArgv)) {
        Usage(argv[0], stdout);
        return(1);
    }

    //
    // Start the awb workbench...    
    AWB_Activate();

    //
    // Let the system execute until our scheduled options are complete.
    CMD_SchedulerLoop();

    return(0);
}


void
PrintInfo()
{
    cout << "ASIM, Copyright (c) 1999 - 2006, Intel Corporation" << endl
         << "Developed by VSSAD, Maintained and Enhanced by the AMI Group" << endl << endl;
}

