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
 * @author David Goodwin, Carl Beckmann
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
// we need this so we can statically know the type of "theController"
// in the API functions declared using CONTROLLER_BASE_EXTERNAL_FUNCTION
#include "asim/provides/controller_alg.h"

//PTV and stats
#include <signal.h>
#include <stdlib.h>
#include "pipe.h"
#include "stats.h"
#include "knob.h"

bool pipetrace_quiet_mode_on=true;
Knob<bool> histo("histo", "Create stats file with histogram output", true);
//Willy Perfchecker
Knob<int> perf_max_msgs("perf_max_msgs", "Max num of perfchecker messages output in stats file", 100);
Knob<int> perfchecker_rulemask("perfchecker_rulemask", "enable bitmask for: <erules> <rules> <implicit>", RULEMASK_ALL);
Knob<int> perfchecker_window("perfchecker_window", "Window (in cycles) before records are forcibly considered closed for perfchecker evaluation", 20000);

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
Knob<void> perf_activate_all_mods("perf_activate_all_mods", "Activate all perfchecker modules", call_perf_activate_all);

static void
call_perf_deactivate_all(void)
{
        perf_activate_all(0);
}
Knob<void> perf_deactivate_all_mods("perf_deactivate_all_mods", "Deactivate all perfchecker modules", call_perf_deactivate_all);

void set_perf_force(void)
{
        theController.perf_force = 1;
}
Knob<void> force("force", "Force perfchecker modules to ignore deactivation by missing pipetrace pattern matches", set_perf_force);

void set_perf_explain(void)
{
        theController.perf_explain = 1;
}
Knob<void> explain("explain", "Verbose perfchecker output for reasons modules enabled/disabled", set_perf_explain);

// Perfplot stuff ykulbak-s
Knob<char*>  perfplot_file("perfp_file", "Perfplot output file name", 0);
Knob<uint32> perfplot_interval("perfp_interval", "Perfplot firing interval", (uint32) 1000);
Knob<char*>  perfplot_stats("perfp_stats", "Perfplot stats to print", 0);
Knob<char*>  perfplot_firing_stat("perfp_firing_stat", "The stat by which Perfplot firing is triggered", 0);
Knob<uint32> perfplot_start_from_end("perfp_start_from_end" , "Start perfplot this many instructions before the end; 0 to disable", (uint32) 0);

static FILE *perfplotfp;
static bool perfplot_enabled = true;

// ykulbak-e

static FILE *pclogfp;


/*Pipetrace and stats
 */
bool pipetrace_closed = false;
uint32 instrs_retired[MAX_THREADS*MAX_CORES*MAX_PROCS];


// SIGABRT handler
static void abort_handler (int signum) 
{
      ASIM_XMSG("Signal:" << signum << " called dumping ptv and stats..." );
      signal(signum, SIG_DFL) ; // reset sig
      theController.DumpStats(1) ;
}


//function for on_exit to use when dumping stats
static void DumpStatsOnExit (int exitStatus, void *arg)
{
    theController.DumpStats(exitStatus);
}


// Function to call onexit to dump ptv/stats.
void
CONTROLLER_X86_CLASS::DumpStats (int err_occurred)
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
CONTROLLER_X86_CLASS::getword(
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

void CONTROLLER_X86_CLASS::Perfplot_Init() {
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
CONTROLLER_X86_CLASS::CMD_Init (UINT32 fdArgc, char **fdArgv, UINT32 pmArgc, char **pmArgv, char **allEnvp)
/*
 * Initialize the performance model. Return false
 * if error.
 */
{
    // invoke base class CMD_Init routine
    theController.CONTROLLER_CLASS::CMD_Init(fdArgc, fdArgv, pmArgc, pmArgv, allEnvp);
 
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
CONTROLLER_X86_CLASS::CMD_Progress (AWB_PROGRESSTYPE type, char *args, CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to show progress of the performance model.
 */
{
    ASIM_XMSG("CMD_Progress... adding worklist item");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    
    ctrlWorkList->Add(new CMD_PROGRESS_X86_CLASS(type, args, trigger, n)); 
    asimSystem->SYS_Break();
}


void
CONTROLLER_X86_CLASS::CMD_Perfplot (CMD_ACTIONTRIGGER trigger, UINT64 n)
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
CONTROLLER_X86_CLASS::CMD_EmitStats (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to emit stats of the performance model.
 */
{
    ASIM_XMSG("CMD_EmitStats...");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    
    ctrlWorkList->Add(new CMD_EMITSTATS_X86_CLASS(trigger, n)); 
    asimSystem->SYS_Break();
}

void
CONTROLLER_X86_CLASS::CMD_ResetStats (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to reset stats of the performance model.
 */
{
    ASIM_XMSG( "CMD_ResetStats...");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.

    ctrlWorkList->Add(new CMD_RESETSTATS_X86_CLASS(trigger, n)); 
    asimSystem->SYS_Break();
}

/*
 * Save a checkpoint file on the trigger
 */
void 
CONTROLLER_X86_CLASS::CMD_SaveCheckpoint (CMD_ACTIONTRIGGER trigger, UINT64 n)
{
    ASIM_XMSG( "CMD_SaveCheckPoint...");
    ctrlWorkList->Add(new CMD_DUMPCHECKPOINT_CLASS(trigger, n));
    asimSystem->SYS_Break();
}

/*
 * Load a checkpoint file on the trigger
 */
void 
CONTROLLER_X86_CLASS::CMD_LoadCheckpoint (CMD_ACTIONTRIGGER trigger, UINT64 n)
{
    ASIM_XMSG( "CMD_LoadCheckPoint...");
    ctrlWorkList->Add(new CMD_LOADCHECKPOINT_CLASS(trigger, n));
    asimSystem->SYS_Break();

}

void
CONTROLLER_X86_CLASS::CMD_Ptv (bool on, CMD_ACTIONTRIGGER trigger, UINT64 n)
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

/**********************************************************************/

void
CONTROLLER_X86_CLASS::CMD_SchedulerLoop (void)
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
CMD_PROGRESS_X86_CLASS::CmdAction (void)
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
//    fire_perfplot_rules();
    perf_fire_perfplot();
}


void
CMD_EMITSTATS_X86_CLASS::CmdAction (void)
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
     IFEEDER_BASE_CLASS::LoadAllFeederState(theController.CheckPointFileName,FUNCTIONAL_MODEL_DUMP_ALL_STATE);

}
void
CMD_DUMPCHECKPOINT_CLASS::CmdAction (void)
{
    IFEEDER_BASE_CLASS::DumpAllFeederState(theController.CheckPointFileName,FUNCTIONAL_MODEL_DUMP_ALL_STATE);

}

void
CMD_RESETSTATS_X86_CLASS::CmdAction (void)
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


/*******************************************************************
 *
 * Performance model thread actions
 *
 ******************************************************************/


CMD_ACK
CMD_INIT_X86_CLASS::PmAction (void)
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
                    knob_name = theController.getword(fp, &linenum);
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
CMD_EXECUTE_X86_CLASS::PmAction (void)
/*
 * Execute for until 'cycle' or committed 'inst'.
 */
{
    ASIM_XMSG("CMD PmAction Execute: until cycle " << cycles
             << " or inst " << insts << " or nanosecond " << nanoseconds);
    bool success = asimSystem->SYS_Execute(nanoseconds, cycles, insts, macroinsts, packets);
    return(new CMD_ACK_CLASS(this, success));
}
