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
 * @brief Driver for "Architects Workbench", awb
 */

// generic
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

// ASIM core
#include "asim/port.h"
#include "asim/syntax.h"
#include "asim/atoi.h"
#include "asim/mesg.h"
#include "asim/state.h"
#include "asim/registry.h"
#include "asim/thread.h"
#include "asim/trace.h"
#include "asim/profile.h"
#include "asim/xcheck.h"
#include "asim/trackmem.h"
#include "asim/ioformat.h"
#include "asim/event.h"

// ASIM public modules
#include "asim/provides/workbench.h"
#include "asim/provides/controller.h"
#include "asim/provides/instfeeder_interface.h"

// ASIM local module
#include "awb.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

// USE_NON_CONST is to remove some incompatibility between tcl 8.3 and
// tcl 8.4 with respect to use of char ** vs. const char ** for
// command arguments;
#define USE_NON_CONST
extern "C" {
#include <tcl.h>
}
#undef USE_NON_CONST

#define AWB_TCLBUF_SZ     256

EXTERNC int PmStateCmd (ClientData clientData, Tcl_Interp *interp,int  argc, char *argv[]);
EXTERNC int PmEventCmd (ClientData clientData, Tcl_Interp *interp,int  argc, char *argv[]);
EXTERNC int PmStripCmd (ClientData clientData, Tcl_Interp *interp,int  argc, char *argv[]);
EXTERNC int PmTraceCmd (ClientData clientData, Tcl_Interp *interp,int  argc, char *argv[]);
EXTERNC int PmMarkerCmd (ClientData clientData, Tcl_Interp *interp,int  argc, char *argv[]);
EXTERNC int PmSymbolCmd (ClientData clientData, Tcl_Interp *interp,int  argc, char *argv[]);
EXTERNC int PmScheduleCmd (ClientData clientData, Tcl_Interp *interp,int  argc, char *argv[]);
EXTERNC int PmControlCmd (ClientData clientData, Tcl_Interp *interp,int  argc, char *argv[]);



static ASIM_STATELINK PmStateList (void);
static void AwbStartInterpreter (void);

static ASIM_STATE PmStateDescToPtr (char *desc);
static INT32 PmStateStates (Tcl_Interp *interp, char *path);
static INT32 PmStateFind (Tcl_Interp *interp, char *name);
static INT32 PmStateValue (Tcl_Interp *interp, char *desc);

static ASIM_THREAD PmThreadDescToPtr (char *desc);
static void PmTraceCmd(int argc, char *argv[]);

static bool DecodeActionTime (char *aStr, char *tStr, CMD_ACTIONTRIGGER *action, UINT64 *time);

bool statsOn = false; 
extern bool stripsOn; 

// The tcl interpreter running the awb workbench
static Tcl_Interp *awbInterp = NULL;

/*
 * File holding commands for awb to execute after initializing.
 */
char *awbCmdFile = "";
 
/*
 * If non-null, this is the full pathname of a workbench to
 * use in place of the default workbench.
 */
char *overrideWorkbench = NULL;

/*
 * Pending progress events
 */
static UINT32 pendingProgressSz = 0;
static char *pendingProgress = NULL;

/*
 * Dump file for events
 */
char* event_dumpfilename = "";
FILE* event_dumpfile = NULL;

/*
 * Class for tracing.
 */
class AWB_TRACER : public TRACEABLE_CLASS
{
public:
    virtual ~AWB_TRACER() {}
    virtual std::string GetTraceableName() 
    {
        return("Trace_Command");
    }
};
AWB_TRACER awbTracer;

void
AWB_Initialize (void)
{
    pendingProgressSz = 1024;
    pendingProgress = Tcl_Alloc(pendingProgressSz);
    pendingProgress[0] = '\0';
    // Initialize events structure. Must be done before creatiung any bow or buffer.
    EVENT(ASIM_DRAL_EVENT_CLASS::InitEvent());
}

void
AWB_Activate (void)
/*
 * Activate awb if it is not already running.
 */
{
    // Start command interpreter
    AwbStartInterpreter();
}




void
AWB_Progress (AWB_PROGRESSTYPE type, const char *args)
/*
 * Exit awb...
 */
{
    static char *pStrs[] = AWB_PROGRESSSTRS;
    
    //
    // Add progress to the list of pending progress events.

    const UINT32 newSz = strlen(pendingProgress) + strlen(pStrs[type]) + strlen(args) + 10;
    if (newSz >= pendingProgressSz) {
        pendingProgressSz = MAX(pendingProgressSz * 2, newSz);
        pendingProgress = Tcl_Realloc(pendingProgress, pendingProgressSz);
    }

    if (pendingProgress[0] != '\0')
        strcat(pendingProgress, " ");
    strcat(pendingProgress, pStrs[type]);

    if (strlen(args) == 0) {
        strcat(pendingProgress, " {}");
    }
    else {
        strcat(pendingProgress, " ");
        strcat(pendingProgress, args);
    }
}    


void
AWB_InformProgress (void)
/*
 * Notify the workbench of the progress events that are pending,
 * and wait for it to acknowledge that it has processed all the
 * events.
 */
{
    if (pendingProgress[0] != '\0') {
        T1_AS(&awbTracer, "TCL_CMD: "
              << "BatchProgress { "
              << pendingProgress
              << " }");
    
        INT32 eval = Tcl_VarEval(awbInterp, "BatchProgress { ", pendingProgress, " }", NULL);

        if ( eval != TCL_OK) {
            ASIMERROR("AWB_InformProgress: "
                << Tcl_GetStringResult(awbInterp) << endl); 
        }
        
        pendingProgress[0] = '\0';
    }

    VERIFYX(pendingProgress[0] == '\0');
}

void
AWB_Exit (void)
/*
 * Exit awb...
 */
{
    //
    // Send awb a progress message ordering it to exit, then wait
    // for awb to signal that it has exited.

    AWB_Progress(AWBPROG_EXIT);
    AWB_InformProgress();

    // gracefully shut down TCL - let it free its memory
    if (pendingProgress) {
        Tcl_Free(pendingProgress);
        pendingProgress = NULL;
    }
    Tcl_DeleteInterp(awbInterp);
    awbInterp = NULL;
    Tcl_Finalize();
}



static void
AwbStartInterpreter (void)
/*
 * Start point for thread responding to event in awb.
 */
{
    //
    // Create the tcl interpreter

    awbInterp = Tcl_CreateInterp();

    //
    // Initialize tcl

    Tcl_Preserve(awbInterp);

    if (Tcl_Init(awbInterp) != TCL_OK)
    {
        ASIMERROR("AwbStartInterpreter: Tcl_Init "
            << Tcl_GetStringResult(awbInterp) << endl);
    }
        
    //
    // Register the commands we provide to awb
    
    Tcl_CreateCommand(awbInterp, "PmState", PmStateCmd, NULL, NULL);
    Tcl_CreateCommand(awbInterp, "PmEvent", PmEventCmd, NULL, NULL);
    Tcl_CreateCommand(awbInterp, "PmStrip", PmStripCmd, NULL, NULL);
    Tcl_CreateCommand(awbInterp, "PmTrace", PmTraceCmd, NULL, NULL);
    Tcl_CreateCommand(awbInterp, "PmMarker", PmMarkerCmd, NULL, NULL);
    Tcl_CreateCommand(awbInterp, "PmSymbol", PmSymbolCmd, NULL, NULL);
    Tcl_CreateCommand(awbInterp, "PmSchedule", PmScheduleCmd, NULL, NULL);
    Tcl_CreateCommand(awbInterp, "PmControl", PmControlCmd, NULL, NULL);
    
    //
    // Load script for the default workbench configured for this
    // model. After loading the script, we can read "awbNamespace" to get
    // the workbench's namespace and then import the init procedure.

    if (overrideWorkbench == NULL) {
        char *awbWorkbench = AWB_WbInit();
        if (Tcl_GlobalEval(awbInterp, awbWorkbench) != TCL_OK) {
            ASIMERROR("AwbStartInterpreter: Tcl_GlobalEval for awbWorkbench: "
                << Tcl_GetStringResult(awbInterp) << endl
                << "Command:" << endl << awbWorkbench); 
        }
        delete[] awbWorkbench;
    }
    else {
        if (Tcl_EvalFile(awbInterp, overrideWorkbench) != TCL_OK) {
            ASIMERROR("AwbStartInterpreter: Tcl_EvalFile "
                << Tcl_GetStringResult(awbInterp)); 
        }
    }

    char *ns = Tcl_GetVar(awbInterp, "AwbNamespace", TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);
    if (ns == NULL) {
        ASIMERROR("AwbStartInterpreter: Tcl_GetVar "
            << Tcl_GetStringResult(awbInterp)); 
    }

    // localize temps
    {
        ostringstream os;
        char * buf;

        os << "namespace import "
           << ns << "::AwbInit "
           << ns << "::BatchProgress";
        buf = strdup(os.str().c_str());
        if (Tcl_GlobalEval(awbInterp, buf) != TCL_OK) {
            ASIMERROR("AwbStartInterpreter: Tcl_GlobalEval for namespace import: "
                << Tcl_GetStringResult(awbInterp)); 
        }
        free(buf);

        //
        // Initialize the workbench telling it to use the root window "."

        os.str(""); // clear
        os << "AwbInit . " << "batch" << " {" << awbCmdFile << "}";
        buf = strdup(os.str().c_str());
        if (Tcl_GlobalEval(awbInterp, buf) != TCL_OK) {
            ASIMERROR("AwbStartInterpreter: Tcl_GlobalEval for AwbInit: "
                << Tcl_GetStringResult(awbInterp)); 
        }
        free(buf);
    }

    Tcl_Release(awbInterp);
}


/******************************************************************
 *
 * Tcl commands
 *
 *****************************************************************/
 

static ASIM_STATE
PmStateDescToPtr (char *desc)
/*
 * Return pointer to the ASIM_STATE object specified by 'desc'.
 * Return NULL on error.
 */
{
    ASIM_STATE state = NULL;
    if (sscanf(desc, "STATE__%p", &state) != 1)
        return(NULL);

    return(state);
}


static ASIM_STATELINK
PmStateList (void)
/*
 * Return the list of exposed performance model state.
 */
{
    class StateListKeeper {
      public:
        ASIM_STATELINK pmState;
        // constructor / destructor
        StateListKeeper() { pmState = NULL; }
        ~StateListKeeper() {
            // delete memory we own
            while (pmState) {
                ASIM_STATELINK nextState = pmState->next;
                delete pmState;
                pmState = nextState;
            }
        }
    };

    static StateListKeeper stateListKeeper;

    if (stateListKeeper.pmState == NULL) {
        stateListKeeper.pmState = CMD_StateList();
    }

    return(stateListKeeper.pmState);
}

int
PmStateCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
/*
 * tcl interface to pm state. Return's a list of available state, or
 * specific state attributes.
 */
{
    ostringstream os;

    if (argc < 2) {
        Tcl_SetResult(interp, "Usage: PmState option ?arg ...?", TCL_STATIC);
        return(TCL_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    char *option = argv[1];

    //
    // states     - optional path argument
    //              returns list of state descriptors for that path
    //
    if (strcmp(option, "states") == 0) {
        if ((argc != 2) && (argc != 3)) {
            Tcl_SetResult(interp, "Usage: PmState states ?path?", TCL_STATIC);
            return(TCL_ERROR);
        }

        return(PmStateStates(interp, (char *)((argc == 2) ? (char *)(NULL) : argv[2])));
    }
    //
    // find    - requires name argument
    //           returns state descriptor for specified name or empty
    //           string if can find named state
    //
    else if (strcmp(option, "find") == 0) {
        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: PmState find ?name?", TCL_STATIC);
            return(TCL_ERROR);
        }

        return(PmStateFind(interp, argv[2]));
    }
    //
    // dump    - requires filename argument
    //           dump statistics to file
    //
    else if (strcmp(option, "dump") == 0) {
        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: PmState dump ?filename?", TCL_STATIC);
            return(TCL_ERROR);
        }

        //
        // Before we print the states, we must make sure that each variable's
        // current value is updated to reflect the unsuspended value (see
        // comments in "Update" for more info...)

        ASIM_STATELINK scan = PmStateList();
        while (scan != NULL) {
            scan->state->Update();
            scan = scan->next;
        }
        
        //
        // Dump stats

        char *filename = argv[2];

        STATE_OUT stateOut = new STATE_OUT_CLASS(filename);
        
        if (! stateOut) 
        {
            ASIMERROR("Unable to create stats output file \"" <<filename
                      << "\", " << strerror(errno));
        }
        //output the module stats and the feeder stats to the stateout 
	asimSystem->PrintModuleStats(stateOut); 
        IFEEDER_BASE_CLASS::DumpAllFeederStats(stateOut);
        if(stateOut)
	{
           // dump stats to file and delete object
           delete stateOut;
        }
        return(TCL_OK);
    }
    //
    // suspend   - requires state descriptor argument
    //             suspends the specified state
    //
    else if (strcmp(option, "suspend") == 0) {
        if (argc == 2) {
            // suspend all
            statsOn = false; 
            for (ASIM_STATELINK scan = PmStateList();
                 scan != NULL;
                 scan = scan->next)
            {
                scan->state->Suspend();
            }
            return(TCL_OK);
        }

        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: PmState suspend ?state?", TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            statsOn = false; 
            state->Suspend();
            return(TCL_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'suspend'";
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_ERROR);
    }
    //
    // unsuspend   - requires state descriptor argument
    //               unsuspends the specified state
    //
    else if (strcmp(option, "unsuspend") == 0) {
        if (argc == 2) {
            // unsuspend all
            statsOn = true; 
            for (ASIM_STATELINK scan = PmStateList();
                 scan != NULL;
                 scan = scan->next)
            {
                scan->state->Unsuspend();
            }
            return(TCL_OK);
        }

        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: PmState unsuspend ?state?",
                          TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            statsOn = true; 
            state->Unsuspend();
            return(TCL_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'unsuspend'";
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_ERROR);
    }
    //
    // value   - requires state descriptor argument
    //           returns state's value, as a list if state is an array
    //
    else if (strcmp(option, "value") == 0) {
        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: PmState value ?state?", TCL_STATIC);
            return(TCL_ERROR);
        }

        return(PmStateValue(interp, argv[2]));
    }
    //
    // path    - requires state descriptor argument
    //           returns path of corresponding state
    //
    else if (strcmp(option, "path") == 0) {
        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: PmState path ?state?", TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            Tcl_SetResult(interp, (char *)state->Path(), TCL_STATIC);
            return(TCL_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'path'";
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_ERROR);
    }
    //
    // name    - requires state descriptor argument
    //           returns name of corresponding state
    //
    else if (strcmp(option, "name") == 0) {
        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: PmState name ?state?", TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            Tcl_SetResult(interp, (char *)state->Name(), TCL_STATIC);
            return(TCL_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'name'";
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_ERROR);
    }
    //
    // desc    - requires state descriptor argument
    //           returns description of corresponding state
    //
    else if (strcmp(option, "desc") == 0) {
        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: PmState desc ?state?", TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            Tcl_SetResult(interp, (char *)state->Description(), TCL_STATIC);
            return(TCL_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'desc'";
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_ERROR);
    }
    
    //
    // unknown option
    //
    os.str(""); // clear
    os << "unknown option \"" << option << "\": must be states, find, dump,"
       << " suspend, unsuspend, value, path, name, or desc";
    
    Tcl_SetResult(interp,
        const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
    return(TCL_ERROR);
}        
    

static INT32
PmStateStates (Tcl_Interp *interp, char *path)
/*
 * Search the state list to find states whose path's start with 'path'. We
 * do a case-insensitive search. If 'path' == NULL then return a list of
 * all state. Return a list of descriptors listing the matching states.
 */
{
    char lastPath[AWB_TCLBUF_SZ+10];
    lastPath[0] = '\0';

    ASIM_STATELINK scan = PmStateList();
    while (scan != NULL) {
        ASIM_STATE state = scan->state;
        ostringstream os;

        //
        // If 'path' is a prefix of 'state's path, then it can either
        // be equal to 'state's path or just part of it.
        
        if ((path == NULL) || (strncasecmp(state->Path(), path, strlen(path)) == 0)) {
            //
            // If 'path' equals 'state's path, then return
            // 'state's descriptor.
            
            if ((path == NULL) || (strcasecmp(state->Path(), path) == 0)) {
                os.str(""); // clear
                os << "STATE__" << fmt_p((void *)state);
                Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
            }
            else {
                //
                // construct the next longest path prefix for 'state'.

                char prefix[AWB_TCLBUF_SZ+10];
                char *nel = strchr(state->Path()+strlen(path)+1, '/');
                if (nel == NULL) {
                    ASSERTX(strlen(state->Path()) < AWB_TCLBUF_SZ);
                    strcpy(prefix, state->Path());
                }
                else {
                    ASSERTX((nel - state->Path() + 1) < AWB_TCLBUF_SZ);

                    strncpy(prefix, state->Path(), nel - state->Path());
                    prefix[nel-state->Path()] = '\0';
                }
                
                //
                // If the new longer prefix is different than the
                // last prefix we saw, then return it.

                if (strcasecmp(lastPath, prefix) != 0) {
                    strcpy(lastPath, prefix);
                    os.str(""); // clear
                    os << "MODULE__" << prefix; 
                    Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
                }
            }
        }
        
        scan = scan->next;
    }

    return(TCL_OK);
}


static INT32
PmStateFind (Tcl_Interp *interp, char *name)
/*
 * Search the state list to find the state named 'name.  We do a
 * case-insensitive search. Return a descriptor for the matching state
 * or "" if can't find.
 */
{
    ASIM_STATELINK scan = PmStateList();
    while (scan != NULL) {
        ASIM_STATE state = scan->state;

        if (strcasecmp(state->Name(), name) == 0) {
            ostringstream os;
            os << "STATE__" << fmt_p((void *)state);
            Tcl_SetResult(interp,
                const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
            return(TCL_OK);
        }

        scan = scan->next;
    }

    Tcl_SetResult(interp, "", TCL_STATIC);
    return(TCL_OK);
}


static INT32
PmStateValue (Tcl_Interp *interp, char *desc)
/*
 * Return 'state's value.
 */
{
    ostringstream os;

    ASIM_STATE state = PmStateDescToPtr(desc);
    if (state != NULL) {
        //
        // Iterate through 'state's values, appending them
        // to the result list.

        switch (state->Type()) {
          case STATE_UINT:
            for (UINT32 i = 0; i < state->Size(); i++) {
              os.str(""); // clear
              os << state->IntValue(i);
              Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
            }
            break;
          case STATE_FP:
            for (UINT32 i = 0; i < state->Size(); i++) {
              os.str(""); // clear
              os << fmt("f", state->FpValue(i));
              Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
            }
          case STATE_HISTOGRAM:
            os.str(""); // clear
            os << "state type STATE_HISTOGRAM not supported";
            Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
            break;
          case STATE_RESOURCE:
            os.str(""); // clear
            os << "state type STATE_RESOURCE not supported";
            Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
            break;
          case STATE_THREE_DIM_HISTOGRAM:
            os.str(""); // clear
            os << "state type STATE_THREE_DIM_HISTOGRAM not supported";
            Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
            break;
          default:
            os.str(""); // clear
            os << "unknown state type " << state->Type();
            Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
        }

        return(TCL_OK);
    }

    os.str(""); // clear
    os << "null state descriptor for 'value' \"" << desc << "\"";
    Tcl_SetResult(interp,
        const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
    return(TCL_ERROR);
}

int
PmEventCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    char *option = argv[1];
    if (!runWithEventsOn)
    {
        Tcl_SetResult(
            interp, "You are trying to generate events in a model not compiled "
            "with events. Build the model with EVENTS=1", TCL_STATIC);
        return(TCL_ERROR);
    }
    if (strcmp(option, "on") == 0) {
        DRALEVENT(TurnOn());
        if (firstTurnedOn)
        {
            firstTurnedOn=false;
            DRALEVENT(StartActivity(ASIM_CLOCKABLE_CLASS::GetClockServer()->getFirstDomainCycle()));
        }
        ASIM_CLOCKABLE_CLASS::GetClockServer()->DralTurnOn();
        eventsOn = true;
    }
    else if (strcmp(option, "off") == 0) {
        DRALEVENT(TurnOff());
        eventsOn = false;
    }
    else if (strcmp(option, "filename") == 0) {
        if (argc != 3) {
            Tcl_SetResult(interp, "Usage: AwbEvents filename ?filename?", TCL_STATIC);
            return(TCL_ERROR);
        }
        /*
        if (eventsOn)
        {
            Tcl_SetResult(
                interp, "The events file name can only be changed "
                "if the dral server is turned off", TCL_STATIC);
            return(TCL_ERROR);
        }
        */
        DRALEVENT(ChangeFileName(argv[2]));
    }
    return(TCL_OK);
}    


int
PmStripCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    char *option = argv[1];
    if (strcmp(option, "on") == 0) {
        stripsOn = true;
    }
    else {
        stripsOn = false;
    }
    return(TCL_OK);
}    

int
PmTraceCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    // TBD snpeffer
    /*char *option = argv[1];
    if (strcmp(option, "on") == 0) {
        traceOn = true;
    }
    else {
        traceOn = false;
        }*/
    return(TCL_OK);
}    

//----------------------------------------------------------------------------

int
PmMarkerCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
/*
 * tcl interface to pm marker. Set / Clear markers
 */
{
    ostringstream os;

    if (argc < 2) {
        Tcl_SetResult(interp, "Usage: PmMarker option ?arg ...?", TCL_STATIC);
        return(TCL_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    char *option = argv[1];

    //
    // set        - requires thread, markerID, subcommand
    //              subcommand: pc requires address
    //              subcommand: inst requires bits, mask
    // syntax: PmMarker set <thread> <id> {("pc" <addr>) | "inst" <bits> <mask>}
    //
    if (strcmp(option, "set") == 0) {
        if ((argc < 5)) {
            Tcl_SetResult(interp, "Usage: PmMarker set ?thread? ?id? ?subcmd?", TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
        INT32 markerID = atoi_general(argv[3]);
        char *subcmd = argv[4];

        // subcommand parsing
        //
        // subcommand: pc <addr>
        if (strcmp(subcmd, "pc") == 0) {
            if ((argc != 6)) {
                Tcl_SetResult(interp, "Usage: PmMarker set ?thread? ?id? pc ?addr?", TCL_STATIC);
                return(TCL_ERROR);
            }
            UINT64 addr = atoi_general_unsigned(argv[5]);
            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_SET_PC,
                                      thread->IStreamHandle(),
                                      markerID,
                                      addr);
            return(TCL_OK);
        }
        //
        // subcommand: inst
        else if (strcmp(subcmd, "inst") == 0) {
            if ((argc != 7)) {
                Tcl_SetResult(interp, "Usage: PmMarker set ?thread? ?id? inst ?bits? ?mask?", TCL_STATIC);
                return(TCL_ERROR);
            }

            UINT32 bits = atoi_general_unsigned(argv[5]);
            UINT32 mask = atoi_general_unsigned(argv[6]);

            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_SET_INST,
                                      thread->IStreamHandle(),
                                      markerID,
                                      /*pc*/0,
                                      bits,
                                      mask);
            return(TCL_OK);
        }
        //
        // unknown subcommand
        os.str(""); // clear
        os << "PmMarker set: unknown subcommand \"" << subcmd << "\""
           << ": must be pc, or inst";
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_ERROR);
    }
    //
    // clear      - requires thread, markerID, subcommand
    //              subcommand: pc requires address
    //              subcommand: all
    // syntax: PmMarker clear <thread> <id> {("pc" <addr>) | "all"}
    //
    else if (strcmp(option, "clear") == 0) {
        if ((argc < 5)) {
            Tcl_SetResult(interp, "Usage: PmMarker clear ?thread? ?id? ?subcmd?", TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
        INT32 markerID = atoi_general(argv[3]);
        char *subcmd = argv[4];

        // subcommand parsing
        //
        // subcommand: pc <addr>
        if (strcmp(subcmd, "pc") == 0) {
            if ((argc != 6)) {
                Tcl_SetResult(interp, "Usage: PmMarker clear ?thread? ?id? pc ?addr?", TCL_STATIC);
                return(TCL_ERROR);
            }

            UINT64 addr = atoi_general_unsigned(argv[5]);
            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_CLEAR_PC,
                                      thread->IStreamHandle(),
                                      markerID,
                                      addr);
            return(TCL_OK);
        }
        //
        // subcommand: all
        else if (strcmp(subcmd, "all") == 0) {
            if ((argc != 5)) {
                Tcl_SetResult(interp, "Usage: PmMarker clear ?thread? ?id? all", TCL_STATIC);
                return(TCL_ERROR);
            }

            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_CLEAR_ALL,
                                      thread->IStreamHandle(),
                                      markerID);
            return(TCL_OK);
        }
        //
        // unknown subcommand
        os.str(""); // clear
        os << "PmMarker clear: unknown subcommand \"" << subcmd << "\""
           << ": must be pc, or all";
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_ERROR);
    }
    
    //
    // unknown option
    //
    os.str(""); // clear
    os << "PmMarker: unknown option \"" << option << "\""
       << ": must be set, or clear";
    Tcl_SetResult(interp,
        const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
    return(TCL_ERROR);
}        
    
//----------------------------------------------------------------------------

int
PmSymbolCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
/*
 * tcl interface to pm symbol table; query symbol addresses
 */
{
    ostringstream os;

    if (argc < 2) {
        Tcl_SetResult(interp, "Usage: PmSymbol option ?arg ...?", TCL_STATIC);
        return(TCL_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    char *option = argv[1];

    //
    // find       - requires thread, symbol name
    //
    // syntax: PmSmbol find <thread> <name>
    //
    if (strcmp(option, "find") == 0) {
        if ((argc != 4)) {
            Tcl_SetResult(interp, "Usage: PmSymbol find ?thread? ?name?", TCL_STATIC);
            return(TCL_ERROR);
        }

        UINT64 addr;
        char *symname = argv[3];
        if (strncmp(argv[2], "THREAD__", strlen("THREAD__")) == 0) {
            ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
            addr = thread->IFeeder()->Symbol(thread->IStreamHandle(), symname);
        } else {
            UINT32 tid = atoi_general_unsigned(argv[2]);
            ASSERT(tid == 0, "Only 0 thread-id supported in PmSymbol");
            addr = IFEEDER_BASE_CLASS::SingleFeederSymbolHack(symname);
        }
        if (addr == (UINT64) NULL) {
            os.str(""); // clear
            os << "PmSymbol: unknown symbol \"" << symname << "\"";
            Tcl_SetResult(interp,
                const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
            return(TCL_ERROR);
        } else {
            os.str(""); // clear
            os << "0x" << fmt_x(addr);
            Tcl_SetResult(interp,
                const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
            return(TCL_OK);
        }

    }
    
    //
    // unknown option
    //
    os.str(""); // clear
    os << "PmSymbol: unknown option \"" << option << "\": must be find";
    Tcl_SetResult(interp,
        const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
    return(TCL_ERROR);
}        
    
//----------------------------------------------------------------------------

int
PmScheduleCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
/*
 * tcl interface to pm scheduler. Examine, add, or delete from schedule.
 */
{
    ostringstream os;
    bool timeError = false;
    
    if (argc < 2) {
        Tcl_SetResult(interp, "Usage: PmSchedule option ?arg ...?", TCL_STATIC);
        return(TCL_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    char *option = argv[1];

    //
    // progress     - 'inst'/'macroinst'/'cycle'/'nanosecond' and 'period'/'clear'
    //
    if (strcmp(option, "progress") == 0) {
        if (argc != 4) {
            Tcl_SetResult(interp, "Usage:\tPmSchedule progress ?inst|cycle|nanosecond? ?clear|period?\n",
                          TCL_STATIC);
            return(TCL_ERROR);
        }

        char *period = argv[3];
        CMD_ACTIONTRIGGER action;
        if (strcmp(argv[2], "inst") == 0)
            action = ACTION_INST_PERIOD;
        else if (strcmp(argv[2], "macroinst") == 0)
            action = ACTION_MACROINST_PERIOD;
        else if (strcmp(argv[2], "cycle") == 0)
            action = ACTION_CYCLE_PERIOD;
        else if (strcmp(argv[2], "nanosecond") == 0)
            action = ACTION_NANOSECOND_PERIOD;
        else
        {
            os.str(""); // clear
            os << "invalid action option \"" << argv[2] << "\""
               << ": must be inst, cycle or nanosecond";
            Tcl_SetResult(interp,
                const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
            return(TCL_ERROR);
        }         
        
        if (strcmp(period, "clear") == 0)
        {
            if (action == ACTION_CYCLE_PERIOD)
                CMD_Progress(AWBPROG_CLEARCYCLE, "", ACTION_NOW);
            else if (action == ACTION_MACROINST_PERIOD)
                CMD_Progress(AWBPROG_CLEARMACROINST, "", ACTION_NOW);
            else if (action == ACTION_NANOSECOND_PERIOD)
                CMD_Progress(AWBPROG_CLEARNANOSECOND, "", ACTION_NOW);
            else
                CMD_Progress(AWBPROG_CLEARINST, "", ACTION_NOW);
        }
        else if (action == ACTION_CYCLE_PERIOD)
        {
            CMD_Progress(AWBPROG_CYCLE, "", action, atoi_general(period));
        }
        else if (action == ACTION_NANOSECOND_PERIOD)
        {
            CMD_Progress(AWBPROG_NANOSECOND, "", action, atoi_general(period));
        }
        else if (action == ACTION_MACROINST_PERIOD)
        {
            CMD_Progress(AWBPROG_MACROINST, "", action, atoi_general(period));
        }
        else
        {
            CMD_Progress(AWBPROG_INST, "", action, atoi_general(period));
        }

        return(TCL_OK);
    }
    //
    // start   - start performance model
    //
    else if (strcmp(option, "start") == 0) {
        if (argc != 2) {
            Tcl_SetResult(interp, "Usage: PmSchedule start", TCL_STATIC);
            return(TCL_ERROR);
        }

        //
        // It only makes sense to have a start action for "now". A start
        // action for a future time is meaningless if the model is already
        // running, and if the model is not running, then the future time
        // will never be reached.

        CMD_Start();
        return(TCL_OK);
    }
    //
    // stop     - requires action time
    //            stop the performance model
    //
    else if (strcmp(option, "stop") == 0) {
        if ((argc != 3) && (argc != 4)) {
            Tcl_SetResult(interp, "Usage: PmSchedule stop ?action? ?time?", TCL_STATIC);
            return(TCL_ERROR);
        }

        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[2], (char *)((argc == 4) ? argv[3] : 
			     (char *)(NULL)), &action, &time)) {
            CMD_Stop(action, time);
            return(TCL_OK);
        }

        timeError = true;
    }
    //
    // exit     - requires action time
    //            exit the performance model
    //
    else if (strcmp(option, "exit") == 0) {
        if ((argc != 3) && (argc != 4)) {
            Tcl_SetResult(interp, "Usage: PmSchedule exit ?action? ?time?", TCL_STATIC);
            return(TCL_ERROR);
        }

        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[2], (char *)((argc == 4) ? argv[3] : (char *)(NULL)), &action, &time)) {
            CMD_Exit(action, time);
            return(TCL_OK);
        }

        timeError = true;
    }
    //
    // schedthread     - requires action, time, thread
    //                   schedule thread on performance model
    //
    else if (strcmp(option, "schedthread") == 0) {
        if ((argc != 4) && (argc != 5)) {
            Tcl_SetResult(interp, "Usage: PmSchedule schedthread ?thread? ?action? ?time?", TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
        if (thread == NULL) {
            os.str(""); // clear
            os << "invalid thread descriptor \"" << argv[2] << "\"";
            Tcl_SetResult(interp,
                const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
            return(TCL_ERROR);
        }
        
        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[3], (char *)((argc == 5) ? argv[4] : (char *)(NULL)), &action, &time)) {
            CMD_ScheduleThread(thread, action, time);
            return(TCL_OK);
        }

        timeError = true;
    }
    //
    // unschedthread     - requires action, time, thread
    //                     unschedule thread on performance model
    //
    else if (strcmp(option, "unschedthread") == 0) {
        if ((argc != 4) && (argc != 5)) {
            Tcl_SetResult(interp, "Usage: PmSchedule unschedthread ?thread? ?action? ?time?", TCL_STATIC);
            return(TCL_ERROR);
        }

        ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
        if (thread == NULL) {
            os.str(""); // clear
            os << "invalid thread descriptor \"" << argv[2] << "\"";
            Tcl_SetResult(interp,
                const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
            return(TCL_ERROR);
        }
        
        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[3], (char *)((argc == 5) ? argv[4] : (char *)NULL), &action, &time)) {
            CMD_UnscheduleThread(thread, action, time);
            return(TCL_OK);
        }

        timeError = true;
    }
    //
    // skipthread     - requires action, time, thread
    //                  skip thread
    //
    else if (strcmp(option, "skipthread") == 0) {
        if ((argc != 6) && (argc != 7)) {
            Tcl_SetResult(interp, "Usage: PmSchedule skipthread ?thread? ?insts? ?markerID? ?action? ?time?", TCL_STATIC);
            return(TCL_ERROR);
        }

        //
        // If ?thread? is "all", then set thread to NULL to indicate that all
        // threads should be skipped.
        
        ASIM_THREAD thread = NULL;
        if (strcasecmp(argv[2], "all") != 0) {
            thread = PmThreadDescToPtr(argv[2]);
            if (thread == NULL) {
                os.str(""); // clear
                os << "invalid thread descriptor \"" << argv[2] << "\"";
                Tcl_SetResult(interp,
                    const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
                return(TCL_ERROR);
            }
        }

        UINT64 insts = atoi_general_unsigned(argv[3]);
        INT32 markerID = atoi_general(argv[4]);
        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[5], (char *)((argc == 7) ? argv[6] : (char *)(NULL)), &action, &time)) {
            CMD_SkipThread(thread, insts, markerID, action, time);
            return(TCL_OK);
        }

        timeError = true;
    }

    //
    // unknown option or time format error
    //
    if (timeError) {
        os.str(""); // clear
        os << "invalid time option \"" << argv[2] << "\": must be now,"
           << " cycle_once, cycle period, inst_once, or inst_period";
    }
    else {
        os.str(""); // clear
        os << "unknown option \"" << option << "\": must be progress, exit,"
           << " start, stop, schedthread, unschedthread or skipthread";
    }
    Tcl_SetResult(interp,
        const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
    return(TCL_ERROR);
}
    

static ASIM_THREAD
PmThreadDescToPtr (char *desc)
/*
 * Return pointer to the ASIM_THREAD object specified by 'desc'.
 * Return NULL on error.
 */
{
    ASIM_THREAD thread = NULL;
    if (sscanf(desc, "THREAD__%p", &thread) != 1)
        return(NULL);

    return(thread);
}

static bool
DecodeActionTime (char *aStr, char *tStr, CMD_ACTIONTRIGGER *action, UINT64 *time)
/*
 * Decode 'aStr' to extract the 'action' trigger and 'time'.
 * Return false on error.
 */
{
    if (strcmp(aStr, "now") == 0) {
        *action = ACTION_NOW;
        *time = 0;
        return(true);
    }
    else if (tStr != NULL) {
        //
        // 'aStr' must be "action" and 'tStr' must be "time"...

        if (sscanf(tStr, FMT64U, time) == 1) {
            if (strcmp(aStr, "cycle_once") == 0) {
                *action = ACTION_CYCLE_ONCE;
                return(true);
            }
            else if (strcmp(aStr, "cycle_period") == 0) {
                *action = ACTION_CYCLE_PERIOD;
                return(true);
            }
            else if (strcmp(aStr, "nanosecond_period") == 0) {
                *action = ACTION_NANOSECOND_PERIOD;
                return(true);
            }
            else if (strcmp(aStr, "nanosecond_once") == 0) {
                *action = ACTION_NANOSECOND_ONCE;
                return(true);
            }
            else if (strcmp(aStr, "inst_once") == 0) {
                *action = ACTION_INST_ONCE;
                return(true);
            }
            else if (strcmp(aStr, "inst_period") == 0) {
                *action = ACTION_INST_PERIOD;
                return(true);
            }
            else if (strcmp(aStr, "macroinst_once") == 0) {
                *action = ACTION_MACROINST_ONCE;
                return(true);
            }
            else if (strcmp(aStr, "macroinst_period") == 0) {
                *action = ACTION_MACROINST_PERIOD;
                return(true);
            }
            else if (strcmp(aStr, "packet_once") == 0) {
                *action = ACTION_PACKET_ONCE;
                return(true);
            }
        }
    }
    
    return(false);
}


int
PmControlCmd (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
/*
 * tcl interface to pm scheduler. Examine, add, or delete from schedule.
 */
{
    ostringstream os;
    
    if (argc < 2) {
        Tcl_SetResult(interp, "Usage: PmControl option", TCL_STATIC);
        return(TCL_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    char *option = argv[1];

    //
    // cycle            - return the current system cycle
    //
    if (strcmp(option, "cycle") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_Cycle();
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }    
    //
    // cpucycle            - return the current cycle for this cpu
    //
    else if (strcmp(option, "cpucycle") == 0) {
      UINT32 cpunum = atoi_general_unsigned(argv[2]);
        os.str(""); // clear
        os << asimSystem->SYS_Cycle(cpunum);
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }    
    //
    // nanosecond            - return the current nanoseconds simulated
    //
    if (strcmp(option, "nanosecond") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_Nanosecond();
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }    
    //
    // active           - tells if a cpu has a software context assigned
    //
    else if (strcmp(option, "active") == 0) {
        UINT32 cpunum = atoi_general_unsigned(argv[2]);
        os.str(""); // clear
        os << asimSystem->SYS_IsCpuActive(cpunum);
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }
    //
    // committed        - return the current cpu's committed instructions
    //
    else if (strcmp(option, "committed") == 0) {
        UINT32 cpunum = atoi_general_unsigned(argv[2]);
        os.str(""); // clear
        os << asimSystem->SYS_CommittedInsts(cpunum);
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }
    //
    // committedmacros        - return the current cpu's committed instructions
    //
    else if (strcmp(option, "committedmacros") == 0) {
        UINT32 cpunum = atoi_general_unsigned(argv[2]);
        os.str(""); // clear
        os << asimSystem->SYS_CommittedMacroInsts(cpunum);
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }
    //
    // globalcommitted        - return all cpus' # committed instructions
    //
    else if (strcmp(option, "globalcommitted") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_GlobalCommittedInsts();
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);	
    }
    //
    // numcpus 	- return the number of cpus in asimsystem
    //
    else if (strcmp(option, "numcpus") == 0) {
        os.str(""); // clear
        os << asimSystem->NumCpus();
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }	
    //
    // committedMarkers     - return # committed watched markers
    //
    else if (strcmp(option, "committedMarkers") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_CommittedMarkers();
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }
    //
    // commitWatchMarker    - set commit watch marker
    //
    else if (strcmp(option, "commitWatchMarker") == 0) {
        INT32 markerID = atoi_general(argv[2]);
        asimSystem->SYS_CommitWatchMarker() = markerID;
        return(TCL_OK);
    }
    //
    // beginDrain       - begin draining the pipeline inbetween samples
    //
    else if (strcmp(option, "beginDrain") == 0) {
      asimSystem->SYS_beginDrain();
      return(TCL_OK);
    }
    //
    // endDrain       - end draining the pipeline
    //
    else if (strcmp(option, "endDrain") == 0) {
      asimSystem->SYS_endDrain();
      return(TCL_OK);
    }
    //
    // ReceivedPackets - return # received packets
    // (for the network simulator)
    else if (strcmp(option, "receivedPkt") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_ReceivedPkt();
        Tcl_SetResult(interp,
            const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
        return(TCL_OK);
    }
    os.str(""); // clear
    os << "unknown option \"" << option << "\": must be cycle, cpucycle, committed, committedmacros,"
       << " nanosecond, globalcommitted, numcpus, committedMarkers, commitWatchMarker,"
       << " getprogress, doneprogress, beginDrain, or endDrain";
    Tcl_SetResult(interp,
        const_cast<char*>(os.str().c_str()), TCL_VOLATILE);
    return(TCL_ERROR);
}


static void 
PmTraceCmd (int argc, char *argv[])
{
    T1_AS(&awbTracer, "TCL_CMD: " 
          << ((argc >= 1)?argv[0]:"") << " "
          << ((argc >= 2)?argv[1]:"") << " "
          << ((argc >= 3)?argv[2]:"") << " "
          << ((argc >= 4)?argv[3]:"") << " "
          << ((argc >= 5)?argv[4]:"") << " ");
}





