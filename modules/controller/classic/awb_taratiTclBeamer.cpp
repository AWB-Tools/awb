/**************************************************************************
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
 * @author Artur Klauser
 * @brief Stub to bind remote TCL (TaratiTclBeamer) to local functions.
 */

// generic
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <iostream>

// ASIM core
#include "asim/port.h"
#include "asim/syntax.h"
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
#include "asim/provides/controller.h"
#include "asim/provides/instfeeder_interface.h"
#include "asim/provides/awb_stub.h"
#include "asim/provides/tarati_tcl_beamer.h"
#include "asim/provides/tarati_system.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

// note: the following must be in accordance with tcl.h
const int AWB_OK = 0;     // TCL_OK
const int AWB_ERROR = 1;  // TCL_ERROR

const int AWB_BUF_SIZE = 256;

int PmStateCmd (int  argc, const char *argv[], string & result);
int PmEventCmd (int  argc, const char *argv[], string & result);
int PmStripCmd (int  argc, const char *argv[], string & result);
int PmTraceCmd (int  argc, const char *argv[], string & result);
int PmMarkerCmd (int  argc, const char *argv[], string & result);
int PmSymbolCmd (int  argc, const char *argv[], string & result);
int PmScheduleCmd (int  argc, const char *argv[], string & result);
int PmControlCmd (int  argc, const char *argv[], string & result);

static ASIM_STATELINK PmStateList (void);
static ASIM_STATE PmStateDescToPtr (const char *desc);
static INT32 PmStateStates (const char *path, string & result);
static INT32 PmStateFind (const char *name, string & result);
static INT32 PmStateValue (const char *desc, string & result);

static ASIM_THREAD PmThreadDescToPtr (const char *desc);
static void PmTraceCmd(int argc, const char *argv[]);

static bool DecodeActionTime (const char *aStr, const char *tStr,
                              CMD_ACTIONTRIGGER *action, UINT64 *time);

bool statsOn = false;
extern bool stripsOn;

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
static string pendingProgress;

/*
 * Dump file for events
 */
char* event_dumpfilename = "";
FILE* event_dumpfile = NULL;

/**
 * Initialize awb.
 * @note: Called before the system is initialized (exists)
 */
void
AWB_Initialize (void)
{
    // Initialize events structure. Must be done before creatiung any bow
    // or buffer.
    EVENT(ASIM_DRAL_EVENT_CLASS::InitEvent());
}

/**
 * Activate awb if it is not already running.
 * @note: Called after the system is initialized (exists)
 */
void
AWB_Activate (void)
{
    // make sure the global has been initialized correctly
    ASSERT(AsimTarati::tclBeamer, "global tclBeamer object no initialized");

    // Register the commands we provide to awb
    AsimTarati::tclBeamer->RegisterCommand("PmState",    PmStateCmd);
    AsimTarati::tclBeamer->RegisterCommand("PmEvent",    PmEventCmd);
    AsimTarati::tclBeamer->RegisterCommand("PmStrip",    PmStripCmd);
    AsimTarati::tclBeamer->RegisterCommand("PmTrace",    PmTraceCmd);
    AsimTarati::tclBeamer->RegisterCommand("PmMarker",   PmMarkerCmd);
    AsimTarati::tclBeamer->RegisterCommand("PmSymbol",   PmSymbolCmd);
    AsimTarati::tclBeamer->RegisterCommand("PmSchedule", PmScheduleCmd);
    AsimTarati::tclBeamer->RegisterCommand("PmControl",  PmControlCmd);
}

/**
 * Add a progress message to be delivered later
 */
void
AWB_Progress (AWB_PROGRESSTYPE type, const char *args)
{
    static char *pStrs[] = AWB_PROGRESSSTRS;
    
    //
    // Add progress to the list of pending progress events.

    if ( ! pendingProgress.empty()) {
        pendingProgress += " ";
    }
    pendingProgress += pStrs[type];

    if (strlen(args) == 0) {
        pendingProgress += " {}";
    } else {
        pendingProgress += " ";
        pendingProgress += args;
    }
}    

// FIXME: the location of asimTaratiSystem needs to be seriously cleaned up
extern AsimTarati::System * asimTaratiSystem;

/**
 * Notify the workbench of the progress events that are pending,
 * and wait for it to acknowledge that it has processed all the
 * events.
 */
void
AWB_InformProgress (void)
{
    enum NotifyState {
      initial = 0,
      listen,
      accepted,
      error
    };
    static NotifyState notifyState = initial;

    if ( ! pendingProgress.empty()) {
        TRACE(Trace_Cmd, cout << "TCL_CMD: "
                              << "BatchProgress { "
                              << pendingProgress
                              << " }"
                              << endl);

        switch (notifyState) {
          case initial:
            cout << "Waiting for remote TCL controller to connect ... "
                 << flush;
            notifyState = listen;
            break;
          case listen:
            break;
          case accepted:
            // notify client that we are waiting
            AsimTarati::tclBeamer->NotifyClient();
            break;
          default:
            ASIMERROR("unknown notifyState " << notifyState);
        }
    }

    // note: we assume pendingProgress is read and reset via the Tarati
    //       when it has been processed;
    //       (this is done via PmControl getprogress | doneprogress)
    while ( ! pendingProgress.empty()) {
        // the progress notify channel will finish its connection setup
        // while we wait for the progress to get processed, so we have to
        // handle this here and send out the notification that we could
        // not send earlier because the notify channel setup was still
        // pending;
        if (notifyState == listen) {
            // finish the notification connection setup
            if (AsimTarati::tclBeamer->NotifyAccept()) {
              // notify channel accepted the client connection
              cout << "connected" << endl << flush;
              notifyState = accepted;
              // notify client that we are waiting
              AsimTarati::tclBeamer->NotifyClient();
            }
        }
        // yield the CPU until we've got work to do
        asimTaratiSystem->Wait();
        asimTaratiSystem->Work();
    }
}

/**
 * Exit awb...
 */
void
AWB_Exit (void)
{
    // Send awb a progress message ordering it to exit, then wait
    // for awb to signal that it has exited.

    AWB_Progress(AWBPROG_EXIT);
    AWB_InformProgress();
}

/******************************************************************
 *
 * AWB low-level commands
 *
 *****************************************************************/

static ASIM_STATE
PmStateDescToPtr (const char *desc)
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
PmStateCmd (int argc, const char *argv[], string & result)
/*
 * tcl interface to pm state. Return's a list of available state, or
 * specific state attributes.
 */
{
    ostringstream os;

    if (argc < 2) {
        result = "Usage: PmState option ?arg ...?";
        return(AWB_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    const char *option = argv[1];

    //
    // states     - optional path argument
    //              returns list of state descriptors for that path
    //
    if (strcmp(option, "states") == 0) {
        if ((argc != 2) && (argc != 3)) {
            result = "Usage: PmState states ?path?";
            return(AWB_ERROR);
        }

        return(PmStateStates((char *)((argc == 2) ? (char *)(NULL) : argv[2]),
                             result));
    }
    //
    // find    - requires name argument
    //           returns state descriptor for specified name or empty
    //           string if can find named state
    //
    else if (strcmp(option, "find") == 0) {
        if (argc != 3) {
            result = "Usage: PmState find ?name?";
            return(AWB_ERROR);
        }

        return(PmStateFind(argv[2], result));
    }
    //
    // dump    - requires filename argument
    //           dump statistics to file
    //
    else if (strcmp(option, "dump") == 0) {
        if (argc != 3) {
            result = "Usage: PmState dump ?filename?";
            return(AWB_ERROR);
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

        const char *filename = argv[2];

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
        return(AWB_OK);
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
            return(AWB_OK);
        }

        if (argc != 3) {
            result = "Usage: PmState suspend ?state?";
            return(AWB_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            statsOn = false; 
            state->Suspend();
            return(AWB_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'suspend'";
        result = os.str();
        return(AWB_ERROR);
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
            return(AWB_OK);
        }

        if (argc != 3) {
            result = "Usage: PmState unsuspend ?state?";
            return(AWB_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
	        statsOn = true; 
            state->Unsuspend();
            return(AWB_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'unsuspend'";
        result = os.str();
        return(AWB_ERROR);
    }
    //
    // value   - requires state descriptor argument
    //           returns state's value, as a list if state is an array
    //
    else if (strcmp(option, "value") == 0) {
        if (argc != 3) {
            result = "Usage: PmState value ?state?";
            return(AWB_ERROR);
        }

        return(PmStateValue(argv[2], result));
    }
    //
    // path    - requires state descriptor argument
    //           returns path of corresponding state
    //
    else if (strcmp(option, "path") == 0) {
        if (argc != 3) {
            result = "Usage: PmState path ?state?";
            return(AWB_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            result = state->Path();
            return(AWB_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'path'";
        result = os.str();
        return(AWB_ERROR);
    }
    //
    // name    - requires state descriptor argument
    //           returns name of corresponding state
    //
    else if (strcmp(option, "name") == 0) {
        if (argc != 3) {
            result = "Usage: PmState name ?state?";
            return(AWB_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            result = state->Name();
            return(AWB_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'name'";
        result = os.str();
        return(AWB_ERROR);
    }
    //
    // desc    - requires state descriptor argument
    //           returns description of corresponding state
    //
    else if (strcmp(option, "desc") == 0) {
        if (argc != 3) {
            result = "Usage: PmState desc ?state?";
            return(AWB_ERROR);
        }

        ASIM_STATE state = PmStateDescToPtr(argv[2]);
        if (state != NULL) {
            result = state->Description();
            return(AWB_OK);
        }

        os.str(""); // clear
        os << "invalid state descriptor \"" << argv[2] << "\" for 'desc'";
        result = os.str();
        return(AWB_ERROR);
    }
    
    //
    // unknown option
    //
    os.str(""); // clear
    os << "unknown option \"" << option << "\": must be states, find, dump,"
       << " suspend, unsuspend, value, path, name, or desc";
    
    result = os.str();
    return(AWB_ERROR);
}        
    

static INT32
PmStateStates (const char *path, string & result)
/*
 * Search the state list to find states whose path's start with 'path'. We
 * do a case-insensitive search. If 'path' == NULL then return a list of
 * all state. Return a list of descriptors listing the matching states.
 */
{
    char lastPath[AWB_BUF_SIZE+10];
    lastPath[0] = '\0';

    ASIM_STATELINK scan = PmStateList();
    ostringstream os;
    bool first = true;
    while (scan != NULL) {
        ASIM_STATE state = scan->state;

        //
        // If 'path' is a prefix of 'state's path, then it can either
        // be equal to 'state's path or just part of it.
        
        if ((path == NULL) ||
            (strncasecmp(state->Path(), path, strlen(path)) == 0))
        {
            //
            // If 'path' equals 'state's path, then return
            // 'state's descriptor.
            
            if ((path == NULL) || (strcasecmp(state->Path(), path) == 0)) {
                if (first) {
                    first = false;
                } else {
                    os << " ";
                }
                os << "{" << "STATE__" << fmt_p((void *)state) << "}";
            }
            else {
                //
                // construct the next longest path prefix for 'state'.

                char prefix[AWB_BUF_SIZE+10];
                char *nel = strchr(state->Path()+strlen(path)+1, '/');
                if (nel == NULL) {
                    ASSERTX(int(strlen(state->Path())) < AWB_BUF_SIZE);
                    strcpy(prefix, state->Path());
                }
                else {
                    ASSERTX((nel - state->Path() + 1) < AWB_BUF_SIZE);

                    strncpy(prefix, state->Path(), nel - state->Path());
                    prefix[nel-state->Path()] = '\0';
                }
                
                //
                // If the new longer prefix is different than the
                // last prefix we saw, then return it.

                if (strcasecmp(lastPath, prefix) != 0) {
                    strcpy(lastPath, prefix);
                    if (first) {
                        first = false;
                    } else {
                        os << " ";
                    }
                    os << "{" << "MODULE__" << prefix << "}";
                }
            }
        }
        result = os.str();
        
        scan = scan->next;
    }

    return(AWB_OK);
}


static INT32
PmStateFind (const char *name, string & result)
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
            result = os.str();
            return(AWB_OK);
        }

        scan = scan->next;
    }

    result = "";
    return(AWB_OK);
}


static INT32
PmStateValue (const char *desc, string & result)
/*
 * Return 'state's value.
 */
{
    ostringstream os;
    bool first = true;

    ASIM_STATE state = PmStateDescToPtr(desc);
    if (state != NULL) {
        //
        // Iterate through 'state's values, appending them
        // to the result list.

        switch (state->Type()) {
          case STATE_UINT:
            os.str(""); // clear
            for (UINT32 i = 0; i < state->Size(); i++) {
                if (first) {
                    first = false;
                } else {
                    os << " ";
                }
                os << "{" << state->IntValue(i) << "}";
            }
            result = os.str();
            break;
          case STATE_FP:
            os.str(""); // clear
            for (UINT32 i = 0; i < state->Size(); i++) {
                if (first) {
                    first = false;
                } else {
                    os << " ";
                }
                os << "{" << fmt("f", state->FpValue(i)) << "}";
            }
            result = os.str();
          case STATE_HISTOGRAM:
            os.str(""); // clear
            os << "{state type STATE_HISTOGRAM not supported}";
            result = os.str();
            break;
          case STATE_RESOURCE:
            os.str(""); // clear
            os << "{state type STATE_RESOURCE not supported}";
            result = os.str();
            break;
          case STATE_THREE_DIM_HISTOGRAM:
            os.str(""); // clear
            os << "{state type STATE_THREE_DIM_HISTOGRAM not supported}";
            result = os.str();
            break;
          default:
            os.str(""); // clear
            os << "{unknown state type " << state->Type() << "}";
            result = os.str();
        }

        return(AWB_OK);
    }

    os.str(""); // clear
    os << "null state descriptor for 'value' \"" << desc << "\"";
    result = os.str();
    return(AWB_ERROR);
}

int
PmEventCmd (int argc, const char *argv[], string & result)
{
    if (argc != 2 && argc != 3) {
        result = "Usage: AwbEvents on|off|filename";
        return(AWB_ERROR);
    }

    if (!runWithEventsOn)
    {
        result = "You are trying to generate events in a model not compiled "
            "with events. Build the model with EVENTS=1";
        return(AWB_ERROR);
    }

    const char *option = argv[1];

    if (strcmp(option, "on") == 0) {
        DRALEVENT(TurnOn());
        eventsOn = true;
        if (firstTurnedOn)
        {
            firstTurnedOn = false;
            DRALEVENT(StartActivity());
        }
    }
    else if (strcmp(option, "off") == 0) {
        DRALEVENT(TurnOff());
        eventsOn = false;
    }
    else if (strcmp(option, "filename") == 0) {
        if (argc != 3) {
            result = "Usage: AwbEvents filename ?filename?";
            return(AWB_ERROR);
        }
        /*
        if (eventsOn)
        {
            result = "The events file name can only be changed if the dral "
                    "server is turned off";
            return(TCL_ERROR);
        }
        */
        DRALEVENT(ChangeFileName(argv[2]));
    }
    else
    {
        result = "Usage: AwbEvents on|off|filename";
        return(AWB_ERROR);
    }
    return(AWB_OK);
}    

int
PmStripCmd (int argc, const char *argv[], string & result)
{
    if (argc != 2) {
        result = "Usage: PmStrip on|off";
        return(AWB_ERROR);
    }

    const char *option = argv[1];
    if (strcmp(option, "on") == 0) {
        stripsOn = true;
    }
    else {
        stripsOn = false;
    }
    return(AWB_OK);
}    

int
PmTraceCmd (int argc, const char *argv[], string & result)
{
    if (argc != 2) {
        result = "Usage: PmTrace on|off";
        return(AWB_ERROR);
    }

    const char *option = argv[1];
    if (strcmp(option, "on") == 0) {
        traceOn = true;
    }
    else {
        traceOn = false;
    }
    return(AWB_OK);
}    

//----------------------------------------------------------------------------

int
PmMarkerCmd (int argc, const char *argv[], string & result)
/*
 * tcl interface to pm marker. Set / Clear markers
 */
{
    ostringstream os;

    if (argc < 2) {
        result = "Usage: PmMarker option ?arg ...?";
        return(AWB_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    const char *option = argv[1];

    //
    // set        - requires thread, markerID, subcommand
    //              subcommand: pc requires address
    //              subcommand: inst requires bits, mask
    // syntax: PmMarker set <thread> <id> {("pc" <addr>) | "inst" <bits> <mask>}
    //
    if (strcmp(option, "set") == 0) {
        if ((argc < 5)) {
            result = "Usage: PmMarker set ?thread? ?id? ?subcmd?";
            return(AWB_ERROR);
        }

        ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
        INT32 markerID = atoi_general(argv[3]);
        const char *subcmd = argv[4];

        // subcommand parsing
        //
        // subcommand: pc <addr>
        if (strcmp(subcmd, "pc") == 0) {
            if ((argc != 6)) {
                result = "Usage: PmMarker set ?thread? ?id? pc ?addr?";
                return(AWB_ERROR);
            }

            UINT64 addr = atoi_general_unsigned(argv[5]);

            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_SET_PC,
                                      thread->IStreamHandle(),
                                      markerID,
                                      addr);
            return(AWB_OK);
        }
        //
        // subcommand: inst
        else if (strcmp(subcmd, "inst") == 0) {
            if ((argc != 7)) {
                result = "Usage: PmMarker set ?thread? ?id? inst ?bits? ?mask?";
                return(AWB_ERROR);
            }

            UINT32 bits = atoi_general_unsigned(argv[5]);
            UINT32 mask = atoi_general_unsigned(argv[6]);

            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_SET_INST,
                                      thread->IStreamHandle(),
                                      markerID,
                                      /*pc*/0,
                                      bits,
                                      mask);
            return(AWB_OK);
        }
        //
        // unknown subcommand
        os.str(""); // clear
        os << "PmMarker set: unknown subcommand \"" << subcmd << "\""
           << ": must be pc, or inst";
        result = os.str();
        return(AWB_ERROR);
    }
    //
    // clear      - requires thread, markerID, subcommand
    //              subcommand: pc requires address
    //              subcommand: all
    // syntax: PmMarker clear <thread> <id> {("pc" <addr>) | "all"}
    //
    else if (strcmp(option, "clear") == 0) {
        if ((argc < 5)) {
            result = "Usage: PmMarker clear ?thread? ?id? ?subcmd?";
            return(AWB_ERROR);
        }

        ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
        INT32 markerID = atoi_general(argv[3]);
        const char *subcmd = argv[4];

        // subcommand parsing
        //
        // subcommand: pc <addr>
        if (strcmp(subcmd, "pc") == 0) {
            if ((argc != 6)) {
                result = "Usage: PmMarker clear ?thread? ?id? pc ?addr?";
                return(AWB_ERROR);
            }


            UINT64 addr = atoi_general_unsigned(argv[5]);


            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_CLEAR_PC,
                                      thread->IStreamHandle(),
                                      markerID,
                                      addr);
            return(AWB_OK);
        }
        //
        // subcommand: all
        else if (strcmp(subcmd, "all") == 0) {
            if ((argc != 5)) {
                result = "Usage: PmMarker clear ?thread? ?id? all";
                return(AWB_ERROR);
            }

            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_CLEAR_ALL,
                                      thread->IStreamHandle(),
                                      markerID);
            return(AWB_OK);
        }
        //
        // unknown subcommand
        os.str(""); // clear
        os << "PmMarker clear: unknown subcommand \"" << subcmd << "\""
           << ": must be pc, or all";
        result = os.str();
        return(AWB_ERROR);
    }
    
    //
    // unknown option
    //
    os.str(""); // clear
    os << "PmMarker: unknown option \"" << option << "\""
       << ": must be set, or clear";
    result = os.str();
    return(AWB_ERROR);
}        
    
//----------------------------------------------------------------------------

int
PmSymbolCmd (int argc, const char *argv[], string & result)
/*
 * tcl interface to pm symbol table; query symbol addresses
 */
{
    ostringstream os;

    if (argc < 2) {
        result = "Usage: PmSymbol option ?arg ...?";
        return(AWB_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    const char *option = argv[1];

    //
    // find       - requires thread, symbol name
    //
    // syntax: PmSmbol find <thread> <name>
    //
    if (strcmp(option, "find") == 0) {
        if ((argc != 4)) {
            result = "Usage: PmSymbol find ?thread? ?name?";
            return(AWB_ERROR);
        }

        UINT64 addr;
        const char *symname = argv[3];
        if (strncmp(argv[2], "THREAD__", strlen("THREAD__")) == 0) {
            ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
            addr = thread->IFeeder()->Symbol(thread->IStreamHandle(),
                // yikes, Symbol() is missing const
                const_cast<char*>(symname));
        } else {
            UINT32 tid = atoi_general_unsigned(argv[2]);
            ASSERT(tid == 0, "Only 0 thread-id supported in PmSymbol");
            addr = IFEEDER_BASE_CLASS::SingleFeederSymbolHack(
                // another yikes, SingleFeederSymbolHack() is missing const too
                const_cast<char*>(symname));
        }
        if (addr == (UINT64) NULL) {
            os.str(""); // clear
            os << "PmSymbol: unknown symbol \"" << symname << "\"";
            result = os.str();
            return(AWB_ERROR);
        } else {
            os.str(""); // clear
            os << "0x" << fmt_x(addr);
            result = os.str();
            return(AWB_OK);
        }

    }
    
    //
    // unknown option
    //
    os.str(""); // clear
    os << "PmSymbol: unknown option \"" << option << "\": must be find";
    result = os.str();
    return(AWB_ERROR);
}        
    
//----------------------------------------------------------------------------

int
PmScheduleCmd (int argc, const char *argv[], string & result)
/*
 * tcl interface to pm scheduler. Examine, add, or delete from schedule.
 */
{
    ostringstream os;
    bool timeError = false;
    
    if (argc < 2) {
        result = "Usage: PmSchedule option ?arg ...?";
        return(AWB_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    const char *option = argv[1];

    //
    // progress     - 'inst'/'cycle'/'nanosecond' and 'period'/'clear'
    //
    if (strcmp(option, "progress") == 0) {
        if (argc != 4) {
            result = "Usage:\tPmSchedule progress ?inst|cycle|nanosecond|macroinst? ?clear|period?\n";
            return(AWB_ERROR);
        }

        const char *period = argv[3];
        CMD_ACTIONTRIGGER action;
        if (strcmp(argv[2], "inst") == 0)
            action = ACTION_INST_PERIOD;
        else if (strcmp(argv[2], "cycle") == 0)
            action = ACTION_CYCLE_PERIOD;
        else if (strcmp(argv[2], "nanosecond") == 0)
            action = ACTION_NANOSECOND_PERIOD;
        else if (strcmp(argv[2], "macroinst") == 0)
            action = ACTION_MACROINST_PERIOD;

        else {
            os.str(""); // clear
            os << "invalid action option \"" << argv[2] << "\""
               << ": must be inst or cycle or nanosecond";
            result = os.str();
            return(AWB_ERROR);
        }            
        
        if (strcmp(period, "clear") == 0) {
            if (action == ACTION_CYCLE_PERIOD)
                CMD_Progress(AWBPROG_CLEARCYCLE, "", ACTION_NOW);
            else if (action == ACTION_NANOSECOND_PERIOD)
                CMD_Progress(AWBPROG_CLEARNANOSECOND, "", ACTION_NOW);
            else if (action == ACTION_MACROINST_PERIOD)
                CMD_Progress(AWBPROG_CLEARMACROINST, "", ACTION_NOW);

            else
                CMD_Progress(AWBPROG_CLEARINST, "", ACTION_NOW);
        }
        else if (action == ACTION_CYCLE_PERIOD) {
            CMD_Progress(AWBPROG_CYCLE, "", action, atoi_general(period));
        }
        else if (action == ACTION_NANOSECOND_PERIOD) {
            CMD_Progress(AWBPROG_NANOSECOND, "", action, atoi_general(period));
        }
        else if (action == ACTION_MACROINST_PERIOD) {
            CMD_Progress(AWBPROG_MACROINST, "", action, atoi_general(period));
        }

        else {
            CMD_Progress(AWBPROG_INST, "", action, atoi_general(period));
        }

        return(AWB_OK);
    }
    //
    // start   - start performance model
    //
    else if (strcmp(option, "start") == 0) {
        if (argc != 2) {
            result = "Usage: PmSchedule start";
            return(AWB_ERROR);
        }

        //
        // It only makes sense to have a start action for "now". A start
        // action for a future time is meaningless if the model is already
        // running, and if the model is not running, then the future time
        // will never be reached.

        CMD_Start();
        return(AWB_OK);
    }
    //
    // stop     - requires action time
    //            stop the performance model
    //
    else if (strcmp(option, "stop") == 0) {
        if ((argc != 3) && (argc != 4)) {
            result = "Usage: PmSchedule stop ?action? ?time?";
            return(AWB_ERROR);
        }

        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[2], (char *)((argc == 4) ? argv[3] : 
			     (char *)(NULL)), &action, &time)) {
            CMD_Stop(action, time);
            return(AWB_OK);
        }

        timeError = true;
    }
    //
    // exit     - requires action time
    //            exit the performance model
    //
    else if (strcmp(option, "exit") == 0) {
        if ((argc != 3) && (argc != 4)) {
            result = "Usage: PmSchedule exit ?action? ?time?";
            return(AWB_ERROR);
        }

        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[2], (char *)((argc == 4) ? argv[3] : (char *)(NULL)), &action, &time)) {
            CMD_Exit(action, time);
            return(AWB_OK);
        }

        timeError = true;
    }
    //
    // schedthread     - requires action, time, thread
    //                   schedule thread on performance model
    //
    else if (strcmp(option, "schedthread") == 0) {
        if ((argc != 4) && (argc != 5)) {
            result = "Usage: PmSchedule schedthread ?thread? ?action? ?time?";
            return(AWB_ERROR);
        }

        ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
        if (thread == NULL) {
            os.str(""); // clear
            os << "invalid thread descriptor \"" << argv[2] << "\"";
            result = os.str();
            return(AWB_ERROR);
        }
        
        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[3], (char *)((argc == 5) ? argv[4] : (char *)(NULL)), &action, &time)) {
            CMD_ScheduleThread(thread, action, time);
            return(AWB_OK);
        }

        timeError = true;
    }
    //
    // unschedthread     - requires action, time, thread
    //                     unschedule thread on performance model
    //
    else if (strcmp(option, "unschedthread") == 0) {
        if ((argc != 4) && (argc != 5)) {
            result = "Usage: PmSchedule unschedthread ?thread? ?action? ?time?";
            return(AWB_ERROR);
        }

        ASIM_THREAD thread = PmThreadDescToPtr(argv[2]);
        if (thread == NULL) {
            os.str(""); // clear
            os << "invalid thread descriptor \"" << argv[2] << "\"";
            result = os.str();
            return(AWB_ERROR);
        }
        
        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[3], (char *)((argc == 5) ? argv[4] : (char *)NULL), &action, &time)) {
            CMD_UnscheduleThread(thread, action, time);
            return(AWB_OK);
        }

        timeError = true;
    }
    //
    // skipthread     - requires action, time, thread
    //                  skip thread
    //
    else if (strcmp(option, "skipthread") == 0) {
        if ((argc != 6) && (argc != 7)) {
            result = "Usage: PmSchedule skipthread ?thread? ?insts? ?markerID? ?action? ?time?";
            return(AWB_ERROR);
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
                result = os.str();
                return(AWB_ERROR);
            }
        }

        UINT64 insts = atoi_general_unsigned(argv[3]);
        INT32 markerID = atoi_general(argv[4]);
        CMD_ACTIONTRIGGER action;
        UINT64 time;
        if (DecodeActionTime(argv[5], (char *)((argc == 7) ? argv[6] : (char *)(NULL)), &action, &time)) {
            CMD_SkipThread(thread, insts, markerID, action, time);
            return(AWB_OK);
        }

        timeError = true;
    }

    //
    // unknown option or time format error
    //
    if (timeError) {
        os.str(""); // clear
        os << "invalid time option \"" << argv[2] << "\": must be now,"
           << " cycle_once, cycle_period, nanosecond_once, nanosecond_period, inst_once, or inst_period";
    }
    else {
        os.str(""); // clear
        os << "unknown option \"" << option << "\": must be progress, exit,"
           << " start, stop, schedthread, unschedthread or skipthread";
    }
    result = os.str();
    return(AWB_ERROR);
}
    

static ASIM_THREAD
PmThreadDescToPtr (const char *desc)
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
DecodeActionTime (
    const char *aStr,
    const char *tStr,
    CMD_ACTIONTRIGGER *action, UINT64 *time)
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
            else if (strcmp(aStr, "nanosecond_once") == 0) {
                *action = ACTION_NANOSECOND_ONCE;
                return(true);
            }
            else if (strcmp(aStr, "nanosecond_period") == 0) {
                *action = ACTION_NANOSECOND_PERIOD;
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
PmControlCmd (int argc, const char *argv[], string & result)
/*
 * tcl interface to pm scheduler. Examine, add, or delete from schedule.
 */
{
    ostringstream os;
    
    if (argc < 2) {
        result = "Usage: PmControl option";
        return(AWB_ERROR);
    }

    PmTraceCmd(argc, argv);

    fflush(stdout);
    fflush(stderr);

    //
    // Parse the option field.
    
    const char *option = argv[1];

    //
    // cycle            - return the current system cycle
    //
    if (strcmp(option, "cycle") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_Cycle();
        result = os.str();
        return(AWB_OK);
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
    // committed        - return the current cpu's committed instructions
    //
    else if (strcmp(option, "committed") == 0) {
        if (argc != 3) {
            result = "Usage: PmControl committed cpunum";
            return(AWB_ERROR);
        }

        UINT32 cpunum = atoi_general_unsigned(argv[2]);
        os.str(""); // clear
        os << asimSystem->SYS_CommittedInsts(cpunum);
        result = os.str();
        return(AWB_OK);
    }
    //
    // committedmacros - return the current cpu's committed macro instructions
    //
    else if (strcmp(option, "committedmacros") == 0) {
        if (argc != 3) {
            result = "Usage: PmControl committedmacros cpunum";
            return(AWB_ERROR);
        }

        UINT32 cpunum = atoi_general_unsigned(argv[2]);
        os.str(""); // clear
        os << asimSystem->SYS_CommittedMacroInsts(cpunum);
        result = os.str();
        return(AWB_OK);
    }
    //
    // nanosecond        - return the current nanoseconds simulated
    //
    else if (strcmp(option, "nanosecond") == 0) {
        if (argc != 3) {
            result = "Usage: PmControl nanosecond";
            return(AWB_ERROR);
        }

        os.str(""); // clear
        os << asimSystem->SYS_Nanosecond();
        result = os.str();
        return(AWB_OK);
    }
    //
    // globalcommitted        - return all cpus' # committed instructions
    //
    else if (strcmp(option, "globalcommitted") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_GlobalCommittedInsts();
        result = os.str();
        return(AWB_OK);	
    }
    //
    // numcpus 	- return the number of cpus in asimsystem
    //
    else if (strcmp(option, "numcpus") == 0) {
        os.str(""); // clear
        os << asimSystem->NumCpus();
        result = os.str();
        return(AWB_OK);
    }	
    //
    // committedMarkers     - return # committed watched markers
    //
    else if (strcmp(option, "committedMarkers") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_CommittedMarkers();
        result = os.str();
        return(AWB_OK);
    }
    //
    // commitWatchMarker    - set commit watch marker
    //
    else if (strcmp(option, "commitWatchMarker") == 0) {
        if (argc != 3) {
            result = "Usage: PmControl commitWatchMarker id";
            return(AWB_ERROR);
        }

        INT32 markerID = atoi_general(argv[2]);
        asimSystem->SYS_CommitWatchMarker() = markerID;
        return(AWB_OK);
    }
    //
    // getprogress       - get pending progress messages
    //
    else if (strcmp(option, "getprogress") == 0) {
      result = pendingProgress;
      return(AWB_OK);
    }
    //
    // doneprogress       - clear pending progress messages
    //
    else if (strcmp(option, "doneprogress") == 0) {
      pendingProgress.clear();
      return(AWB_OK);
    }
    //
    // beginDrain       - begin draining the pipeline inbetween samples
    //
    else if (strcmp(option, "beginDrain") == 0) {
      asimSystem->SYS_beginDrain();
      return(AWB_OK);
    }
    //
    // endDrain       - end draining the pipeline
    //
    else if (strcmp(option, "endDrain") == 0) {
      asimSystem->SYS_endDrain();
      return(AWB_OK);
    }
    //
    // ReceivedPackets - return # received packets
    // (for the network simulator)
    else if (strcmp(option, "receivedPkt") == 0) {
        os.str(""); // clear
        os << asimSystem->SYS_ReceivedPkt();
        result = os.str();
        return(AWB_OK);
    }
    os.str(""); // clear
    os << "unknown option \"" << option << "\": must be cycle, committed, nanosecond, "
       << " globalcommitted, numcpus, committedMarkers, commitWatchMarker,"
       << " getprogress, doneprogress, beginDrain, endDrain, receivedPkt";
    result = os.str();
    return(AWB_ERROR);
}


static void 
PmTraceCmd (int argc, const char *argv[])
{
    TRACE(Trace_Cmd, cout << "TCL_CMD: " 
                          << ((argc >= 1)?argv[0]:"") << " "
                          << ((argc >= 2)?argv[1]:"") << " "
                          << ((argc >= 3)?argv[2]:"") << " "
                          << ((argc >= 4)?argv[3]:"") << " "
                          << ((argc >= 5)?argv[4]:"") << " "
                          << endl);
}
