/*
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
 * @author David Goodwin & Joel Emer
 * @brief Asim argument parsing
 */

// generic
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/atoi.h"
#include "asim/port.h"
#include "asim/mesg.h"
#include "asim/profile.h"
#include "asim/state.h"
#include "asim/registry.h"
#include "asim/trace.h"
#include "asim/ioformat.h"
#include "asim/xcheck.h"
#include "asim/param.h"

// ASIM public modules
#include "asim/provides/controller.h"
// we need this so we can statically know the type of "theController"
// in the API functions declared using CONTROLLER_BASE_EXTERNAL_FUNCTION
#include "asim/provides/controller_alg.h"


// allocate and clean up just additional fields in x86 controller,
// since the base class will handle the base class fields
CONTROLLER_X86_CLASS::CONTROLLER_X86_CLASS() {
    knobsArgv = NULL;
    CheckPointFileName = NULL;
    perf_force = 0;
    perf_explain = 0;
}

CONTROLLER_X86_CLASS::~CONTROLLER_X86_CLASS() {
    if (knobsArgv) {
        delete [] knobsArgv;
    }
    if (CheckPointFileName) {
	delete CheckPointFileName;
    }
}

//
// Partition arguments into awb's, system's and feeder's, and knobs
//
void
CONTROLLER_X86_CLASS::PartitionArgs (INT32 argc, char **argv)
{
    // initialize knobs argument vector:
    knobsArgc = 0;
    ASSERTX(knobsArgv == NULL);
    knobsArgv = new char *[argc+1]; 

    knobsArgs = false;

    // do most of the work in the parent class:
    theController.CONTROLLER_CLASS::PartitionArgs( argc, argv );

    // null-terminate the knobs argument vector:
    knobsArgv[knobsArgc] = NULL;
}

// Partition arguments into awb's, system's and feeder's,
// one single loop iteration.
void
CONTROLLER_X86_CLASS::PartitionOneArg (INT32 argc, char *argv[], INT32 &i)
{
        // --         following args are awb's
        // --feeder   following args are feeder's
        // --system   following args are system's
        // --knobs    following args are knobs

        if (strcmp(argv[i], "--knobs") == 0) 
        {
            feederArgs = false;
            systemArgs = false;
            knobsArgs = true;
        }
        // if you get anything that starts with "--",
        // then this is the end of the knobs section...
        else if (strncmp(argv[i], "--", 2) == 0)
        {
            knobsArgs = false;
            // ...but the parent class might still have some work to do:
            theController.CONTROLLER_CLASS::PartitionOneArg (argc, argv, i);
        }
        // a real argument...
        else 
	{
            if (knobsArgs) 
            {
                // a real knobs argument, so add it to the knobs arg list:
                knobsArgv[knobsArgc++] = argv[i];
            }
            else 
            {
                // not a knobs argument? handle it in the parent class:
                theController.CONTROLLER_CLASS::PartitionOneArg (argc, argv, i);
            }
        }
}

// Parse command line arguments
// Return -1 if no argument is consumed, 0 if one argument consumed, 1 if two, 2 if three ...
int
CONTROLLER_X86_CLASS::ParseVariables(char **argv, UINT32 argc)
{
    // options specific to this derived class:
    if ((strcmp(argv[0], "-snrfile") == 0))
    {
        CheckPointFileName = new char[strlen(argv[1])+1];
        strcpy(CheckPointFileName, argv[1]);
        return 1;
    }
    else
    {
        // not one of the above?  then handle it in the parent class:
        return theController.CONTROLLER_CLASS::ParseVariables(argv, argc);
    }
}

// Parse command line arguments, 
// return false if there is a parse error.
// Advance the argument pointer i, passed by reference.
bool
CONTROLLER_X86_CLASS::ParseOneEvent (INT32 argc, char *argv[], INT32 &i)
{
        //--------------------------------------------------------------------
        // if that fails, try parsing these additional arguments:
        //--------------------------------------------------------------------
        // -ptsi <n>     turn on pipetrace after committing 'n' insts.
        //
	if ((strcmp(argv[i], "-ptsi") == 0) && (argc > (i+1))) {
            CMD_Ptv(true, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -di <n>     dump state after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-di") == 0) && (argc > (i+1))) {
            CMD_SaveCheckpoint(ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -ri <n>     restore state after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-ri") == 0) && (argc > (i+1))) {
            CMD_LoadCheckpoint(ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things off 'by instruction'
        //--------------------------------------------------------------------
        // -ptei <n>     turn off pipetrace after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-ptei") == 0) && (argc > (i+1))) {
            CMD_Ptv(false, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things on 'by macro instruction'
        //--------------------------------------------------------------------
        // -dm <n>     dump state after committing 'n' macro insts.
        //
        else if ((strcmp(argv[i], "-dm") == 0) && (argc > (i+1))) {
            CMD_SaveCheckpoint(ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -rm <n>     restore state after committing 'n' macro insts.
        //
        else if ((strcmp(argv[i], "-rm") == 0) && (argc > (i+1))) {
            CMD_LoadCheckpoint(ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things on 'by cycle'
        //--------------------------------------------------------------------
        // -ptsc <n>     turn on pipetrace on cycle 'n'
        //
        else if ((strcmp(argv[i], "-ptsc") == 0) && (argc > (i+1))) {
            CMD_Ptv(true, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -dc <n>     dump state after cycle 'n'
        //
        else if ((strcmp(argv[i], "-dc") == 0) && (argc > (i+1))) {
            CMD_SaveCheckpoint(ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -rc <n>     restore state after cycle 'n'
        //
        else if ((strcmp(argv[i], "-rc") == 0) && (argc > (i+1))) {
            CMD_LoadCheckpoint(ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things off 'by cycle'
        //--------------------------------------------------------------------
        // -ptec <n>     turn off pipetrace on cycle 'n'
        //
        else if ((strcmp(argv[i], "-ptec") == 0) && (argc > (i+1))) 
        {
            CMD_Ptv(false, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things on 'by nanosecond'
        //--------------------------------------------------------------------
        // -ptsn <n>     turn on pipetrace on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-ptsn") == 0) && (argc > (i+1))) {
            CMD_Ptv(true, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }        
        // -dn <n>     dump state after nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-dn") == 0) && (argc > (i+1))) {
            CMD_SaveCheckpoint(ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -rn <n>     restore state after nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-rn") == 0) && (argc > (i+1))) {
            CMD_LoadCheckpoint(ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things off 'by nanosecond'
        //--------------------------------------------------------------------
        // -pten <n>     turn off pipetrace on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-pten") == 0) && (argc > (i+1))) 
        {
            CMD_Ptv(false, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        else if ((strcmp(argv[i], "-skc") == 0) && (argc > (i+1)))
        {
            ++i;
            long int inst_val = 0;
            char * dup_str = argv[i];
            while(dup_str != NULL)
            {
                inst_val = strtol(dup_str, NULL, 10);
                dup_str = strstr(dup_str, ",");
                if(dup_str)
                {
			dup_str = strpbrk(dup_str, "0123456789"); // try to find the first digit
                }
                CMD_EmitStats(ACTION_INST_ONCE, inst_val);		
            }
	    //CMD_EmitStats(ACTION_INST_ONCE, atoi_general(argv[++i]));
       }
       else if ((strcmp(argv[i], "-ski") == 0) && (argc > (i+1)))
       {
            ++i;
            long int inst_val = 0;
            char * dup_str = argv[i];
            while(dup_str != NULL)
            {
                inst_val = strtol(dup_str, NULL, 10);
                dup_str = strstr(dup_str, ",");
                if(dup_str)
                {
                        dup_str = strpbrk(dup_str, "0123456789"); // try to find the first digit
                }
                CMD_EmitStats(ACTION_INST_ONCE, inst_val);
                CMD_ResetStats(ACTION_INST_ONCE, inst_val);
            }
        }
        else if ((strcmp(argv[i], "-skp") == 0) && (argc > (i+1)))
        {
            ++i;
            CMD_EmitStats(ACTION_INST_PERIOD, atoi_general(argv[i]));
            CMD_ResetStats(ACTION_INST_PERIOD, atoi_general(argv[i]));
        }
        //--------------------------------------------------------------------
        // if all else fails, try the parent class:
        //--------------------------------------------------------------------
        else {
            return theController.CONTROLLER_CLASS::ParseOneEvent (argc, argv, i);
        }

    return(true);
}


void
CONTROLLER_X86_CLASS::Usage (char *exec, FILE *file)
/*
 * Dump general stand-alone usage, and then specific usage for the feeder
 * and system.
 */
{
    ostringstream os;

    os << "\nUsage: " << exec
       << " <arg1> <arg2> .. --feeder <feeder args> --system <system args>\n"
       << "\n" << exec << " flags:\n"
       << "\t-h\t\t\tPrint this help\n"
       << "\n"
       << "\t-cf <c>\t\t\tFile holding commands for awb to execute after init\n"
       << "\t-wb <w>\t\t\tUse workbench 'w' instead of built-in workbench\n"
       << "\n"
       << "\t-n <n>\t\t\tRun simulation for <n> nanoseconds\n"
       << "\t-c <n>\t\t\tRun simulation for <n> cycles\n"
       << "\t-i <n>\t\t\tRun simulation for <n> committed insts/uops.\n"
       << "\t-m <n>\t\t\tRun simulation for <n> committed macro insts.\n"
       << "\t-s <f>\t\t\tDump statistics to file 'f' on exit\n"
       << "\n"
       << "\t-t\t\t\tTurn on instruction tracing\n"
       << "\t-tm\t\t\tSet Trace Mask\n"
       << "\t-tms\t\t\tSet Trace Mask using a comma-separated String\n"
       << "\t-tr [</regex/[=012]]>\tSet trace level by regular expression. Can be given multiple times.\n"
       << "\t\t\t\tIf not specified, the trace level will default to 1 and the regex to .*\n"
       << "\t-rbs\t\t\tDump Buffer Stats in Stats file\n"
       << "\t-rps\t\t\tDump Port Stats in Stats file\n"
       << "\tTurning various things on and off:\n"
       << "\t\tSyntax: -[drcepst(pt)][se][cinm] <n>\n"
       << "\t\t\t1st char:\n"
       << "\t\t\t\tc ... stripChart\n"
       << "\t\t\t\te ... Events\n"
       << "\t\t\t\tp ... Profiling\n"
       << "\t\t\t\ts ... Stats\n"
       << "\t\t\t\tt ... Tracing\n"
       << "\t\t\t\tt ... Dumping\n"
       << "\t\t\t\tt ... Restoring\n"
       << "\t\t\t\tpt ... PipeTrace\n"
       << "\t\t\t2nd char:\n"
       << "\t\t\t\ts ... Start\n"
       << "\t\t\t\te ... End\n"
       << "\t\t\t3rd char:\n"
       << "\t\t\t\tc ... Cycle\n"
       << "\t\t\t\ti ... commited Instructions\n"
       << "\t\t\t\tn ... nanoseconds\n"
       << "\t\t\t\tm ... Macro Instructions (equivalent to i on non-x86uop simulators)\n"
       << "\t\t\t<n>: ........ # of cycles or instructions or nanoseconds or macro instructions\n"
       << "\n"
       << "\t\t\t-t[cinm] [</regex/[=012]]> <n>\n"
       << "\t\t\t\tLike -tr but delay the effect.\n"
       << "\t\t\t-tr[cinm] <n>\n"
       << "\t\t\t\tDelay the effect of all previous -tr commands until time <n>.\n"
       << "\t\tExample:\n"
       << "\t\t\t-tsc 1000 ... start tracing at cycle 1000\n"
       << "\n"
       << "\t-p <n>\t\t\tTurn profiling on\n"
       << "\t-d <n>\t\t\tTurn warnings and debug information on\n"
       << "\t-e \t\t\tTurn events on\n"
       << "\n"
       << "\t-pc <n>\t\t\tIndicate progress info every <n> cycles\n"
       << "\t-pi <n>\t\t\tIndicate progress info every <n> committed insts.\n"
       << "\t-pn <n>\t\t\tIndicate progress info every <n> nanoseconds.\n"
       << "\n"
       << "\t-sc <n>\t\t\tEmit stats file every <n> cycles\n"
       << "\t-sn <n>\t\t\tEmit stats file every <n> nanoseconds\n"
       << "\t-si <n>\t\t\tEmit stats file every <n> instructions\n"
       << "\t-sm <n>\t\t\tEmit stats file every <n> macro instructions\n"
       << "\n"
       << "\t-rsc <n>\t\t\tReset stats on cycle <n>\n"
       << "\t-rsi <n>\t\t\tReset stats on instruction <n>\n"
       << "\t-rsm <n>\t\t\tReset stats on macro instruction <n>\n"
       << "\t-rsn <n>\t\t\tReset stats on nanosecond <n>\n"
       << "\tAdding a p after the previous flags (for instance -rscp) will reset stats periodically after <n> events.\n"
       << "\n"
       << "\t-param <name>=<value>\tdefine dynamic parameter <name> = <value>\n"
       << "\t-listparams\t\tlist all registered dynamic parameters\n"
       << "\t-listmasks\t\tlist the possible mask strings\n"
       << "\t-listnames\t\tlist the possible traceable object names\n"
       << "\n"
       << "\t-xcheck <file>\t\tGenerate output for cross-checking to file <file>\n"
       << "\n\n\tWARNING! The nanosecond options are meaningless if running a model without a clockserver system!\n"
       << endl;

    fputs (os.str().c_str(), file);
    CMD_Usage(file);
}
