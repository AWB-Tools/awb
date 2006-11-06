/*
 * Copyright (C) 1999 Compaq Computer Corporation
 * Copyright (C) 2004-2006 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @author David Goodwin
 * @brief
 */

// generic
#include <iostream>
#include <sstream>
#include <string>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/trace.h"
#include "asim/profile.h"

// ASIM public modules
#include "asim/provides/instfeeder_interface.h"
#include "asim/provides/controller.h"
#include "asim/provides/context_scheduler.h"

//PTV and stats
#include <signal.h>
#include <stdlib.h>
#include "pipe.h"
#include "stats.h"
#include "knob.h"

bool pipetrace_quiet_mode_on=true;
static Knob<int> histo("histo", "Create stats file with histogram output", true);
//Willy Perfchecker
static Knob<int> perf_max_msgs("perf_max_msgs", "Max num of perfchecker messages output in stats file", 100);
static Knob<int> perfchecker_rulemask("perfchecker_rulemask", "enable bitmask for: <erules> <rules> <implicit>", RULEMASK_ALL);
static Knob<int> perfchecker_window("perfchecker_window", "Window (in cycles) before records are forcibly considered closed for perfchecker evaluation", 20000);

/*
 * These are definitions used for the following special knobs to support
 * perfchecker module enabling/disabling in a consistent manner.
 */

class Knob_Perf_Module_Activate { /* dummy class */ };
template <>
class Knob<Knob_Perf_Module_Activate>: public Knob_Generic {
public:
        Knob(const char *name, const char *desc) : Knob_Generic(name, desc, false) { }
        virtual int parse(const char *s, int n) {
                perf_activate(s, 1);
                return 1;
        }
        virtual int print(char *s, int n) {
                if (n > 0)
                      *s = '\0';
                return 1;
        }
};
static Knob<Knob_Perf_Module_Activate> perf_activate_mods("perf_activate_mods", "Activate the named perfchecker modules");
class Knob_Perf_Module_Deactivate { /* dummy class */ };
template <>
class Knob<Knob_Perf_Module_Deactivate>: public Knob_Generic {
public:
        Knob(const char *name, const char *desc) : Knob_Generic(name, desc, false) { }
        virtual int parse(const char *s, int n) {
                perf_activate(s, 0);
                return 1;
        }
        virtual int print(char *s, int n) {
                if (n > 0)
                        *s = '\0';
                return 1;
        }
};
static Knob<Knob_Perf_Module_Deactivate> perf_deactivate_mods("perf_deactivate_mods", "Deactivate the named perfchecker modules");

static void
call_perf_activate_all(void)
{
        perf_activate_all(1);
}
static Knob<void> perf_activate_all_mods("perf_activate_all_mods", "Activate all perfchecker modules", call_perf_activate_all);

static void
call_perf_deactivate_all(void)
{
        perf_activate_all(0);
}
static Knob<void> perf_deactivate_all_mods("perf_deactivate_all_mods", "Deactivate all perfchecker modules", call_perf_deactivate_all);

static int perf_force = 0;
static void
set_perf_force(void)
{
        perf_force = 1;
}
static Knob<void> force("force", "Force perfchecker modules to ignore deactivation by missing pipetrace pattern matches", set_perf_force);

static int perf_explain = 0;
static void
set_perf_explain(void)
{
        perf_explain = 1;
}
static Knob<void> explain("explain", "Verbose perfchecker output for reasons modules enabled/disabled", set_perf_explain);

// Perfplot stuff ykulbak-s
Knob<char*>  perfplot_file("perfp_file", "Perfplot output file name", 0);
Knob<uint32> perfplot_interval("perfp_interval", "Perfplot firing interval", (uint32) 1000);
Knob<char*>  perfplot_stats("perfp_stats", "Perfplot stats to print", 0);
Knob<char*>  perfplot_firing_stat("perfp_firing_stat", "The stat by which Perfplot firing is triggered", 0);
Knob<uint32> perfplot_start_from_end("perfp_start_from_end" , "Start perfplot this many instructions before the end; 0 to disable", (uint32) 0);

FILE *perfplotfp;
bool perfplot_enabled = true;

// ykulbak-e

FILE *pclogfp;



using namespace std;

class CONTROL_TRACEABLE_CLASS : public TRACEABLE_CLASS
{
public:
    CONTROL_TRACEABLE_CLASS()
    {
        SetTraceableName("Control");
    }
};

CONTROL_TRACEABLE_CLASS controlTraceable;

#define ASIM_XMSG(x) \
({ \
       T1_AS(&controlTraceable, __FILE__ << ":" << __LINE__ << ": " <<  x); \
})
//#undef ASIM_XMSG
//#define ASIM_XMSG(x) do{ TTRACE(x); exit(1); }while(0)

#define DMSG(x) \
({ \
       T1_UNCOND(__FILE__ << ":" << __LINE__ << ": " <<  x); \
})

CMD_ACK PmProcessEvent (CMD_WORKITEM workItem);

/*
 * The system being simulated.
 */
ASIM_SYSTEM asimSystem;

/*
 * 'ctrlWorkList' contains new work items that the controller needs to
 * schedule.  The performance model and awb puts things on the list.
 */
static CMD_WORKLIST ctrlWorkList;

/*
 * Schedule of events that need to be performed.
 */
static CMD_SCHEDULE schedule;

/*
 * True if the performance model is stopped. When stopped time doesn't
 * advance, so no actions are performance.
 */
static volatile bool pmStopped = true;

/*
 * True if the performance model is exiting. The exit command has been
 * received and we are going through final cleanup operations.
 */
static volatile bool pmExiting = false;

/*
 * True if the performance model has been initialized.
 */
static bool pmInitialized = false;

// TraceFileName (in debugger mode)
char *StatsFileName = NULL; 
char *CheckPointFileName = NULL;
extern bool stripsOn;

/*Pipetrace and stats
 */
bool pipetrace_closed = false;
uint32 instrs_retired[MAX_THREADS*MAX_CORES*MAX_PROCS];

// Funtion called when an exit condition is reached
void
CheckExitConditions(EXIT_CONDITION ec)
{
    switch(ec)
    {
        case THREAD_END:
            if(STOP_THREAD==1)
                ctrlWorkList->Add(new CMD_EXIT_CLASS(ACTION_NOW,0));
            break;
        default: ;
    }
}

// SIGABRT handler
void abort_handler (int signum) 
{
      ASIM_XMSG("Signal:" << signum << " called dumping ptv and stats..." );
      signal(signum, SIG_DFL) ; // reset sig
      DumpStats(1) ;
}


//function for on_exit to use when dumping stats
void DumpStatsOnExit (int exitStatus, void *arg)
{
    DumpStats(exitStatus);
}


// Function to call onexit to dump ptv/stats.
void
DumpStats (int err_occurred)
/*
 * Dump PTV/stats
 */
{
    ASIM_XMSG("Dumpstats...");
    perf_end_perfchecker();
    if (perfchecker && !stats.empty() ) {
      fclose(pclogfp);
    }
    if (!pt.empty() && !pipetrace_closed)
      {
	pipe_close_file(PIPE_NO_RECORD);
	pipetrace_closed = 1;
      }   
    //Dump PTV Stats
    if(!stats.empty())
      {
	ofstream statfile(stats.c_str());
	if(histo){
	  stat_generic_write(statfile);
	}
	else{
	  stats_write(statfile);
	}
	statfile.close();
      }
    
     if (!pt_dump.empty() && pt.empty() && err_occurred) 
       {
	 pipe_dump_buffer(pt_dump.c_str(), 
			  (uint32)pipetrace_dump_max,
			  (pt_ascii ? PT_ASCII : PT_Binary));
       }

}

//Function to return the next token in the feeder config file.
string
getword(
    FILE *fp, 
    int *linenum)
{
/*
 * Parse command config file arguments, return the token on the specified linenum
 *
 */
    int c;
    string token;
    int junkchar = 1;
    while(junkchar)
    {
        junkchar=0;
        while ((c = getc(fp)) != EOF && isspace(c)); //eat spaces
        if ((c != EOF) && (c == '\n'))  
        {
            (*linenum)++;
            c = getc(fp);
        }
        if ((c != EOF) && (c == '#')) {
            while ((c = getc(fp)) != EOF && c != '\n');
            (*linenum)++;
            junkchar=1;
        } 
    }
    if (c != EOF) 
    {
        do {
            token += c;
        } while ((c = getc(fp)) != EOF && !isspace(c));
        if (c != EOF){ ungetc(c, fp);}
    }
    if (c == EOF && token.size() == 0)      // Don't barf if no last \n
        throw getword_error_dummy();
    return token;
}

void
CMD_Perfplot (CMD_ACTIONTRIGGER trigger, UINT64 n);

void Perfplot_Init() {
    // Perfplot Init
    if (perfplot_file) {
        CMD_ACTIONTRIGGER trigger = ACTION_NEVER;

        int pc =  (strcmp (perfplot_firing_stat.val(), "cycles") == 0);
        int pi =  (strcmp (perfplot_firing_stat.val(), "instrs_retired") == 0);

        if (!pc && ! pi) {
            fprintf(stderr, "Perfplot: Stat [%s] not supported, please use cycles or instrs_retired",
                    perfplot_firing_stat.val());  
            exit(EXIT_FAILURE);
        } else {
            if (pc) {
                trigger = ACTION_CYCLE_PERIOD;
            } else {
                trigger = ACTION_INST_PERIOD;
            }
        }

        perfplotfp = xfopen(perfplot_file, "w");
        if (!perfplotfp)
        {
            fprintf(stderr, "Perfplot: can't create %s\n", (char *) perfplot_file);  
            exit(EXIT_FAILURE);
        }

        if (perfplot_start_from_end > 0)
            perfplot_enabled = false;

        perf_start_perfplot(perfplotfp, perfplot_firing_stat, perfplot_interval, perfplot_stats);
        CMD_Perfplot(trigger, perfplot_interval);
    }

}

bool
CMD_Init (UINT32 fdArgc, char **fdArgv, UINT32 pmArgc, char **pmArgv, char **allEnvp)
/*
 * Initialize the performance model. Return false
 * if error.
 */
{
    ASIM_XMSG("CMD_Init...");

    //
    // Initially there is no system...

    asimSystem = NULL;
    
    //
    // Create controller work list.

    ctrlWorkList = new CMD_WORKLIST_CLASS;  
    
    //
    // Create the schedule...

    schedule = new CMD_SCHEDULE_CLASS; 		
    
    // Initialize the performance model
    CMD_ACK ack = PmProcessEvent(
        new CMD_INIT_CLASS(fdArgc, fdArgv, pmArgc, pmArgv, allEnvp));

    pmInitialized = ack->Success();
    delete ack->WorkItem();
    delete ack;
 
    // Trap signals to dump ptv etc
    signal(SIGABRT, abort_handler) ;
    signal(SIGINT, abort_handler) ;
    signal(SIGSEGV, abort_handler) ;

    //PTV init
    thread_initialize(60000); // deadlock detector count
    si_processors = Stat_Index(nproc*ncore, "p");
    si_cores = Stat_Index(ncore, "c");
    si_threads = Stat_Index(nthread, "t");
     
    pipe_threads_per_proc(nthread);
    pipe_init(pipetrace_dump_max);
#if !PTV_EVENTS
    pipe_quiet();
#endif
    //This has to be called after all the event types have been registered !!!
    stat_generic_init();

     //Perfchecker Init
    if (!stats.empty()){
      if (perfchecker) {
        string pclogfile(stats.c_str());
        string::size_type p = pclogfile.rfind(".gz");
        if (p == string::npos)
          p = pclogfile.length();
        pclogfile.insert(p, ".pclog");
        pclogfp = fopen(pclogfile.c_str(), "w");
        if (!pclogfp) {
          fprintf(stderr, "willy: can't create %s\n", pclogfile.c_str());  
          exit(EXIT_FAILURE);
        }
        perf_max_messages(perf_max_msgs);
        perf_start_perfchecker(pclogfp, perf_explain, perf_force,
                               perfchecker_rulemask, perfchecker_window);
      }
    }

    // initiate perfplot
    Perfplot_Init();

    on_exit(DumpStatsOnExit, NULL);
    return(pmInitialized);
}


void
CMD_Usage (FILE *file)
/*
 * Print the usage info for the feeder and system.
 */
{
    ASIM_XMSG("CMD_Usage...");

    //
    // This command function just calls the usage function directly.

    SYS_Usage(file);
    IFEEDER_Usage(file);
}



/*******************************************************************
 *                                                                  
 *  Most of the following routines add 'command' events to the 
 *  controller work list. They are processed by the scheduler.
 *
 ******************************************************************/


void
CMD_Start (void)
/*
 * Start the performance model running.
 */
{
    ASIM_XMSG("CMD_Start...");

    //
    // We have to clear 'pmStopped' ourselves, since the controller thread
    // can't (it's blocked since 'pmStopped' is true)

    pmStopped = false;

    //
    // Generate the start work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_START_CLASS);	
    asimSystem->SYS_Break();
}


void
CMD_Stop (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to stop the performance model.
 */
{
    ASIM_XMSG("CMD_Stop...");

    //
    // Generate the stop work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_STOP_CLASS(trigger, n));  
    asimSystem->SYS_Break();
}

void
CMD_Progress (AWB_PROGRESSTYPE type, char *args, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to show progress of the performance model.
 */
{
    ASIM_XMSG("CMD_Progress... adding worklist item");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    
    ctrlWorkList->Add(new CMD_PROGRESS_CLASS(type, args, trigger, n)); 
    asimSystem->SYS_Break();
}


void
CMD_Perfplot (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to show progress of the performance model.
 */
{
    ASIM_XMSG("CMD_Perfplot... adding worklist item");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    
    ctrlWorkList->Add(new CMD_PERFPLOT_CLASS(trigger, n)); 
    asimSystem->SYS_Break();
}


void
CMD_EmitStats (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to emit stats of the performance model.
 */
{
    ASIM_XMSG("CMD_EmitStats...");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    
    ctrlWorkList->Add(new CMD_EMITSTATS_CLASS(trigger, n)); 
    asimSystem->SYS_Break();
}

void
CMD_ResetStats (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to reset stats of the performance model.
 */
{
    ASIM_XMSG( "CMD_ResetStats...");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_RESETSTATS_CLASS(trigger, n)); 
    asimSystem->SYS_Break();
}
/*
 * Save a checkpoint file on the trigger
 */
void 
CMD_SaveCheckpoint (CMD_ACTIONTRIGGER trigger, UINT64 n)
{
    ASIM_XMSG( "CMD_SaveCheckPoint...");
    ctrlWorkList->Add(new CMD_DUMPCHECKPOINT_CLASS(trigger, n));
    asimSystem->SYS_Break();
}

/*
 * Load a checkpoint file on the trigger
 */
void 
CMD_LoadCheckpoint (CMD_ACTIONTRIGGER trigger, UINT64 n)
{
    ASIM_XMSG( "CMD_LoadCheckPoint...");
    ctrlWorkList->Add(new CMD_LOADCHECKPOINT_CLASS(trigger, n));
    asimSystem->SYS_Break();

}

void
CMD_Debug (bool on, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to turn 'on' or off debugging as specified by 'trigger'
 * and 'n'.
 */
{
    ASIM_XMSG("CMD_Debug...");
    
    //
    // Generate the debug work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_DEBUG_CLASS(on, trigger, n)); 
    asimSystem->SYS_Break();
}

void
CMD_Trace (TRACEABLE_DELAYED_ACTION act, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to turn 'on' or off debugging as specified by 'trigger'
 * and 'n'.
 */
{
    ASIM_XMSG("CMD_Trace...");

    //
    // Generate the debug work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_TRACE_CLASS(act, trigger, n));
    asimSystem->SYS_Break();
}

void
CMD_Stats (bool on, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to turn 'on' or off stats as specified by 'trigger'
 * and 'n'.
 */
{
    ASIM_XMSG("CMD_Stats...");
    
    //
    // Generate the debug work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    ctrlWorkList->Add(new CMD_STATS_CLASS(on, trigger, n)); 
    asimSystem->SYS_Break();
}

void
CMD_Ptv (bool on, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to turn 'on' or off ptv as specified by 'trigger'
 * and 'n'.
 */
{
    ASIM_XMSG("CMD_Ptv...");
    //
    // Generate the debug work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    ctrlWorkList->Add(new CMD_PTV_CLASS(on, trigger, n)); 
    asimSystem->SYS_Break();
}

void
CMD_Events (bool on, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to turn 'on' or off events as specified by 'trigger'
 * and 'n'.
 */
{
    ASIM_XMSG("CMD_Events...");
    
    //
    // Generate the debug work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_EVENTS_CLASS(on, trigger, n)); 
    asimSystem->SYS_Break();
}

void
CMD_Stripchart (bool on, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to turn 'on' or off stripcharts as specified by 'trigger'
 * and 'n'.
 */
{
    ASIM_XMSG("CMD_Stripchart...");
    //
    // Generate the debug work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_STRIPCHART_CLASS(on, trigger, n)); 
    asimSystem->SYS_Break();
}

void
CMD_Profile (bool on, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to turn 'on' or off profiling as specified by 'trigger'
 * and 'n'.
 */
{
    ASIM_XMSG("CMD_Profile...");
    
    //
    // Generate the profile work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_PROFILE_CLASS(on, trigger, n));
    asimSystem->SYS_Break();
}


ASIM_STATELINK
CMD_StateList (void)
/*
 * Return a list of all the exposed performance model state. This cannot be
 * called until the model has been initialized, or an error will occur.
 */
{
    if (!pmInitialized)
        ASIMERROR("CMD_StateList called on uninitialized performance model.\n");

    STATE_ITERATOR_CLASS iter(asimSystem, true);
    ASIM_STATELINK pmState = NULL;
    ASIM_STATE state;

    while ((state = iter.Next()) != NULL) {
        pmState = new ASIM_STATELINK_CLASS(state, pmState, false);
    }

    return(pmState);
}


void 
CMD_ThreadBegin (ASIM_THREAD thread)
/*
 * Called by feeder to inform that a new thread is available to be
 * schedule on the performance model.
 */
{
    ASIM_XMSG("CMD_ThreadBegin...");

    //
    // Generate the new-thread work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_THDBEGIN_CLASS(thread));
    if (asimSystem != NULL)
    {
        asimSystem->SYS_Break();
    }
}


void 
CMD_ThreadEnd (ASIM_THREAD thread)
/*
 * Called by feeder to inform that a thread is ending.
 */
{
    ASIM_XMSG("CMD_ThreadEnd...");

    //
    // Generate the thread-end work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    ctrlWorkList->Add(new CMD_THDEND_CLASS(thread));        
    asimSystem->SYS_Break();
}

void 
CMD_ThreadUnblock (ASIM_THREAD thread)
/*
 * Called by feeder to inform that a thread is available again to be
 * re-scheduled on the performance model.
 */
{
    ASIM_XMSG("CMD_ThreadUnblock...");

    //
    // Generate the thread-unblock work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_THDUNBLOCK_CLASS(thread));
    asimSystem->SYS_Break();
}


void 
CMD_ThreadBlock (ASIM_THREAD thread, ASIM_INST inst)
/*
 * Called by feeder to inform that a thread is blocking.
 */
{
    ASIM_XMSG("CMD_ThreadBlock...");

    //
    // Generate the thread-block work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_THDBLOCK_CLASS(thread, inst));
    asimSystem->SYS_Break();
}

void
CMD_SkipThread (ASIM_THREAD thread, UINT64 insts, INT32 markerID, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Skip 'insts' instructions in 'thread'.
 */
{
    ASIM_XMSG("CMD_SkipThread...");
    
    //
    // Generate the debug work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_THDSKIP_CLASS(thread, insts, markerID, trigger, n));
    asimSystem->SYS_Break();
}


void
CMD_ScheduleThread (ASIM_THREAD thread, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Schedule 'thread' on the performance model.
 */
{
    ASIM_XMSG("CMD_ThreadSchedule...");
    
    //
    // Generate the scheduling work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_THDSCHED_CLASS(thread, trigger, n));
    asimSystem->SYS_Break();
}


void
CMD_UnscheduleThread (ASIM_THREAD thread, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Unschedule 'thread' on the performance model.
 */
{
    ASIM_XMSG("CMD_ThreadUnschedule...");
    
    //
    // Generate the unscheduling work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_THDUNSCHED_CLASS(thread, trigger, n));
    asimSystem->SYS_Break();
}



void
CMD_Exit (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to exit the performance model.
 */
{
    ASIM_XMSG( "CMD_Exit... trigger " << trigger << " n=" << n );
    
    //
    // Generate the exit work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_EXIT_CLASS(trigger, n));
    asimSystem->SYS_Break();
}


/**********************************************************************/

void
CMD_SchedulerLoop (void)
/*
 * Controller scheduler loop. Maintains a list of the next "action" that
 * needs to be performed by the performance model or by the controller
 * itself and advances through the list by running the performance model
 * forward.
 */
{
    ASIM_XMSG("CMD_SchedulerLoop");
    while ( ! pmExiting )
    {
        
        const UINT64 currentCycle = asimSystem->SYS_Cycle();
        
        const UINT64 currentPacket = asimSystem->SYS_ReceivedPkt();
        
        const UINT64 currentNanosecond = asimSystem->SYS_Nanosecond();
        
        UINT64 currentInst = 0;
        for (UINT32 i = 0; i < asimSystem->NumCpus(); i++)
        {
            currentInst += asimSystem->SYS_CommittedInsts(i); 
        }
        
        UINT64 currentMacroInst = 0;
        for (UINT32 i = 0; i < asimSystem->NumCpus(); i++)
        {
            currentMacroInst += asimSystem->SYS_CommittedMacroInsts(i); 
        }
        ASIM_XMSG("CMD_SchedulerLoop called on cycle " << currentCycle << " nanosecond " << currentNanosecond);
        //
        // While there are things on the controller's worklist, remove them
        // and update the schedule. If the performance model is stopped,
        // then the controller thread stops here as well, until there is
        // new work placed on the worklist.

        while (ctrlWorkList->Head())
        {
            CMD_WORKITEM wItem = ctrlWorkList->Remove();
            wItem->Schedule(schedule, currentNanosecond, currentCycle, currentInst, currentMacroInst, currentPacket);
            ASIM_XMSG("CMD Scheduled item " << wItem->Name()
                 << ": current nanosecond = " << currentNanosecond
                 << ": current cycle = " << currentCycle
                 << ": current macro inst = " << currentMacroInst
                 << ", inst = " << currentInst );
        }

        //
        // Clear system's break flag. Any scheduled events that we process
        // below can set it again, causing the system to break execution.
        
        asimSystem->SYS_ClearBreak();
        
        //
        // Process any scheduled events that are ready to occur.

        CMD_WORKITEM sItem = schedule->ReadyEvent(currentNanosecond, currentCycle, currentInst, currentMacroInst, currentPacket);
        while (sItem != NULL)
        {
            //
            // Cause the performance model to perform the action
            
            ASIM_XMSG("CMD_SchedulerLoop -> PmProcessEvent: " << sItem->Name());
            CMD_ACK ack = PmProcessEvent(sItem);

            //
            // Perform any command action associated with 'sItem'.

            ASIM_XMSG("CMD CmdAction tobegin cycle " << currentCycle
                 << " event " << sItem->Name()
                 << " trigger " << sItem->Trigger()
                 << " actionTime " << sItem->ActionTime()
                 << " period " << sItem->Period()); 

            sItem->CmdAction();

            //
            // If 'sItem' is a periodic action, then reschedule
            // it. Otherwise delete it.

            if ((sItem->Trigger() == ACTION_INST_PERIOD) ||
                (sItem->Trigger() == ACTION_MACROINST_PERIOD) ||
                (sItem->Trigger() == ACTION_CYCLE_PERIOD) ||
                (sItem->Trigger() == ACTION_NANOSECOND_PERIOD))
            {
                sItem->Schedule(schedule, currentNanosecond, currentCycle, currentInst, currentMacroInst, currentPacket);
                ASIM_XMSG( "CMD Rescheduled periodic item " << sItem->Name()
                      << ": current nanosecond = " << currentNanosecond
                      << ": current cycle = " << currentCycle
                      << ", macro inst = " << currentMacroInst
                      << ", inst = " << currentInst );
            }
            else
            {
                delete sItem;
            }

            delete ack;

            sItem = schedule->ReadyEvent(currentNanosecond, currentCycle, currentInst, currentMacroInst, currentPacket);
        }

        //
        // Inform the workbench of all actions that just took place.

        ASIM_XMSG("CMD_SchedulerLoop -> AWB_InformProgress");
        AWB_InformProgress();
        ASIM_XMSG("CMD_SchedulerLoop -> past AWB_InformProgress");
        //
        // If the performance model is stopped, then don't allow it
        // to execute.

        if ( ! pmStopped && ! pmExiting )
        {
            //
            // Get the number of cycles to the next scheduled cycle event,
            // and the number of instructions until the next schedule
            // instruction event.

            UINT64 nNanosecond = schedule->NanosecondForNextEvent(currentNanosecond);
            UINT64 nCycle = schedule->CycleForNextEvent(currentCycle);
            UINT64 nInst = schedule->InstForNextEvent(currentInst);
            UINT64 nMacroInst = schedule->MacroInstForNextEvent(currentMacroInst);

            UINT64 nPacket = schedule->PacketForNextEvent(currentPacket);            
            ASSERTX((nCycle != currentCycle) && (nInst != currentInst)&& (nMacroInst != currentMacroInst) && (nPacket != currentPacket) && (nNanosecond != currentNanosecond));
            //
            // Instruct the performance model to execute until cycle
            // 'nCycle' or until 'nInst' instructions are committed. The
            // performance model can ack us before either of those
            // conditions are met only if it is stopped with SYS_Break().

            ASIM_XMSG("CMD_SchedulerLoop -> PmProcessEvent CMD_EXECUTE_CLASS");
            CMD_ACK ack = PmProcessEvent(
                new CMD_EXECUTE_CLASS(nNanosecond, nCycle, nInst, nMacroInst, nPacket));

            delete ack->WorkItem();
            delete ack;
        }
    }

    // now this is a bit peculiar, but we perform the cleanup operations
    // for CMD* here, since this is the last piece of CMD* code that is
    // running before exit; this is essentially cleaning up what CMD_Init
    // has set up
    
    ASIM_XMSG("CMD_SchedulerLoop exiting ... calling AWB_Exit");

    //Stop the threads
    //asimSystem->SYS_StopPThreads();

    // Stop the awb workbench
    AWB_Exit();

    // print "AtExit" stats
    if (StatsFileName)
    {
        // create a STATE_OUT object for the stats file
        STATE_OUT stateOut = new STATE_OUT_CLASS(StatsFileName);
        
        if (! stateOut) 
        {
            ASIMERROR("Unable to create stats output file \"" <<StatsFileName
                      << "\", " << strerror(errno));
        }

        //output the module stats and the feeder stats to the stateout 
        asimSystem->PrintModuleStats(stateOut); 
        IFEEDER_BASE_CLASS::DumpAllFeederStats(stateOut);

        if (stateOut)
        {
           // dump stats to file and delete object
           delete stateOut;
        }
    }

    //cleanup feeder(s)
    IFEEDER_BASE_CLASS::AllDone();
    IFEEDER_BASE_CLASS::DeleteAllFeeders();
    delete asimSystem;

    // delete controller work list
    if (ctrlWorkList)
    {
        delete ctrlWorkList;
    }

    // delete the schedule
    if (schedule)
    {
        delete schedule;
    }
    
    // we just un-initialized it
    pmInitialized = false;
}


/*******************************************************************
 *
 * Command thread actions
 *
 ******************************************************************/

void
CMD_START_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_START_CLASS::CmdAction");
    pmStopped = false;
    AWB_Progress(AWBPROG_START);
}


void
CMD_STOP_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_STOP_CLASS::CmdAction");
    pmStopped = true;
    AWB_Progress(AWBPROG_STOP);
}


void
CMD_PROGRESS_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_PROGRESS_CLASS::CmdAction");
    //
    // If 'type' is a clearing progress action, then we don't perform any
    // action for it. It's action takes place at schedule time.

    if ((type != AWBPROG_CLEARCYCLE) && (type != AWBPROG_CLEARINST) && (type != AWBPROG_CLEARNANOSECOND) && (type != AWBPROG_CLEARPACKET)) {
        AWB_Progress(type, args);
    }
}


void
CMD_PERFPLOT_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_PERFPLOT_CLASS::CmdAction -> fire_perfplot_rules");
    fire_perfplot_rules();
}


void
CMD_EMITSTATS_CLASS::CmdAction (void)
{
    ostringstream statsFileName;

    UINT64 currentInst = 0;
    for (UINT32 i = 0; i < asimSystem->NumCpus(); i++)
    {
        currentInst += asimSystem->SYS_CommittedInsts(i); 
    }

    if(WILLY_STYLE_STATS)
    {
	assert(!stats.empty());
	string filename = stats.c_str();
	stringstream f1; 
	f1 << currentInst; 
	filename = filename + "." + f1.str() + ".stats";
	ofstream small_stat_file(filename.c_str());
	stats_write(small_stat_file);
	small_stat_file.close();
    }
    else
    {	
    statsFileName.str("");      // Clear
    statsFileName << "cycle_"  << asimSystem->SYS_Cycle()
                  << "_nano_"  << asimSystem->SYS_Nanosecond()
                  << "_insts_" << currentInst
                  << ".stats";
    
    ASIM_XMSG("CMD_EMITSTATS emitting intermediate stats: " << statsFileName.str());


    STATE_OUT stateOut = new STATE_OUT_CLASS(statsFileName.str().c_str());
    if (! stateOut) 
    {
        ASIMERROR("Unable to create stats output file \"" <<
                  statsFileName.str() << "\", " << strerror(errno));
    }

    asimSystem->PrintModuleStats(stateOut); 
    IFEEDER_BASE_CLASS::DumpAllFeederStats(stateOut);
    delete stateOut;
  
   }
}
void
CMD_LOADCHECKPOINT_CLASS::CmdAction (void)
{
//    ASSERT(0,"Load check point not finished");
     IFEEDER_BASE_CLASS::LoadAllFeederState(CheckPointFileName,FUNCTIONAL_MODEL_DUMP_ALL_STATE);

}
void
CMD_DUMPCHECKPOINT_CLASS::CmdAction (void)
{
    IFEEDER_BASE_CLASS::DumpAllFeederState(CheckPointFileName,FUNCTIONAL_MODEL_DUMP_ALL_STATE);

}

void
CMD_RESETSTATS_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_RESETSTATS resetting all stats");

    if(WILLY_STYLE_STATS)
    {
       stat_generic_clear();
    }
    else
    {
       asimSystem->ClearModuleStats(); 
       IFEEDER_BASE_CLASS::ClearAllFeederStats();
    }
}


void
CMD_THDBEGIN_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_THDBEGIN_CLASS::CmdAction");
    ostringstream os;
    if (thread)
    {
        os << "{" << thread->TUid() << " THREAD__" << fmt_p((void*)thread) << "}";
    }
    AWB_Progress(AWBPROG_THREADBEGIN, os.str().c_str());
}


void
CMD_THDEND_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_THDEND_CLASS::CmdAction");
    ostringstream os;
    if (thread)
    {
        os << "{" << thread->TUid() << " THREAD__" << fmt_p((void*)thread) << "}";
    }
    else
    {
        os << "{ THREAD__NULL }";
    }
    AWB_Progress(AWBPROG_THREADEND, os.str().c_str());
}

void
CMD_THDUNBLOCK_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_THDUNBLOCK_CLASS::CmdAction");
    ostringstream os;
    os << "{" << thread->TUid() << " THREAD__" << fmt_p((void*)thread) << "}";
    AWB_Progress(AWBPROG_THREADUNBLOCK, os.str().c_str());
}


void
CMD_THDBLOCK_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_THDBLOCK_CLASS::CmdAction");
    ostringstream os;
    os << "{" << thread->TUid() << " THREAD__" << fmt_p((void*)thread) << "}";
    AWB_Progress(AWBPROG_THREADBLOCK, os.str().c_str());
}

void
CMD_EXIT_CLASS::CmdAction (void)
{    
    ASIM_XMSG("CMD Exiting performance model...");

    // indicate to command processing loop to exit
    pmExiting = true;
}


/*******************************************************************
 *
 *
 *
 *******************************************************************/

void
CMD_PROGRESS_CLASS::Schedule (CMD_SCHEDULE schedule, UINT64 currentNanosecond, UINT64 currentCycle, UINT64 currentInst, UINT64 currentMacroInst, UINT64 currentPacket)
{
    ASIM_XMSG("CMD_PROGRESS_CLASS::Schedule");
    //
    // If 'type' is a clearing progress action, then we clear
    // progress events from the schedule.

    if (type == AWBPROG_CLEARNANOSECOND) {
        schedule->ClearNanosecondProgress();
    }
    else if (type == AWBPROG_CLEARCYCLE) {
        schedule->ClearCycleProgress();
    }
    else if (type == AWBPROG_CLEARINST) {
        schedule->ClearInstProgress();
    }
    else if (type == AWBPROG_CLEARMACROINST) {
        schedule->ClearMacroInstProgress();
    }
    else if (type == AWBPROG_CLEARPACKET) {
        schedule->ClearPacketProgress();
    }
    else {
        schedule->Schedule(this, currentNanosecond, currentCycle, currentInst, currentMacroInst, currentPacket);
    }
}

/*******************************************************************
 *
 * This is the processing for the performance model itself.
 *
 ******************************************************************/


CMD_ACK
PmProcessEvent (CMD_WORKITEM workItem)
/*
 * Performance model processes the event and returns the status
 */
{
    ASIM_XMSG("PmProcessEvent");
    fflush(stdout);
    fflush(stderr);

    CMD_ACK ack = workItem->PmAction();
    
    return (ack);
}

/*******************************************************************
 *
 * Performance model thread actions
 *
 ******************************************************************/


CMD_ACK
CMD_INIT_CLASS::PmAction (void)
/*
 * Initialize the performance model by initializing the instruction feeder
 * and the system. If either fails to initialize correctly, indicate
 * failure for this work item.
 */
{
    ASIM_XMSG("CMD PmAction INIT: Initializing performance model...");

    IFEEDER_BASE feeder = IFEEDER_New();
    bool success = feeder->Init(fdArgc, fdArgv, Envp);
    
    if (success)
    {
        asimSystem = SYS_Init(pmArgc, pmArgv, Envp, feeder->NActiveThreads());
        success = (asimSystem != NULL);
    }

    if (success)
    {
      //Read in -cfg <file> if specified and expand out all the knobs contained inside the file.
      for (UINT32 i = 0; i < fdArgc; i++) 
        {
          if (strcmp(fdArgv[i],"-cfg")==0)
            {
              ASSERTX(i+1 < fdArgc);
              i++;
              //Copy fdArgv into fdArgvCfg with additional space to put in arguments from cfg file.
              char **fdArgvCfg = new char * [fdArgc + CFG_ARGS_MAX];
              int fdArgcCfg = 0;
              for (UINT32 j = 0; j < fdArgc; j++)
                {
                  fdArgvCfg[j] = fdArgv[j];
                  fdArgcCfg++;
                }
              FILE *fp;
              int linenum = 1; 
              string fname = fdArgv[i];
              string knob_name;
              if ((fp = fopen(fname.c_str(), "r")) == NULL) 
                {
                  cout<<"Can't open feeder config file: "<<fname<<endl;
                  return false;
                }
              try {
                for (;;)
                  {
                    knob_name = ""; // Notice this if getword throws error
                    knob_name = getword(fp, &linenum);
                    fdArgvCfg[fdArgcCfg] = strdup(knob_name.c_str()); 
                    fdArgcCfg++;
                  }
                }
              catch (getword_error_dummy) {} 
              fclose(fp);
              //Store the all the original args and new args from cfg file back into argv, update argc.
              delete [] fdArgv;
              fdArgv = fdArgvCfg;
              fdArgc = fdArgcCfg; 
          } 
        }

    }

    ASIM_XMSG("CMD PmAction INIT complete: "
         << (success ? "SUCCESS" : "FAILURE") );

    if(printTraceNames) {
        TRACEABLE_CLASS::PrintNames();
        cout << endl << "-listnames given, printed traceable objects." << endl;
        exit(0);
    }

    return(new CMD_ACK_CLASS(this, success));
}


CMD_ACK
CMD_DEBUG_CLASS::PmAction (void)
/*
 * We should provide an interface here to allow the feeder and performance
 * model to activate/deactivate whatever debugging facilities they want
 * to. For now we just turn the trace on or off.
 */
{
    ASIM_XMSG("CMD PmAction DEBUG: Turning trace " << (on ? "on" : "off")
         << "..." );
    traceOn = on; 
    return(new CMD_ACK_CLASS(this, true));
}

CMD_ACK
CMD_TRACE_CLASS::PmAction (void)
{
    ASIM_XMSG("CMD PmAction TRACE: Enabling delayed traces...");
    if(regex) {
        regex->go();
    }
    return(new CMD_ACK_CLASS(this, true));
}

CMD_ACK
CMD_STATS_CLASS::PmAction (void)
{
    ASIM_XMSG("CMD PmAction STATS: Turning stats " << (on ? "on" : "off")
             << "..." );

    statsOn = on; 

    STATE_ITERATOR_CLASS iter(asimSystem, true);
    ASIM_STATE state;
   
    if(statsOn)
    {
        while ((state = iter.Next()) != NULL) {
            state->Unsuspend();
        }
    }
    else
    {
        while ((state = iter.Next()) != NULL) {
            state->Suspend();
        }
    }
    return(new CMD_ACK_CLASS(this, true));
}

CMD_ACK
CMD_PTV_CLASS::PmAction (void)
{
    ASIM_XMSG("CMD PmAction PTV: Turning ptv " << (on ? "on" : "off")
             << "..." );   
    if(on)
    {
#if !PTV_EVENTS
      pipe_enable_collection();
      pipetrace_quiet_mode_on=false;
#endif
      if(!pt.empty())
	{
	  pipe_open_file(pt.c_str(),
			 (pt_ascii ? PT_ASCII : PT_Binary),
			 PIPE_NO_RECORD);
	  pipetrace_closed = 0;
	}
    }
    else
    {
#if !PTV_EVENTS
        pipe_quiet();
        pipetrace_quiet_mode_on=true;
#endif
	if (!pt.empty() && !pipetrace_closed)
	  {
	    pipe_close_file(PIPE_NO_RECORD);
	    pipetrace_closed = 1;
	  }   
    }
    return(new CMD_ACK_CLASS(this, true));
}


CMD_ACK
CMD_EVENTS_CLASS::PmAction (void)
{
    ASIM_XMSG("CMD PmAction EVENTS: Turning events " << (on ? "on" : "off")
             << "...");
    eventsOn = on;
    if (on)
    {
        DRALEVENT(TurnOn());
        if (firstTurnedOn)
        {
            firstTurnedOn = false;
            DRALEVENT(StartActivity(ASIM_CLOCKABLE_CLASS::GetClockServer()->getFirstDomainCycle()));
        }
        ASIM_CLOCKABLE_CLASS::GetClockServer()->DralTurnOn();
    }
    else
    {
        DRALEVENT(TurnOff());
    }

    return(new CMD_ACK_CLASS(this, true));
}

CMD_ACK
CMD_STRIPCHART_CLASS::PmAction (void)
{
    ASIM_XMSG("CMD PmAction STRIPCHART: Turning stripchart "
             << (on ? "on" : "off") << "...");
    stripsOn = on; 
    return(new CMD_ACK_CLASS(this, true));
}


CMD_ACK
CMD_PROFILE_CLASS::PmAction (void)
{
    ASIM_XMSG("CMD PmAction PROFILE: Turning profile " << (on ? "on" : "off")
             << "..." );
    profileOn = on; 
    return(new CMD_ACK_CLASS(this, true));
}


CMD_ACK
CMD_EXECUTE_CLASS::PmAction (void)
/*
 * Execute for until 'cycle' or committed 'inst'.
 */
{
    ASIM_XMSG("CMD PmAction Execute: until cycle " << cycles
             << " or inst " << insts << " or nanosecond " << nanoseconds);
    bool success = asimSystem->SYS_Execute(nanoseconds, cycles, insts, macroinsts, packets);
    return(new CMD_ACK_CLASS(this, success));
}


CMD_ACK
CMD_THDSKIP_CLASS::PmAction (void)
/*
 * Skip the feeder...
 */
{
    ASIM_XMSG("CMD PmAction THDSKIP: Skipping " << insts
             << " insts or until marker " << markerID
             << "..." ); 

    UINT64 actual = thread->IFeeder()->Skip(thread->IStreamHandle(),
                                            insts,
                                            markerID);
    if (actual != insts && markerID < 0)
        ASIMWARNING("CMD_THDSKIP for " << insts
            << "... only skipped " << actual << endl);
    return(new CMD_ACK_CLASS(this, true));
}


CMD_ACK
CMD_THDSCHED_CLASS::PmAction (void)
/*
 * Schedule the 'thread'
 */
{
    ASIM_XMSG("CMD PmAction THDSCHED: Scheduling...");
    bool success = asimSystem->SYS_ScheduleThread(thread);
    return(new CMD_ACK_CLASS(this, success));
}


CMD_ACK
CMD_THDUNSCHED_CLASS::PmAction (void)
/*
 * Unschedule the 'thread'
 */
{
    ASIM_XMSG("CMD PmAction THDUNSCHED: Unscheduling...");
    bool success = asimSystem->SYS_UnscheduleThread(thread);
    return(new CMD_ACK_CLASS(this, success));
}

CMD_ACK
CMD_THDUNBLOCK_CLASS::PmAction (void)
/*
 * Unblock the 'thread'
 */
{
    ASIM_XMSG("PmAction THDUNBLOCK: Un-blocking...");
    bool success = asimSystem->SYS_UnblockThread(thread);
    return(new CMD_ACK_CLASS(this, success));
}


CMD_ACK
CMD_THDBLOCK_CLASS::PmAction (void)
/*
 * Block the 'thread'
 */
{
    ASIM_XMSG("PmAction THDBLOCK: Blocking..." );
    bool success = asimSystem->SYS_BlockThread(thread, inst);
    return(new CMD_ACK_CLASS(this, success));
}


