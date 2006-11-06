/*
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
 * @author David Goodwin (modified to remove TCL by Mark Charney)
 * @brief Driver for "Architects Workbench", awb
 *
 *
 * FIXME 11/19/03 Mark Charney: This code is pretty hideous. I made
 * the mistake of converting the TCL to C++ somewhat mechanically.
 * All the old TCL functionality is not present in this code. There
 * is no general interpreter, and skipping and markers are not fully
 * supported.
 */

// generic
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <list>
#include <string>

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
#include "asim/provides/workbench.h"
#include "asim/provides/controller.h"
#include "asim/provides/instfeeder_interface.h"

// ASIM local module
#include "awb-notcl.h"

//FIXME: remove this IoFormat stuff!
namespace iof = IoFormat;
using namespace iof;
using namespace std;

////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////



#define SUCCESS 1
#define FAILURE 0

#define EMSG(x) \
({\
    cerr << __FILE__ << ":" << __LINE__ << ": " << x << endl; \
})

#define XMSG(x) \
({ \
       TMSG(Trace_Cmd, __FILE__ << ":" << __LINE__ << ": " <<  x); \
})
#define DMSG(x) \
({\
    cout << __FILE__ << ":" << __LINE__ << ": " << x << endl; \
})

////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////

void
PmStrip(string& onoff);

void
PmTrace(string& onoff);


string
PmStates(string& path);

string
PmStateFind(string name );


static ASIM_STATELINK
PmStateList (void);

static void
AwbStartInterpreter (void);

static ASIM_STATE
PmStateDescToPtr (const string& desc);

static ASIM_THREAD
PmThreadDescToPtr (const string& desc);

static string
PmStateStates (const string& path);

static list<string>
PmStateValue (char *desc);


static bool
DecodeActionTime(char *aStr,
                 char *tStr,
                 CMD_ACTIONTRIGGER *action,
                 UINT64 *time);
static bool
DecodeAction(string& saction,
             CMD_ACTIONTRIGGER& action);

////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////

bool statsOn = false; 
extern bool stripsOn; 

/*
 * File holding commands for awb to execute after initializing.
 */
char *awbCmdFile = "awbcmds"; // default -- can be overridden by args.cpp

/*
 * not used. Just for compatibility with args.cpp '-wb' option
 */
char *overrideWorkbench = NULL;

/*
 * Pending progress events
 */

static list<string> pendingProgress;

static COMMAND_PARSER_CLASS* awbCmdIntrp;

/*
 * Dump file for events
 */
char* event_dumpfilename = "";
FILE* event_dumpfile = NULL;

//////////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////////
static INT32
atoi(string& s)
{
    return ::atoi(s.c_str());
}

static void
myexit(int r)
{
    ::exit(r);
}

static bool
match_nocase(const string& s1,
             const string& s2)
{
    string::const_iterator p1 = s1.begin();
    string::const_iterator p2 = s2.begin();
    while(p1 != s1.end() && p2 != s2.end())
    {
        if (toupper(*p1) != toupper(*p2))
        {
            return false;
        }
        p1++;
        p2++;
    }

    if (s1.size() == s2.size())
    {
        return true;
    }
    return false;
}



///////////////////////////////////////////////////////////////////////////


void
AWB_Initialize (void)
{
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
AWB_Progress (AWB_PROGRESSTYPE type,
              string args)
{
    static char *pStrs[] = AWB_PROGRESSSTRS;
    
    //
    // Add progress to the list of pending progress events.

    XMSG("AWB_Progress: " << pStrs[type] << " args: " << args);
    pendingProgress.push_back(pStrs[type]);

    if (args.length() > 0)
    {
        pendingProgress.push_back(args);
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
    XMSG("AWB_InformProgress");
    if ( ! pendingProgress.empty() )
    {
    
        awbCmdIntrp->BatchProgress( pendingProgress );

        pendingProgress.clear();
    }
}

void
AWB_Exit (void)
/*
 * Exit awb...
 */
{
    XMSG("AWB_Exit");
    //
    // Send awb a progress message ordering it to exit, then wait
    // for awb to signal that it has exited.

    AWB_Progress(AWBPROG_EXIT);
    AWB_InformProgress();

    delete awbCmdIntrp;
    awbCmdIntrp=0;
}



static void
AwbStartInterpreter (void)
/*
 * Start point for thread responding to event in awb.
 */
{
    XMSG("AwbStartInterpreter");
    awbCmdIntrp = new COMMAND_PARSER_CLASS(awbCmdFile);
}


/******************************************************************
 *
 * Tcl commands
 *
 *****************************************************************/
 

static ASIM_STATE
PmStateDescToPtr (const string& desc)
/*
 * Return pointer to the ASIM_STATE object specified by 'desc'.
 * Return NULL on error.
 */
{
    ASIM_STATE state = NULL;
    if (sscanf(desc.c_str(), "STATE__%p", &state) != 1)
        return(NULL);

    return(state);
}


static ASIM_STATELINK 
PmStateList (void)  // MJC: ok
/*
 * Return the list of exposed performance model state.
 */
{
    XMSG("PmStateList");

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

static void
prep()
{
    fflush(stdout);
    fflush(stderr);
}



string
PmStates(string& path)
{
    prep();
    // states     - optional path argument
    //              returns list of state descriptors for that path
    //
    return PmStateStates( path );
}





void
PmStateDump(string& filename)
{
    //           dump statistics to file
    
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
    

    STATE_OUT stateOut = new STATE_OUT_CLASS(filename.c_str());
    
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
}

void PmStateSuspend(string& thread_desc)
{
    prep();
    //
    // suspend   - requires state descriptor argument
    //             suspends the specified state
    //
    if (thread_desc == "all")
    {
        // suspend all
        statsOn = false; 
        for (ASIM_STATELINK scan = PmStateList();
             scan != NULL;
             scan = scan->next)
        {
                scan->state->Suspend();
        }
        return;
    }

    ASIM_STATE state = PmStateDescToPtr(thread_desc);
    if (state != NULL) {
        statsOn = false; 
        state->Suspend();
        return;
    }

    EMSG("invalid state descriptor [" << thread_desc << "] for 'suspend'");
    myexit(1);
}

void PmStateUnsuspend(string& thread_desc)
{
    XMSG("PmStateUnsuspend " << thread_desc);
    prep();
    //
    // unsuspend   - requires state descriptor argument
    //             unsuspends the specified state
    //
    if (thread_desc == "all")
    {
        // unsuspend all
        statsOn = false; 
        for (ASIM_STATELINK scan = PmStateList();
             scan != NULL;
             scan = scan->next)
        {
            //XMSG("PmStateUnsuspend: unsuspending " << scan->state->Name());
            scan->state->Unsuspend();
        }
        return;
    }

    ASIM_STATE state = PmStateDescToPtr(thread_desc);
    if (state != NULL) {
        statsOn = false; 
        state->Unsuspend();
        return;
    }

    EMSG("invalid state descriptor [" << thread_desc << "] for 'unsuspend'");
    myexit(1);
}





const char*
PmStatePath(string& arg_state)
{
    // path    - requires state descriptor argument
    //           returns path of corresponding state

    ASIM_STATE state = PmStateDescToPtr(arg_state);
    if (state != NULL) {
        return state->Path() ;
    }
    
    EMSG("invalid state descriptor \[" << arg_state << "] for 'path'");
    myexit(1);
    return 0;
}

const char*
PmStateName(string& arg_state)
{
    // name    - requires state descriptor argument
    //           returns name of corresponding state

    ASIM_STATE state = PmStateDescToPtr(arg_state);
    if (state != NULL) {
        return state->Name();
    }
    
    EMSG( "invalid state descriptor \[" << arg_state  << "] for 'name'");
    myexit(1);
    return 0;
}

const char*
PmStateDesc(string& arg_state)
{
    // desc    - requires state descriptor argument
    //           returns description of corresponding state
    ASIM_STATE state = PmStateDescToPtr(arg_state);
    if (state != NULL) {
        return state->Description();
    }

    EMSG("invalid state descriptor \[" << arg_state << "] for 'desc'");
    myexit(1);
    return 0;
}
    
    

static string
PmStateStates (const string& path) //FIXME
/*
 * Search the state list to find states whose path's start with 'path'. We
 * do a case-insensitive search. If 'path' == NULL then return a list of
 * all state. Return a list of descriptors listing the matching states.
 */
{

    return "Not done yet";
    // MJC: This function looks pretty buggy. strcase and strncase are mixed.
#if 0
    const unsigned int AWB_TCLBUF_SZ = 256;
    char lastPath[AWB_TCLBUF_SZ+10];
    lastPath[0] = '\0';

    ASIM_STATELINK scan = PmStateList();
    while (scan != NULL) {
        ASIM_STATE state = scan->state;
        ostringstream os;

        //
        // If 'path' is a prefix of 'state's path, then it can either
        // be equal to 'state's path or just part of it.
        
        if ((path == NULL) ||
            (strncasecmp(state->Path(), path, strlen(path)) == 0))
        {
            //
            // If 'path' equals 'state's path, then return
            // 'state's descriptor.
            
            if ((path == NULL) ||
                (strcasecmp(state->Path(), path) == 0))
            {
                os.str(""); // clear
                os << "STATE__" << fmt_p((void *)state);
                Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
            }
            else
            {
                //
                // construct the next longest path prefix for 'state'.

                char prefix[AWB_TCLBUF_SZ+10];
                char *nel = strchr(state->Path()+strlen(path)+1, '/');
                if (nel == NULL)
                {
                    ASSERTX(strlen(state->Path()) < AWB_TCLBUF_SZ);
                    strcpy(prefix, state->Path());
                }
                else 
                {
                    ASSERTX((nel - state->Path() + 1) < AWB_TCLBUF_SZ);

                    strncpy(prefix, state->Path(), nel - state->Path());
                    prefix[nel-state->Path()] = '\0';
                }
                
                //
                // If the new longer prefix is different than the
                // last prefix we saw, then return it.

                if (strcasecmp(lastPath, prefix) != 0) 
                {
                    strcpy(lastPath, prefix);
                    os.str(""); // clear
                    os << "MODULE__" << prefix; 
                    Tcl_AppendElement(interp, const_cast<char*>(os.str().c_str()));
                }
            }
        }
        
        scan = scan->next;
    }

    return(SUCCESS);
#endif
}



string
PmStateFind (string sname) 
/*
 * Search the state list to find the state named 'name.  We do a
 * case-insensitive search. Return a descriptor for the matching state
 * or "" if can't find.
 */
{
    ASIM_STATELINK scan = PmStateList();
    while (scan != NULL) {
        ASIM_STATE state = scan->state;

        const string sn(state->Name());
        if (match_nocase(sname, sn))
        {
            ostringstream os;
            os << "STATE__" << fmt_p((void *)state) ;
            return os.str();
        }

        scan = scan->next;
    }
    string retval = "";
    return retval;
}


static list<string>
PmStateValue (char *desc)
/*
 * Return 'state's value.
 */
{
    list<string> ret;

    ASIM_STATE state = PmStateDescToPtr(desc);
    if (state != NULL) {
        //
        // Iterate through 'state's values, appending them
        // to the result list.

        switch (state->Type())
        {
          case STATE_UINT:
            for (UINT32 i = 0; i < state->Size(); i++)
            {
              ostringstream os;
              os << state->IntValue(i);
              ret.push_back(os.str());
            }
            break;
          case STATE_FP:
            for (UINT32 i = 0; i < state->Size(); i++)
            {
              ostringstream os;
              os << fmt("f", state->FpValue(i));
              ret.push_back(os.str());
            }
          case STATE_HISTOGRAM:
          case STATE_RESOURCE:
          case STATE_THREE_DIM_HISTOGRAM:
          default:
            ostringstream os;
            os << "unknown state type " << state->Type();
            ret.push_back(os.str());
        }

        return ret;
    }

    EMSG("null  state descriptor for 'value' \[" << desc << "]");
    myexit(1);
    // notreached
    return ret;
}

static void
CheckEvents()
{
    if (!runWithEventsOn)
    {
        EMSG("You are trying to generate events in a model not compiled "
             << "with events. Build the model with EVENTS=1");
        myexit(1);
    }
}

void PmEventOn()
{
    CheckEvents();
    DRALEVENT(TurnOn());
    eventsOn = true;
    if (firstTurnedOn)
    {
        DRALEVENT(StartActivity(ASIM_CLOCKABLE_CLASS::GetClockServer()->getFirstDomainCycle()));
        firstTurnedOn = false;
    }
}
void PmEventOff()
{
    CheckEvents();
    DRALEVENT(TurnOff());
    eventsOn = false;
}
void PmEventFilename(string& filename)
{
    CheckEvents();
    DRALEVENT(ChangeFileName(filename.c_str()));
}


void
PmStrip(string& onoff)
{
    if (onoff == "on")
    {
        stripsOn = true;
    }
    else
    {
        stripsOn = false;
    }
}    

void
PmTrace(string& onoff )
{
    if (onoff == "on")
    {
        traceOn = true;
    }
    else
    {
        traceOn = false;
    }
}    

//----------------------------------------------------------------------------

void
PmMarkerSet( string& desc,
             INT32 markerID,
             string& subcmd,
             string& arg1,
             string& arg2)
/*
 * interface to pm marker. Set / Clear markers
 */
{
    ASIM_THREAD thread = PmThreadDescToPtr(desc);
    if (thread == NULL) {
        EMSG("invalid thread descriptor [" << desc << "]");
        myexit(1);
    }

    
    TMSG(Trace_Cmd,"PmMarkerSet "
         << desc << " "
         << markerID << " "
         << subcmd << " "
         << arg1 << " "
         << arg2);


    prep();

    //
    // set        - requires thread, markerID, subcommand
    //              subcommand: pc requires address
    //              subcommand: inst requires bits, mask
    // syntax: PmMarker set <thread> <id> {("pc" <addr>) | "inst" <bits> <mask>}
        // subcommand parsing
        //
        // subcommand: pc <addr>
        if (subcmd == "pc")
        {

            UINT64 addr = atoi_general_unsigned(arg1);

            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_SET_PC,
                                      thread->IStreamHandle(),
                                      markerID,
                                      addr);
            return;
        }
        else if (subcmd == "inst") 
        {
            UINT32 bits = atoi_general_unsigned(arg1);
            UINT32 mask = atoi_general_unsigned(arg2);

            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_SET_INST,
                                      thread->IStreamHandle(),
                                      markerID,
                                      /*pc*/0,
                                      bits,
                                      mask);
            return;
        }
        //
        // unknown subcommand
        EMSG("PmMarkerSet: unknown subcommand [" << subcmd << "]"
             << ": must be pc, or inst");
        myexit(1);
}
    


void
PmMarkerClear( string& desc,
               INT32 markerID,
               string& subcmd,
               string& args)
/*
 * interface to pm marker. Clear markers
 */
{
    ASIM_THREAD thread = PmThreadDescToPtr(desc);
    if (thread == NULL) {
        EMSG("invalid thread descriptor [" << desc << "]");
        myexit(1);
    }

    
    TMSG(Trace_Cmd,"PmMarkerClear "
         << desc << " "
         << markerID << " "
         << subcmd << " "
         << args);


    prep();

    //
    // clear      - requires thread, markerID, subcommand
    //              subcommand: pc requires address
    //              subcommand: all
    // syntax: PmMarker clear <thread> <id> {("pc" <addr>) | "all"}
    //
        // subcommand parsing
        //
        // subcommand: pc <addr>
        if (subcmd == "pc")
        {

          UINT64 addr = atoi_general_unsigned(args);

            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_CLEAR_PC,
                                      thread->IStreamHandle(),
                                      markerID,
                                      addr);
            return;
        }
        //
        // subcommand: all
        else if (subcmd == "all")
        {
            thread->IFeeder()->Marker(IFEEDER_BASE_CLASS::MARKER_CLEAR_ALL,
                                      thread->IStreamHandle(),
                                      markerID);
            return;
        }
        //
        EMSG("PmMarkerClear: unknown subcommand [" << subcmd << "]"
             << ": must be pc, or all");
        myexit(1);
}        
    
//----------------------------------------------------------------------------

    
//----------------------------------------------------------------------------
void
PmScheduleStart()
{
    XMSG("PmScheduleStart");
    prep();
    CMD_Start();
}

void
PmScheduleStopNow()
{
    XMSG("PmScheduleStopNow");
    CMD_ACTIONTRIGGER action = ACTION_NOW;
    UINT64 stopTime = 0;
    CMD_Stop(action, stopTime);
}

void
PmScheduleStopNanosecond(UINT64 stopTime)
{
    XMSG("PmScheduleNanosecond");
    CMD_ACTIONTRIGGER action = ACTION_NANOSECOND_ONCE;
    CMD_Stop(action, stopTime);
}

void
PmScheduleStopCycle(UINT64 stopTime)
{
    XMSG("PmScheduleStopCycle");
    CMD_ACTIONTRIGGER action = ACTION_CYCLE_ONCE;
    CMD_Stop(action, stopTime);
}

void 
PmScheduleStopInst(UINT64 stopCount)
{
    XMSG("PmScheduleStopInst");
    CMD_ACTIONTRIGGER action = ACTION_INST_ONCE;
    CMD_Stop(action, stopCount);
}

void 
PmScheduleStopMacroInst(UINT64 stopCount)
{
    XMSG("PmScheduleStopMacroInst");
    CMD_ACTIONTRIGGER action = ACTION_MACROINST_ONCE;
    CMD_Stop(action, stopCount);
}

void
PmScheduleExitNow()
{
    prep();
    CMD_ACTIONTRIGGER action = ACTION_NOW;
    UINT64 time = 0;
    CMD_Exit(action, time);
    
}

void
PmScheduleProgress(string& type,
                   string& period)
{
    prep();
    // type = inst or cycle
    // period = clear or period

    CMD_ACTIONTRIGGER action;
    if ( type == "inst")
    {
        action = ACTION_INST_PERIOD;
    }
    else if (type == "nanosecond")
    {
        action = ACTION_NANOSECOND_PERIOD;
    }
    if ( type == "macroinst")
    {
        action = ACTION_MACROINST_PERIOD;
    }
    else if (type == "cycle")
    {
        action = ACTION_CYCLE_PERIOD;
    }
    else
    {
        EMSG("invalid action option [" << type << "]"
             << ": must be inst or cycle or nanosecond");
        myexit(1);
    }            
        
    if (period == "clear")
    {
        if (action == ACTION_CYCLE_PERIOD)
        {
            CMD_Progress(AWBPROG_CLEARCYCLE, "", ACTION_NOW);
        }
        else if (action == ACTION_NANOSECOND_PERIOD)
        {
            CMD_Progress(AWBPROG_CLEARNANOSECOND, "", ACTION_NOW);
        }
        else if (action == ACTION_MACROINST_PERIOD)
        {
            CMD_Progress(AWBPROG_CLEARMACROINST, "", ACTION_NOW);
        }
        else
        {
            CMD_Progress(AWBPROG_CLEARINST, "", ACTION_NOW);
        }
    }
    else if (action == ACTION_CYCLE_PERIOD)
    {
        INT32 p = atoi(period);
        XMSG("PmScheduleProgress cycle period " << p);
        CMD_Progress(AWBPROG_CYCLE, "", action, p);
    }
    else if (action == ACTION_NANOSECOND_PERIOD)
    {
        INT32 p = atoi(period);
        XMSG("PmScheduleProgress nanosecond period " << p);
        CMD_Progress(AWBPROG_NANOSECOND, "", action, p);
    }
    else if (action == ACTION_MACROINST_PERIOD)
    {
        INT32 p = atoi(period);
        XMSG("PmScheduleProgress macro inst period " << p);
        CMD_Progress(AWBPROG_MACROINST, "", action, p);
    }
    else
    {
        INT32 p = atoi(period);
        XMSG("PmScheduleProgress inst period " << p);
        CMD_Progress(AWBPROG_INST, "", action, p);
    }
}

void PmScheduleThread(string& desc,
                      string& trigger,
                      UINT64 time)
{

    XMSG("PmScheduleThread desc: " << desc
         << " trigger: " << trigger
         << " time: " << time);

    ASIM_THREAD thread = PmThreadDescToPtr(desc);
    if (thread == NULL) {
        EMSG("invalid thread descriptor [" << desc << "]");
        myexit(1);
    }
        
    CMD_ACTIONTRIGGER action;
    if (DecodeAction(trigger, action))
    {
        CMD_ScheduleThread(thread, action, time);
        return;
    }
    EMSG("Action [" << trigger << "] did not decode properly");
    myexit(1);
    
}

void PmUnscheduleThread(string& desc,
                        string& trigger,
                        UINT64 time)
{
    ASIM_THREAD thread = PmThreadDescToPtr(desc);
    if (thread == NULL) {
        EMSG("invalid thread descriptor [" << desc << "]");
        myexit(1);
    }
        
    CMD_ACTIONTRIGGER action;
    if (DecodeAction(trigger, action))
    {
        CMD_UnscheduleThread(thread, action, time);
        return;
    }
    EMSG("Action [" << action << "] did not decode properly");
    myexit(1);
}

void
PmScheduleSkipThread(string& arg_thread,
                     UINT64 insts,
                     INT32  markerID,
                     string& arg_action,
                     UINT64 time)
{
    prep();
    
    // If ?thread? is "all", then set thread to NULL to indicate that all
    // threads should be skipped.
    
    ASIM_THREAD thread = NULL;
    if (arg_thread == "all")
    {
        EMSG("There was a bug in the original TCL for skipping all threads. Not fixed yet.");
        myexit(1);
    }
    if (arg_thread != "all")
    {
       thread = PmThreadDescToPtr(arg_thread);
        if (thread == NULL)
        {
            EMSG("invalid thread descriptor [" << arg_thread << "]");
            myexit(1);
        }
    }
    
    CMD_ACTIONTRIGGER action;
    if (DecodeAction(arg_action, action))
    {
        CMD_SkipThread(thread, insts, markerID, action, time);
        return;
    }
    EMSG("Action [" << action << "] did not decode properly");
    myexit(1);
}
    

static ASIM_THREAD
PmThreadDescToPtr (const string& desc)
/*
 * Return pointer to the ASIM_THREAD object specified by 'desc'.
 * Return NULL on error.
 */
{
    ASIM_THREAD thread = NULL;
    if (sscanf(desc.c_str(), "THREAD__%p", &thread) != 1)
        return(NULL);

    return(thread);
}

static bool
DecodeActionTime (char *aStr,
                  char *tStr,
                  CMD_ACTIONTRIGGER *action,
                  UINT64 *time)
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

class ACTION_DECODER_CLASS
{
  public:
    string s;
    CMD_ACTIONTRIGGER action;
};

static ACTION_DECODER_CLASS
action_map[] = {
    { "now", ACTION_NOW },
    { "cycle_once", ACTION_CYCLE_ONCE },
    { "cycle_period", ACTION_CYCLE_PERIOD },
    { "nanosecond_once", ACTION_NANOSECOND_ONCE },
    { "nanosecond_period", ACTION_NANOSECOND_PERIOD },
    { "inst_once", ACTION_INST_ONCE },
    { "inst_period", ACTION_INST_PERIOD },
    { "macroinst_once", ACTION_MACROINST_ONCE },
    { "macroinst_period", ACTION_MACROINST_PERIOD },
    { "packet_once", ACTION_PACKET_ONCE },
    { "" , ACTION_NOW }
};

static bool
DecodeAction(string& s,
             CMD_ACTIONTRIGGER& action)
{
    int i=0;
    while(action_map[i].s != "")
    {
        //cerr << "Testing [" << s << "] vs [" << action_map[i].s << "]" << endl;
        if (action_map[i].s == s)
        {
            //cerr << "\t Matched" << endl;
            action = action_map[i].action;
            return true;
        }
        //cerr << "\t No match" << endl;
        i++;
    }
    return false;
}

UINT64 PmCycle()
{
    return asimSystem->SYS_Cycle();
}
UINT64 PmCycle(unsigned int cpunum)
{
    return asimSystem->SYS_Cycle(cpunum);
}
UINT64 PmCommitted(unsigned int cpunum)
{
    return asimSystem->SYS_CommittedInsts(cpunum);
}
UINT64 PmGlobalCommitted()
{
    return asimSystem->SYS_GlobalCommittedInsts();
}
unsigned int PmNumCpus()
{
    return asimSystem->NumCpus();
}
UINT64 PmCommittedMarkers()
{
    return asimSystem->SYS_CommittedMarkers();
}
void PmCommittedWatchMarker(INT32 markerID)
{
    asimSystem->SYS_CommitWatchMarker() = markerID;
}
void PmBeginDrain()
{
      asimSystem->SYS_beginDrain();
}
void PmEndDrain()
{
      asimSystem->SYS_endDrain();
}
UINT64 PmReceivedPackets()
{
      return asimSystem->SYS_ReceivedPkt();
}







//Local Variables:
//pref: "awb.h"
//End:
