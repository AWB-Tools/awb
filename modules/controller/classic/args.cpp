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
 * @author David Goodwin, Joel Emer & Carl Beckmann
 * @brief Asim argument parsing
 */

// generic
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/port.h"
#include "asim/mesg.h"
#include "asim/profile.h"
#include "asim/state.h"
#include "asim/registry.h"
#include "asim/trace.h"
#include "asim/ioformat.h"
#include "asim/xcheck.h"
#include "asim/param.h"
#include "asim/atoi.h"

// ASIM public modules
#include "asim/provides/controller.h"
// we need this so we can statically know the type of "theController"
// in the API functions declared using CONTROLLER_BASE_EXTERNAL_FUNCTION
#include "asim/provides/controller_alg.h"

//
// notice how we call all CONTROLLER_CLASS methods indirectly via theController.
// Although this is verbose, it allows us to get the effect of virtual functions
// (so that if CONTROLLER_CLASS is subclassed, we call the derived class methods)
// without the overhead of virtual functions (no virtual function pointers, and the
// compiler can resolve the function addresses statically).
//


//
// allocate and clean up argument lists here in the
// controller base class constructor and destructor
// (there used to be a janitor class here)
//
CONTROLLER_CLASS::CONTROLLER_CLASS() {
    awbArgv = NULL;
    sysArgv = NULL;
    fdArgv  = NULL;
    StatsFileName = NULL;
    pmStopped     = true;
    pmExiting     = false;
    pmInitialized = false;
}

CONTROLLER_CLASS::~CONTROLLER_CLASS() {
    if (awbArgv) {
        delete [] awbArgv;
    }
    if (sysArgv) {
        delete [] sysArgv;
    }
    if (fdArgv) {
        delete [] fdArgv;
    }
    if (StatsFileName) {
	delete StatsFileName;
    }
}

//
// Partition arguments into awb's, system's and feeder's.
//
void
CONTROLLER_CLASS::PartitionArgs (INT32 argc, char *argv[])
{
    origArgc = argc;
    origArgv = argv;

    awbArgc    = sysArgc    = fdArgc    = 0;
    awbArgcMax = sysArgcMax = fdArgcMax = argc;

    ASSERTX(awbArgv == NULL);
    awbArgv = (char **)malloc( (awbArgcMax + 1) * sizeof(char *) );
    ASSERTX(sysArgv == NULL);
    sysArgv = (char **)malloc( (sysArgcMax + 1) * sizeof(char *) );
    ASSERTX(fdArgv == NULL);
    fdArgv  = (char **)malloc( ( fdArgcMax + 1) * sizeof(char *) );

    //
    // Partition...
    
    feederArgs = false;
    systemArgs = false;
    for (INT32 i = 1; i < argc; i++) 
    {
        theController.PartitionOneArg( argc, argv, i );
    }
    awbArgv[awbArgc] = NULL;
    sysArgv[sysArgc] = NULL;
    fdArgv[fdArgc] = NULL;
}

// Partition arguments into awb's, system's and feeder's,
// one single loop iteration.
void
CONTROLLER_CLASS::PartitionOneArg (INT32 argc, char *argv[], INT32 &i)
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
            // if the command is -cfg <filename> then
	    // parse the file, which will recursively call this to handle each arg in the file...
	    if (strcmp(argv[i], "-cfg") == 0)
	    {
	    	theController.ParseConfigFile( argv[++i] );
	    }
	    else if (feederArgs) 
            {
                if (strcmp(argv[i], "-repeat") == 0)
                {
                    int repeatCount = atoi_general(argv[++i]);

                    // we need to allocate more space for fdArgv array
                    fdArgcMax += repeatCount;
                    fdArgv = (char **)realloc( fdArgv, (fdArgcMax + 1) * sizeof(char *) );
                    
                    for (int cnt = repeatCount; cnt > 0; cnt--)
                    {
                        fdArgv[fdArgc++] = fdArgv[fdArgc-1];
                    }
                }
                else
                {
                    fdArgv[fdArgc++] = argv[i];
                }
            }
            else if (systemArgs) 
            {
                sysArgv[sysArgc++] = argv[i];
            }
            else 
            {
                int incr = theController.ParseVariables(&argv[i], argc - i); 

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

// copy an argument value out of one string into another.
// this copies a sequence of non-white-space characters.
// If the string contains a quote, it copies anything in
// between quotes, but omits the quote characters.
// It returns a pointer to the newly allocated string,
// and advances the source string pointer.
static char *scan_arg_value( char * &source )
{
  const unsigned MAXWORD = 1024;
  char buf[MAXWORD], *s, *b;
  enum {normal, quote, esc, done} state;
  for ( s = source, b = buf, state = normal; *s && state != done; s++ ) {
    switch ( state ) {
      case normal:
        if      ( isspace( *s ) ) { *b++ = '\0'; state = done;   }
	else if ( *s == '"'     ) {              state = quote;  }
	else                      { *b++ = *s;                   }
	break;
      case quote:
        if      ( *s == '\\'    ) { *b++ = *s;   state = esc;    }
	else if ( *s == '"'     ) {              state = normal; }
	else                      { *b++ = *s;                   }
	break;
      case esc:                     *b++ = *s;   state = quote;
	break;
    }
  }
  unsigned len = b - buf;
  char *dest = (char *)malloc( len+1 );   // allocate storage for the string
  ASSERTX( dest );
  strncpy( dest, buf, len );              // copy the string including any opening and closing quotes
  source = s;                             // advance the source string pointer
  return dest;
}

//
// Handle a "-cfg <filename>" command line argument
// by parsing the arguments in the named file.
// This is called from PartitionOneArg(), and also calls PartitionOneArg()
// recursively to handle each argument in the file.
//
// It first reads in the entire file, and builds an argc,argv-like list
// of arguments contained in the file, then it processes them all in a
// loop similar to the one in PartitionArgs().
//
void
CONTROLLER_CLASS::ParseConfigFile( char *cfg_file_name )
{
    const INT32 MAXARGS = 4096;
    char *argv[MAXARGS];
    INT32 argc = 0;
    //
    // parse the input file and build and argv list
    //
    ifstream cfg_file( cfg_file_name );       // open the input file
    while ( cfg_file.good() ) {               // do for each line in the file:
        const INT32 MAXLINE = 256;
        char  *p, nextline[MAXLINE];
	cfg_file.getline( nextline, MAXLINE );
	for ( p = nextline; *p; ) {	        // parse each word on a line:
	    while ( *p && isspace( *p ) ) p++;    // skip leading white space
	    if ( *p == '#' ) break;               // if it's a comment, ignore rest of line
	    argv[argc++] = scan_arg_value( p );   // copy the next word from the input line
	    ASSERTX( argc < MAXARGS );
	}                                       // end parse each word
    }                                         // end for each line
    //
    // process the argv list
    //
    awbArgcMax += argc;                       // first allocate more storage
    sysArgcMax += argc;                       // in the argv arrays
    fdArgcMax  += argc;
    awbArgv = (char **)realloc( awbArgv, (awbArgcMax + 1) * sizeof(char *) );
    sysArgv = (char **)realloc( sysArgv, (sysArgcMax + 1) * sizeof(char *) );
    fdArgv  = (char **)realloc(  fdArgv, ( fdArgcMax + 1) * sizeof(char *) );
    for (int i = 0; i < argc; i++) {          // now process the args
        theController.PartitionOneArg( argc, argv, i );
    }
}

int
CONTROLLER_CLASS::parseTraceCmd(const char *progName, const char *command, string &regex, int &level) 
{
    if(*command != '/') 
    {
        // no regex given, match every name
        regex = ".*";
        level = 1;
        return(0);
    }

    regex = command;
    int pos = regex.size() - 1;
    // the last char should be '0', '1', or '2'
    if(regex[pos] != '0' && regex[pos] != '1' && regex[pos] != '2') 
    {
        // If a level was not given, defaul to 1.
        level = 1;
    } 
    else 
    {
        level = regex[pos] - '0';
        pos--;

        if(regex[pos] != '=') {
            Usage((char *)progName, stdout);
            cout << "\nExpected -tr [/regex/[=012]]" << endl;
            exit(-1);
        }
        pos--;
    }

    // remove the '/' at front and back
    if(regex[pos] != '/' || regex[0] != '/') 
    {
        Usage((char *)progName, stdout);
        cout << "\nExpected -tr [/regex/[=012]]" << endl;
        exit(-1);
    }
    regex.erase(pos);
    regex.erase(0,1);
    
    return(1);
}

// Parse command line arguments
// Return -1 if no argument is consumed, 0 if one argument consumed, 1 if two, 2 if three ...
int
CONTROLLER_CLASS::ParseVariables(char **argv, UINT32 argc)
{
    int incr = 0; 

    // -h           print usage
    if (strcmp(argv[0], "-h") == 0) 
    {
        Usage(argv[0], stdout);
        exit(0);
    }
    else if (strcmp(argv[0], "-t") == 0) 
    {
        ASSERT(BUILT_WITH_TRACE_FLAGS,"You are trying to generate trace in a "
              "model not compiled with tracing enabled. Build the model with TRACE=1");
        traceOn = true;
    }
    // -tr </regex/=[012]>		set trace regular expression (see trace.h)
    // FIXME: This argument is also parsed in ParseEvents
    // Temporal hack to make the ports' trace statements work
    // Ramon Matas-Navarro
    else if (strcmp(argv[0], "-tr") == 0)
    {
        string regex;
        int level;

        ASSERT(BUILT_WITH_TRACE_FLAGS,"You are trying to generate trace in a "
              "model not compiled with tracing enabled. Build the model with TRACE=1");

        if(argc > 1)
            incr += parseTraceCmd(argv[0], argv[1], regex, level);
        else
            incr += parseTraceCmd(argv[0], " ", regex, level);
        
        TRACEABLE_CLASS::EnableTraceByRegex(regex, level);
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
        traceMask = atoi_general(argv[++incr]); 
    }
    else if (strcmp(argv[0], "-tms") == 0) 
    {
        traceMask = find_trace_opt(argv[++incr]);
    }
    else if (strcmp(argv[0], "-listmasks") == 0) 
    {
        print_trace_masks(cout);
        exit(0);
    }
    else if (strcmp(argv[0], "-listnames") == 0)
    {
        printTraceNames = true;
    }
    else if (strcmp(argv[0], "-ppc") == 0) 
    {
        profilePc = atoi_general(argv[++incr]); 
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
        awbCmdFile = argv[++incr];
    }
    // -wb <w>      use workbench 'w' instead of built-in workbench
    //
    else if ((strcmp(argv[0], "-wb") == 0))
    {
        overrideWorkbench = argv[++incr];
    }
    // -s <f>
    else if ((strcmp(argv[0], "-s") == 0))
    {
        StatsFileName = new char[strlen(argv[incr+1])+1];  
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
                ASIMERROR("Don't know about dynamic parameter "
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

// Parse command line arguments, 
// return false if there is a parse error.
bool 
CONTROLLER_CLASS::ParseEvents(INT32 argc, char **argv)
{
    for (INT32 i = 0; i < argc; i++) 
    {
        // parse a single argument.  Call it via theController,
	// which might be a overridden version of the routine below.
        if ( ! theController.ParseOneEvent (argc, argv, i) ) {
	    return false;
	}
    }
    return true;
}

// Parse command line arguments, 
// return false if there is a parse error.
// Advance the argument pointer i, passed by reference.
bool
CONTROLLER_CLASS::ParseOneEvent (INT32 argc, char *argv[], INT32 &i)
{
        // check if tracing was enabled
        if(strncmp(argv[i], "-t", 2) == 0)
        {
            ASSERT(BUILT_WITH_TRACE_FLAGS,"You are trying to generate trace in a "
              "model not compiled with tracing enabled. Build the model with TRACE=1");
        }

        // -n <n>       run simulation for 'n' nanoseconds
        if ((strcmp(argv[i], "-n") == 0) && (argc > (i+1))) {
            theController.CMD_Exit(ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -c <n>       run simulation for 'n' cycles
        else if ((strcmp(argv[i], "-c") == 0) && (argc > (i+1))) {
            theController.CMD_Exit(ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -i <n>       run simulation for 'n' insts.
        else if ((strcmp(argv[i], "-i") == 0) && (argc > (i+1))) {
            theController.CMD_Exit(ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -m <n>       run simulation for 'n' insts.
        else if ((strcmp(argv[i], "-m") == 0) && (argc > (i+1))) {
            theController.CMD_Exit(ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -xcheck <file>		Activate Cross-check going to file <file> (see xcheck.h)
        else if ((strcmp(argv[i], "-xcheck") == 0) && (argc > (i+1))) 
        {
            ActivateCrossChecking(argv[++i]);
        }
        // -tr </regex/=[012]>		set trace regular expression (see trace.h)
        // This has to happen here instead of in ParseVariables so that the
        // -tr and -tr[cin] arguments are processed in the correct order.
        else if (strcmp(argv[i], "-tr") == 0)
        {
            string regex;
            int level;

            if(argc > (i+1))
                i += theController.parseTraceCmd(argv[i], argv[i + 1], regex, level);
            else
                theController.parseTraceCmd(argv[i], " ", regex, level);

            TRACEABLE_CLASS::EnableTraceByRegex(regex, level);
        }
        //--------------------------------------------------------------------
        // turning things on 'by instruction'
        //--------------------------------------------------------------------
        // -tsi <n>     turn on tracing after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-tsi") == 0) && (argc > (i+1))) {
            theController.CMD_Debug(true, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -tri <n>     Take all traces that were turned on by -tr
        //              and turn them on after <n> insts instead of now.
        else if (strcmp(argv[i], "-tri") == 0 && (argc > (i+1))) 
        {
            theController.CMD_Trace(new TRACEABLE_DELAYED_ACTION_CLASS(), 
                      ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -ti </regex/=[012]> <n>     turn on tracing after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-ti") == 0) && (argc > (i+1))) {
            string regex;
            int level;
            i += theController.parseTraceCmd(argv[0], argv[i + 1], regex, level);
            theController.CMD_Trace(new TRACEABLE_DELAYED_ACTION_CLASS(regex, level), 
                      ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -ssi <n>     turn on stats after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-ssi") == 0) && (argc > (i+1))) {
            theController.CMD_Stats(true, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -esi <n>     turn on events after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-esi") == 0) && (argc > (i+1))) {
            ASSERT(runWithEventsOn,"You are trying to generate events in a "
              "model not compiled with events. Build the model with EVENTS=1");
            theController.CMD_Events(true, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -csi <n>     turn on stripCharts after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-csi") == 0) && (argc > (i+1))) {
            theController.CMD_Stripchart(true, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -psi <n>     turn on profiling after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-psi") == 0) && (argc > (i+1))) {
            theController.CMD_Profile(true, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things off 'by instruction'
        //--------------------------------------------------------------------
        // -tei <n>     turn off tracing after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-tei") == 0) && (argc > (i+1))) {
            theController.CMD_Debug(false, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -sei <n>     turn off stats after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-sei") == 0) && (argc > (i+1))) {
            theController.CMD_Stats(false, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -eei <n>     turn off events after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-eei") == 0) && (argc > (i+1))) {
            theController.CMD_Events(false, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -cei <n>     turn off stripCharts after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-cei") == 0) && (argc > (i+1))) {
            theController.CMD_Stripchart(false, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -pei <n>     turn off profiling after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-pei") == 0) && (argc > (i+1))) {
            theController.CMD_Profile(false, ACTION_INST_ONCE, atoi_general(argv[++i]));
        }

        //--------------------------------------------------------------------
        // turning things on 'by macro instruction'
        //--------------------------------------------------------------------
        // -tsm <n>     turn on tracing after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-tsm") == 0) && (argc > (i+1))) {
            theController.CMD_Debug(true, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -trm <n>     Take all traces that were turned on by -tr
        //              and turn them on after <n> Macro insts instead of now.
        else if (strcmp(argv[i], "-trm") == 0 && (argc > (i+1))) 
        {
            theController.CMD_Trace(new TRACEABLE_DELAYED_ACTION_CLASS(), 
                      ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -tm </regex/=[012]> <n>     turn on tracing after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-tm") == 0) && (argc > (i+1))) {
            string regex;
            int level;
            i += theController.parseTraceCmd(argv[0], argv[i + 1], regex, level);
            theController.CMD_Trace(new TRACEABLE_DELAYED_ACTION_CLASS(regex, level), 
                      ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -ssm <n>     turn on stats after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-ssm") == 0) && (argc > (i+1))) {
            theController.CMD_Stats(true, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -esm <n>     turn on events after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-esm") == 0) && (argc > (i+1))) {
            ASSERT(runWithEventsOn,"You are trying to generate events in a "
              "model not compiled with events. Build the model with EVENTS=1");
            theController.CMD_Events(true, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -csm <n>     turn on stripCharts after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-csm") == 0) && (argc > (i+1))) {
            theController.CMD_Stripchart(true, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -psm <n>     turn on profiling after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-psm") == 0) && (argc > (i+1))) {
            theController.CMD_Profile(true, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things off 'by macro instruction'
        //--------------------------------------------------------------------
        // -tem <n>     turn off tracing after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-tem") == 0) && (argc > (i+1))) {
            theController.CMD_Debug(false, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -sem <n>     turn off stats after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-sem") == 0) && (argc > (i+1))) {
            theController.CMD_Stats(false, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -eem <n>     turn off events after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-eem") == 0) && (argc > (i+1))) {
            theController.CMD_Events(false, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -cem <n>     turn off stripCharts after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-cem") == 0) && (argc > (i+1))) {
            theController.CMD_Stripchart(false, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -pem <n>     turn off profiling after committing 'n' insts.
        //
        else if ((strcmp(argv[i], "-pem") == 0) && (argc > (i+1))) {
            theController.CMD_Profile(false, ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }


        //--------------------------------------------------------------------
        // turning things on 'by cycle'
        //--------------------------------------------------------------------
        // -tsc <n>     turn on tracing on cycle 'n'
        //
        else if ((strcmp(argv[i], "-tsc") == 0) && (argc > (i+1))) {
            theController.CMD_Debug(true, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -trc <n>     Take all traces that were turned on by -tr
        //              and turn them on at time <n> instead of now.
        else if (strcmp(argv[i], "-trc") == 0 && (argc > (i+1))) 
        {
            theController.CMD_Trace(new TRACEABLE_DELAYED_ACTION_CLASS(), 
                      ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -tc <n>     turn on tracing on cycle 'n'
        //
        else if ((strcmp(argv[i], "-tc") == 0) && (argc > (i+1))) {
            string regex;
            int level;
            i += theController.parseTraceCmd(argv[0], argv[i + 1], regex, level);
            theController.CMD_Trace(new TRACEABLE_DELAYED_ACTION_CLASS(regex, level), 
                      ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -ssc <n>     turn on stats on cycle 'n'
        //
        else if ((strcmp(argv[i], "-ssc") == 0) && (argc > (i+1))) {
            theController.CMD_Stats(true, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -e           turn events on on cycle 0
        else if ((strcmp(argv[i], "-e") == 0))
        {
            ASSERT(runWithEventsOn,"You are trying to generate events in a "
              "model not compiled with events. Build the model with EVENTS=1");
            theController.CMD_Events(true, ACTION_CYCLE_ONCE, 0);
            // That's the same as -esc 0
        }
        // -esc <n>     turn on events on cycle 'n'
        //
        else if ((strcmp(argv[i], "-esc") == 0) && (argc > (i+1))) {
            ASSERT(runWithEventsOn,"You are trying to generate events in a "
              "model not compiled with events. Build the model with EVENTS=1");
            theController.CMD_Events(true, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -csc <n>     turn on stripCharts on cycle 'n'
        //
        else if ((strcmp(argv[i], "-csc") == 0) && (argc > (i+1))) {
            theController.CMD_Stripchart(true, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -psc <n>     turn on profiling on cycle 'n'
        //
        else if ((strcmp(argv[i], "-psc") == 0) && (argc > (i+1))) {
            theController.CMD_Profile(true, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things off 'by cycle'
        //--------------------------------------------------------------------
        // -tec <n>     turn off tracing on cycle 'n'
        //
        else if ((strcmp(argv[i], "-tec") == 0) && (argc > (i+1))) 
        {
            theController.CMD_Debug(false, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -sec <n>     turn off stats on cycle 'n'
        //
        else if ((strcmp(argv[i], "-sec") == 0) && (argc > (i+1))) {
            theController.CMD_Stats(false, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -eec <n>     turn off events on cycle 'n'
        //
        else if ((strcmp(argv[i], "-eec") == 0) && (argc > (i+1))) {
            theController.CMD_Events(false, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -cec <n>     turn off stripCharts on cycle 'n'
        //
        else if ((strcmp(argv[i], "-cec") == 0) && (argc > (i+1))) {
            theController.CMD_Stripchart(false, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -pec <n>     turn off profiling on cycle 'n'
        //
        else if ((strcmp(argv[i], "-pec") == 0) && (argc > (i+1))) 
        {
            theController.CMD_Profile(false, ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things on 'by nanosecond'
        //--------------------------------------------------------------------
        // -tsn <n>     turn on tracing on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-tsn") == 0) && (argc > (i+1))) {
            theController.CMD_Debug(true, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -trn <n>     Take all traces that were turned on by -tr
        //              and turn them on at time <n> instead of now.
        else if (strcmp(argv[i], "-trn") == 0 && (argc > (i+1))) 
        {
            theController.CMD_Trace(new TRACEABLE_DELAYED_ACTION_CLASS(), 
                      ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -tn </regex/=[012]> <n>     turn on tracing on nanosecond 'n'.
        //
        else if ((strcmp(argv[i], "-tn") == 0) && (argc > (i+1))) {
            string regex;
            int level;
            i += theController.parseTraceCmd(argv[0], argv[i + 1], regex, level);
            theController.CMD_Trace(new TRACEABLE_DELAYED_ACTION_CLASS(regex, level), 
                      ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -ssn <n>     turn on stats on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-ssn") == 0) && (argc > (i+1))) {
            theController.CMD_Stats(true, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -esn <n>     turn on events on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-esn") == 0) && (argc > (i+1))) {
            ASSERT(runWithEventsOn,"You are trying to generate events in a "
              "model not compiled with events. Build the model with EVENTS=1");
            theController.CMD_Events(true, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -csn <n>     turn on stripCharts on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-csn") == 0) && (argc > (i+1))) {
            theController.CMD_Stripchart(true, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -psn <n>     turn on profiling on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-psn") == 0) && (argc > (i+1))) {
            theController.CMD_Profile(true, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // turning things off 'by nanosecond'
        //--------------------------------------------------------------------
        // -ten <n>     turn off tracing on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-ten") == 0) && (argc > (i+1))) 
        {
            theController.CMD_Debug(false, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -sen <n>     turn off stats on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-sen") == 0) && (argc > (i+1))) {
            theController.CMD_Stats(false, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -een <n>     turn off events on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-eec") == 0) && (argc > (i+1))) {
            theController.CMD_Events(false, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -cen <n>     turn off stripCharts on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-cec") == 0) && (argc > (i+1))) {
            theController.CMD_Stripchart(false, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -pen <n>     turn off profiling on nanosecond 'n'
        //
        else if ((strcmp(argv[i], "-pen") == 0) && (argc > (i+1))) 
        {
            theController.CMD_Profile(false, ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        //--------------------------------------------------------------------
        // -pc <n>       indicate progress every <n> cycles
        //
        else if ((strcmp(argv[i], "-pc") == 0) && (argc > (i+1))) 
        {
            theController.CMD_Progress(AWBPROG_CYCLE, "", ACTION_CYCLE_PERIOD,
                         atoi_general(argv[++i]));
        }
        // -pi <n>       indicate progress info every <n> committed insts.
        //
        else if ((strcmp(argv[i], "-pi") == 0) && (argc > (i+1))) 
        {
            theController.CMD_Progress(AWBPROG_INST, "", ACTION_INST_PERIOD,
                         atoi_general(argv[++i]));
        }
        // -pn <n>       indicate progress info every <n> nanoseconds.
        //
        else if ((strcmp(argv[i], "-pn") == 0) && (argc > (i+1))) 
        {
            theController.CMD_Progress(AWBPROG_NANOSECOND, "", ACTION_NANOSECOND_PERIOD,
                         atoi_general(argv[++i]));
        }
        // -sn <n>       emit stats file every <n> nanoseconds
        //
        else if ((strcmp(argv[i], "-sn") == 0) && (argc > (i+1))) 
        {
            theController.CMD_EmitStats(ACTION_NANOSECOND_PERIOD, atoi_general(argv[++i]));
        }
        // -sc <n>       emit stats file every <n> cycles
        //
        else if ((strcmp(argv[i], "-sc") == 0) && (argc > (i+1))) 
        {
            theController.CMD_EmitStats(ACTION_CYCLE_PERIOD, atoi_general(argv[++i]));
        }
        // -si <n>       emit stats every <n> committed insts.
        //
        else if ((strcmp(argv[i], "-si") == 0) && (argc > (i+1)))
        {
            theController.CMD_EmitStats(ACTION_INST_PERIOD, atoi_general(argv[++i]));
        }
        // -sm <n>       emit stats every <n> committed macro insts.
        //
        else if ((strcmp(argv[i], "-sm") == 0) && (argc > (i+1)))
        {
            theController.CMD_EmitStats(ACTION_MACROINST_PERIOD, atoi_general(argv[++i]));
        }
        // -rsc <n>       reset stat on cycle <n>
        //
        else if ((strcmp(argv[i], "-rsc") == 0) && (argc > (i+1))) 
        {
            theController.CMD_ResetStats(ACTION_CYCLE_ONCE, atoi_general(argv[++i]));
        }
        // -rsn <n>       reset stat on nanosecond <n>
        //
        else if ((strcmp(argv[i], "-rsn") == 0) && (argc > (i+1))) 
        {
            theController.CMD_ResetStats(ACTION_NANOSECOND_ONCE, atoi_general(argv[++i]));
        }
        // -rsi <n>       reset stat on instruction <n>
        //
        else if ((strcmp(argv[i], "-rsi") == 0) && (argc > (i+1))) 
        {
            theController.CMD_ResetStats(ACTION_INST_ONCE, atoi_general(argv[++i]));
        }
        // -rsm <n>       reset stat on macro instruction <n>
        //
        else if ((strcmp(argv[i], "-rsm") == 0) && (argc > (i+1))) 
        {
            theController.CMD_ResetStats(ACTION_MACROINST_ONCE, atoi_general(argv[++i]));
        }
        // -rscp <n>       reset stat every <n> cycles
        //
        else if ((strcmp(argv[i], "-rscp") == 0) && (argc > (i+1))) 
        {
            theController.CMD_ResetStats(ACTION_CYCLE_PERIOD, atoi_general(argv[++i]));
        }
        // -rsnc <n>       reset stat every <n> nanoseconds
        //
        else if ((strcmp(argv[i], "-rsnp") == 0) && (argc > (i+1))) 
        {
            theController.CMD_ResetStats(ACTION_NANOSECOND_PERIOD, atoi_general(argv[++i]));
        }
        // -rsip <n>       reset stat every <n> instructions
        //
        else if ((strcmp(argv[i], "-rsip") == 0) && (argc > (i+1))) 
        {
            theController.CMD_ResetStats(ACTION_INST_PERIOD, atoi_general(argv[++i]));
        }
        // -rsmp <n>       reset stat every <n> macro instructions
        //
        else if ((strcmp(argv[i], "-rsmp") == 0) && (argc > (i+1))) 
        {
            theController.CMD_ResetStats(ACTION_MACROINST_PERIOD, atoi_general(argv[++i]));
        }        
        else 
        {
            ASIMWARNING("Unknown flag, " << argv[i] << endl);
            return(false);
        }

    return(true);
}


void
CONTROLLER_CLASS::Usage (char *exec, FILE *file)
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
       << "\t-cfg <file>\t\tRead additional flags from configuration file <file>\n"
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
       << "\t\tSyntax: -[cepst][se][cinm] <n>\n"
       << "\t\t\t1st char:\n"
       << "\t\t\t\tc ... stripChart\n"
       << "\t\t\t\te ... Events\n"
       << "\t\t\t\tp ... Profiling\n"
       << "\t\t\t\ts ... Stats\n"
       << "\t\t\t\tt ... Tracing\n"
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
    theController.CMD_Usage(file);
}
