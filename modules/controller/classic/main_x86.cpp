/*
 * *****************************************************************
 * *                                                               *
 *Copyright (C) 2004-2006 Intel Corporation
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
 * @author David Goodwin, Joel Emer, and Carl Beckmann
 * @brief Main program entry point for Asim x86 controller
 *
 * Asim Classic Controller Algorithm.
 * This is only an "algorithm" in the Asim module sense,
 * i.e. it is a specialization of the Asim "classic" controller family.
 * In this case, it specialized the "Asim classic" controller for x86 models.
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


//STATS
#include "pipe.h"
#include "stats.h"
#include "knob.h"
static void
verbose_help(){
        knob_dump_help(stdout);
	exit(EXIT_SUCCESS);
}
Knob<bool> dump_knobs("dump_knobs", "dump all knob values", false);
Knob<bool> dump_stats("dump_stats", "dump all stat names", false);
Knob<void> minus_help("help", "display verbose help info", verbose_help);


int
CONTROLLER_X86_CLASS::main (INT32 argc, char *argv[], char *envp[])
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
    //Parse Willy style knobs.
    //
    knob_parse_argv((const char**)knobsArgv);
    if (dump_knobs || dump_stats) {
    if (dump_knobs) {
    	knob_dump_value(stdout);
        }

        if (dump_stats) {
            stat_generic_dump_name(cout);
        }
    	exit(EXIT_SUCCESS);
    }

    //
    // Initialize by calling the controller,
    // it will initialize system and feeder.
    //
    if (!theController.CMD_Init(fdArgc, fdArgv, sysArgc, sysArgv, envp)) {
        cout << "Performance model failed to initialize use -h to get options" << endl;
        return(1);
    }
    
    //
    // Parse awb's arguments.
    //
    if (!theController.ParseEvents(awbArgc, awbArgv)) {
        theController.Usage(argv[0], stdout);
        return(1);
    }

    //
    //Start PTV
    //
    int full_pt = 1;
    for (UINT32 i = 0; i< awbArgc; i++)
    {
        if ((strcmp(awbArgv[i], "-ptsi") == 0) || (strcmp(awbArgv[i], "-ptsc") == 0))
        {
            full_pt=0;
        }
    }

    if(!pt.empty() && full_pt)
    {
        pipe_open_file(pt.c_str(),
            (pt_ascii ? PT_ASCII : PT_Binary),
            PIPE_NO_RECORD);
	pipetrace_closed = false;
    } else {
	pipetrace_closed = true;
    }

    //
    // Start the awb workbench...
    AWB_Activate();

    //
    // Let the system execute until our scheduled options are complete.
    theController.CMD_SchedulerLoop();

    return(0);
}
