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

// ASIM local module
#include "args_mini.h"


static int ParseVariables (char **argv);

// Arguments partitioned into awb's, system's and feeder's
UINT32 origArgc, awbArgc, sysArgc, fdArgc;
char **origArgv, **awbArgv, **sysArgv, **fdArgv;

// Events control variable
extern bool stripsOn; 


// Partition arguments into awb's, system's and feeder's.
void
PartitionArgs (INT32 argc, char **argv)
{
    origArgc = argc;
    origArgv = argv;

    awbArgc = sysArgc = fdArgc = 0;

    awbArgv = new char *[argc+1];   
    sysArgv = new char *[argc+1];   
    fdArgv = new char *[argc+1];    

    //
    // Partition...
    
    bool feederArgs = false;
    bool systemArgs = false;
    for (INT32 i = 1; i < argc; i++) 
    {
        // --         following args are awb's
        // --feeder   following args are feeder's
        // --system   following args are system's

        if (strcmp(argv[i], "--") == 0) 
	{
            feederArgs = systemArgs = false;
        }
        else if (strcmp(argv[i], "--feeder") == 0) 
	{
            feederArgs = true;
            systemArgs = false;
        }
        else if (strcmp(argv[i], "--system") == 0) 
	{
            feederArgs = false;
            systemArgs = true;
        }
        // a real argument, we handle -h and -t here since we want
        // them to have an immediate effect.
        else 
	{
            if (feederArgs) 
	    {
                fdArgv[fdArgc++] = argv[i];
            }
            else if (systemArgs) 
	    {
                sysArgv[sysArgc++] = argv[i];
            }
            else 
	    {
		int incr = ParseVariables(&argv[i]); 

		if (incr == -1)
		{
		    awbArgv[awbArgc++] = argv[i];
		}
		else
		{
		    i += incr; 
		}
            }
        }
    }
}

// Parse command line arguments
// Return -1 if no argument is consumed, 0 if one argument consumed, 1 if two, 2 if three ...
static int
ParseVariables(char **argv)
{
    int incr = 0; 

    // -h           print usage
    if (strcmp(argv[0], "-h") == 0) 
    {
	Usage(argv[0], stdout);
	exit(0);
    }
    // -c <n>       run simulation for 'n' cycles
    else if (strcmp(argv[0], "-c") == 0)
    {
	extern UINT64 MaxCycles;
	
        MaxCycles = atoi_general_unsigned(argv[++incr]);
    }
    else if (strcmp(argv[0], "-t") == 0) 
    {
	traceOn = true; 
    }
    else if (strcmp(argv[0], "-e") == 0) 
    {
        ASIMERROR ("There is no events support in the mini args");
    }
    else if (strcmp(argv[0], "-strip") == 0) 
    {
        stripsOn = true;
    }
    else if (strcmp(argv[0], "-p") == 0)
    {
	profileOn = true; 
    }
    else if (strcmp(argv[0], "-w") == 0)
    {
	warningsOn = true; 
    }
    else if (strcmp(argv[0], "-rps") == 0) 
    {
	registerPortStats = true; 
    }
    // -tm <n>		set trace mask (see trace.h)
    else if (strcmp(argv[0], "-tm") == 0) 
    {
        traceMask = atoi_general_unsigned(argv[++incr]);
    }
    else if (strcmp(argv[0], "-ppc") == 0) 
    {
      profilePc = atoi_general_unsigned(argv[++incr]);
    }
    // -batch        batch workbench
    //
    else if (strcmp(argv[0], "-batch") == 0) 
    {
        ASIMERROR ("ASIM no longer supports the -batch flag\n")
    }
    // -awb          interactive architect's workbench
    //
    else if (strcmp(argv[0], "-awb") == 0) 
    {
        ASIMERROR ("ASIM no longer supports the -awb flag" <<
                   " for interactive Cycledisplay mode\n")
    }
    // -cf          file holding commands for awb to execute after initializing
    //
    else if ((strcmp(argv[0], "-cf") == 0))
    {
	// awbCmdFile = argv[++incr];
    }
    // -wb <w>      use workbench 'w' instead of built-in workbench
    //
    else if ((strcmp(argv[0], "-wb") == 0))
    {
	// overrideWorkbench = argv[++incr];
    }
    // -s <f>
    else if ((strcmp(argv[0], "-s") == 0))
    {
	extern char *StatsFileName;
	
	StatsFileName = new char[strlen(argv[incr+1]+1)];  
	strcpy(StatsFileName, argv[incr+1]); 
	++incr;
    }
    // debug on
    else if ((strcmp(argv[0], "-d") == 0))
    {
	debugOn = true; 
    }
    // dynamic parameters
    else if ((strcmp(argv[0], "-param") == 0))
    {
        char * name;
        char * eq;
        char * value;

        name = argv[1];
        eq = index(argv[1], '=');
        if ( ! eq )
        {
            ASIMERROR("Invalid parameter specification in '"
                << argv[0] << " " << argv[1] << "'" << endl
                << "    Correct syntax: -param <name>=<value>" << endl);
        }
        else
        {
            value = eq + 1;
            *eq = '\0';
            if ( ! SetParam(name, value))
            {
                *eq = '=';
                ASIMWARNING("Don't know about dynamic parameter "
                    << name << endl
                    << "    ignoring command line portion '"
                    << argv[0] << " " << argv[1]  << "'" << endl);
            }
            *eq = '=';
        }
        incr = 1;
    }
    // -listparams           list dynamic parameters
    else if (strcmp(argv[0], "-listparams") == 0) 
    {
	ListParams();
	exit(0);
    }
    else
    {	
	return -1; 
    }
    
    return incr; 
}


void
Usage (char *exec, FILE *file)
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
       << "\t-batch\t\t\tTurn on batch architect's workbench\n"
       << "\t-awb\t\t\tTurn on interactive architect's workbench\n"
       << "\t-cf <c>\t\t\tFile holding commands for awb to execute after init\n"
       << "\t-wb <w>\t\t\tUse workbench 'w' instead of built-in workbench\n"
       << "\n"
       << "\t-c <n>\t\t\tRun simulation for <n> cycles\n"
       << "\t-i <n>\t\t\tRun simulation for <n> committed insts.\n"
       << "\t-s <f>\t\t\tDump statistics to file 'f' on exit\n"
       << "\n"
       << "\t-t\t\t\tTurn on instruction tracing\n"
       << "\t-tm\t\t\tSet Trace Mask\n"
       << "\t-rbs\t\t\tDump Buffer Stats in Stats file\n"
       << "\t-rps\t\t\tDump Port Stats in Stats file\n"
       << "\t-tsc <n>\t\tStart tracing on cycle <n>\n"
       << "\t-tec <n>\t\tStop tracing on cycle <n>\n"
       << "\t-tsi <n>\t\tStart tracing after <n> committed insts.\n"
       << "\t-tei <n>\t\tStop tracing after <n> committed insts.\n"
       << "\t-psc <n>\t\tStart profiling on cycle <n>\n"
       << "\t-pec <n>\t\tStop profiling on cycle <n>\n"
       << "\t-psi <n>\t\tStart profiling after <n> committed insts.\n"
       << "\t-pei <n>\t\tStop profiling after <n> committed insts.\n"
       << "\n"
       << "\t-p <n>\t\t\tTurn profiling on\n"
       << "\t-d <n>\t\t\tTurn warnings and debug information on\n"
       << "\n"
       << "\t-pc <n>\t\t\tIndicate progress info every <n> cycles\n"
       << "\t-pi <n>\t\t\tIndicate progress info every <n> committed insts.\n"
       << "\n"
       << "\t-param <name>=<value>\tdefine dynamic parameter <name> = <value>\n"
       << "\t-listparams\t\tlist all registered dynamic parameters\n"
       << "\n"
       << "\t-xcheck <file>\t\tGenerate output for cross-checking to file <file>\n"
       << endl;

    fputs (os.str().c_str(), file);
    SYS_Usage(file);
}
