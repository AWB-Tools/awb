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
 * @author David Goodwin, Joel Emer, Carl Beckmann
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
// we need this so we can statically know the type of "theController"
// in the API functions declared using CONTROLLER_BASE_EXTERNAL_FUNCTION
#include "asim/provides/controller_alg.h"


static void PrintInfo();


//
// Main program for standalone version of asim.
// This just calls the worker routine provided by the base class or its derivative.
//
int
main (INT32 argc, char *argv[], char *envp[])
{
  // just instantiate the classic controller, and call its main:
  theController.main( argc, argv, envp );
}


//
// Main program for standalone version of asim, the Asim "classic" controller.
//
int CONTROLLER_CLASS::main(INT32 argc, char *argv[], char *envp[])
{
    // force line buffering for reasonable synchronization of stderr and
    // stdout in batch runs
    setlinebuf(stdout);
    setlinebuf(stderr);

    // shared buffers between C-style stdio and C++-style streamio
    cin.sync_with_stdio();
    cout.sync_with_stdio();
    cerr.sync_with_stdio();

    theController.PrintInfo();

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
    theController.PartitionArgs(argc, argv);

    //
    // Initialize by calling the controller,
    // it will initialize system and feeder.
    if (!theController.CMD_Init(fdArgc, fdArgv, sysArgc, sysArgv, envp)) {
        cout << "Performance model failed to initialize" << endl;
        return(1);
    }

    //
    // Parse awb's arguments.
    if (!theController.ParseEvents(awbArgc, awbArgv)) {
        theController.Usage(argv[0], stdout);
        return(1);
    }

    //
    // Start the awb workbench...    
    AWB_Activate();

    //
    // Let the system execute until our scheduled options are complete.
    theController.CMD_SchedulerLoop();

    return(0);
}


void
CONTROLLER_CLASS::PrintInfo()
{
    cout << "ASIM, Copyright (c) 1999 - 2006, Intel Corporation" << endl
         << "Developed by VSSAD, Maintained and Enhanced by the AMI Group" << endl << endl;
}

