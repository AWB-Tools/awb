/*
 *Copyright (C) 1999-2006 Intel Corporation
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
 * @author David Goodwin, Alexey Klimkin, Carl Beckmann
 * @brief
 */

// generic
#include <iostream>
#include <sstream>

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
#include "pipe.h"
#include "stats.h"
#include "knob.h"

class ARCHLIB_CLOCK_CTRL_CLASS : public ASIM_CLOCKABLE_CLASS
{
    public:
        ARCHLIB_CLOCK_CTRL_CLASS (const char *domain) : ASIM_CLOCKABLE_CLASS(0) {
            RegisterClock(domain);
        }

    void Clock(UINT64 cycle) {
        thread_sleep_until(cycle, thread_priority);

        // fire perfplot, this routine checks firing stat by itself
        fire_perfplot_rules();
        // process stat knobs
        // process ptv knobs
    }
    const char *ProfileId(void) const { return "ARCHLIB_CLOCK_CTRL"; }
};
typedef ARCHLIB_CLOCK_CTRL_CLASS* ARCHLIB_CLOCK_CTRL;

static bool _ArchlibSupport_Done = false;
// Perfchecker ykulbak-e
static FILE *pclogfp = 0;
static FILE *perfplotfp = 0;

Knob<bool> perfchecker          ("perfchecker", "Do run perfchecker", false);
Knob<int>  perfchecker_max_msgs ("perfchecker_max_msgs", "Max num of perfchecker messages output in stats file", 100);

Knob<char*>  perfplot_firing("perfp_firing", "The stat by which Perfplot firing is triggered", 0);

// PTV
bool pipetrace_open = false;
#define PT_FORMAT (pipetrace_ascii ? PT_ASCII : PT_Binary)
Knob<char*>  pipetrace_yes_file  ("pt_yes"        , "pipetrace Yes! file name", 0);
Knob<char*>  pipetrace_file      ("pt"            , "pipetrace file name", 0);
Knob<bool>   pipetrace_ascii     ("pt_ascii"      , "pipetrace in textual format", false);
Knob<char*>  pipetrace_dump_file ("pt_dump"       , "pipetrace dump file name", 0);
Knob<uint32> pipetrace_dump_max  ("pt_dump_max"   , "pipetrace dump maximum size", 2000U);
Knob<bool>   pipetrace_dump_passing  ("pt_dump_passing", "generate pt file even for passing traces", false);
//Knob<uint32> pipe_start_from_end ("pt_start_from_end"  , "Start pipetraces at retirement of specified instruction from end", 0U);
//Knob<uint32> pipe_start_inst     ("pt_start_inst" , "Start pipetraces at retirement of specified instruction", 0U);
Pipe_Recordtype *prt_start         = pipe_new_recordtype("Welcome", "Watch Window warmup hack");

// Stats
ARCHLIB_CLOCK_CTRL archlibClockCtrl = 0;
extern uint32 instrs_retired[MAX_PROCS * MAX_CORES * MAX_THREADS];
//Knob<uint> clear_stats( "clear_stats", "reset stat vals at insn count", 0U);
Knob<uint32> nproc   ("nprocs"  , "number of processors", 1);
Knob<uint32> ncore   ("ncores"  , "number of cores"     , 1);
Knob<uint32> nthread ("nthreads", "number of threads"   , 1);
Knob<char*>  stats_file ("stats", "stats file name", 0);

// Knobs
static void
verbose_help()
{
    knob_dump_help(stdout);
    exit(EXIT_SUCCESS);
}
Knob<bool> dump_knobs("dump_knobs", "dump all knob values", false);
Knob<bool> dump_stats("dump_stats", "dump all stat names", false);
Knob<void> minus_help("help", "display verbose help info", verbose_help);


/*
 * CMD_ARCHSIM_SUPPORT
 *
 * Per cycle archsim support library maintenance.
 * FIXME: check if clocking with clockserver works faster
 */
typedef class CMD_ARCHSIM_SUPPORT_CLASS *CMD_ARCHSIM_SUPPORT;
class CMD_ARCHSIM_SUPPORT_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_ARCHSIM_SUPPORT_CLASS () : CMD_WORKITEM_CLASS("ARCHSIM_SUPPORT",
                                                          ACTION_CYCLE_PERIOD, 1) {
            // Do nothing
        }

        CMD_ACK PmAction (void) {
            // FIXME: assuming everything is running on the same cycle
            return(new CMD_ACK_CLASS(this, true));
        }

};

void
ArchlibSupport_SetThreads(uint32 np, uint32 nc, uint32 nt)
{
    pipe_threads_per_proc(nt);
    pipe_threads_total(np * nc * nt);

    // adklimki FIXME: cores will make sense with new version of support/pipe.cc, 
    // before - use processors in sense of cores
    np *= nc;
    nc = np;

    char snp[8], snc[8], snt[8];
    const char *thr_knobs[] = {
        "-nprocs", snp,
        "-ncores", snc,
        "-nthreads", snt,
        0
    };
    snprintf(snp, 8, "%d", np);
    snprintf(snc, 8, "%d", nc);
    snprintf(snt, 8, "%d", nt);

    knob_parse_argv((const char **)thr_knobs);
    
    si_processors   = Stat_Index(nproc, "p");
    si_cores        = Stat_Index(ncore, "c");
    si_threads      = Stat_Index(nthread, "t");
}

void
ArchlibSupport_Init_0(char **knobsArgv)
{
    //
    //Parse Willy style knobs.
    knob_parse_argv((const char **)knobsArgv);
    if (dump_knobs || dump_stats) {
        if (dump_knobs) {
            knob_dump_value(stdout);
        }

        if (dump_stats) {
            stat_generic_dump_name_and_description(cout);
        }
        exit(EXIT_SUCCESS);
    }

    for (uint32 i = 0; i < MAX_PROCS * MAX_CORES * MAX_THREADS; i++)
        instrs_retired[i] = 0;
    
    //
    // Initialize thread library
    thread_initialize(3000);
    
    //
    // si_processors are used in support/stat.cc to represent single cpu - this can be the core in some models
    si_processors   = Stat_Index(nproc, "p");
    si_cores        = Stat_Index(ncore, "c");
    si_threads      = Stat_Index(nthread, "t");

    set_non_power_of_2_nthreads();
    pipe_threads_per_proc(nthread);
    pipe_threads_total(nproc * ncore * nthread);

    if (pipetrace_dump_file) {
        //pipe_init(pipetrace_dump_max);
        pipe_init_noalloc(pipetrace_dump_max);
        pipe_set_will_dump();
    } else {
        pipe_init_noalloc(3);
    }

}

void
ArchlibSupport_Init_1()
{
    stat_generic_init();

    if (pipetrace_file || pipetrace_dump_file) {
        if (pipetrace_yes_file) {
            pipe_embed_yes_file(pipetrace_yes_file);
        }
        pipe_enable_collection();
    } else {
        pipe_quiet();
    }

    if (pipetrace_file) {
        pipe_open_file(pipetrace_file, PT_FORMAT, PIPE_NO_RECORD);
        pipetrace_open = true;
    } 
    else {
        pipetrace_open = false;
    }

    //
    // Initialize perfchecker
    if (perfchecker && stats_file) {
        string pclogfile(stats_file);
        string::size_type p = pclogfile.rfind(".gz");
        if (p == string::npos)
            p = pclogfile.length();
        pclogfile.insert(p, ".pclog");
        pclogfp = xfopen(pclogfile.c_str(), "w");
        if (!pclogfp) {
            fprintf(stderr, "willy: can't create %s\n", pclogfile.c_str());  
            exit(EXIT_FAILURE);
        }
        perf_max_messages(perfchecker_max_msgs);
        perf_start_perfchecker(pclogfp, theController.perf_explain, theController.perf_force,
                               perfchecker_rulemask, perfchecker_window);
    }
    
    //
    // Initialize perfplot
    if (perfplot_file) {
        perfplotfp = xfopen(perfplot_file, "w");
        if (!perfplotfp) {
            fprintf(stderr, "Perfplot: can't create %s\n", (char *) perfplot_file);  
            exit(EXIT_FAILURE);
        }
        perf_start_perfplot(perfplotfp, perfplot_firing, perfplot_interval, perfplot_stats);
    }

    // Setup per cycle routine
//    ctrlWorkList->Add(new CMD_ARCHSIM_SUPPORT_CLASS()); 
    _ArchlibSupport_Done = false;
}

void
ArchlibSupport_Done(bool err_occurred)
{
    if (_ArchlibSupport_Done)
        return;
    _ArchlibSupport_Done = true;

    // Stats
    if (stats_file) {
        ofstream statfile(stats_file);
        if (histo) {
            stat_generic_write(statfile);
        }
        else {
            stats_write(statfile);
        }
        statfile.close();
    }

    // PTV
    if (pipetrace_dump_file
        && (err_occurred || pipetrace_dump_passing)) {
        pipe_dump_buffer(pipetrace_dump_file, pipetrace_dump_max, PT_FORMAT);
    }

    if (pipetrace_open) {
        pipe_close_file(PIPE_NO_RECORD);
        pipetrace_open = false;
    }

    // Perfchecker
    perf_end_perfchecker();
    if (pclogfp) {
        fclose(pclogfp);
        pclogfp = 0;
    }

    // Perfplot
    if (perfplotfp) {
        xfclose(perfplotfp);
        perfplotfp = 0;
    }
}

//function for on_exit to use when dumping stats
static void DumpStatsOnExit (int exitStatus, void *arg)
{
    ASIM_XMSG("DumpStatsOnExit...");

    ArchlibSupport_Done(exitStatus);
}

// SIGABRT handler
static void abort_handler (int signum) 
{
    printf("abort_handler: %d\n", signum);
    ASIM_XMSG("Signal:" << signum << " called dumping ptv and stats..." );
    signal(signum, SIG_DFL) ; // reset sig
    ArchlibSupport_Done(true) ;
    exit(1);
}


bool
CONTROLLER_X86E_CLASS::CMD_Init (UINT32 fdArgc, char **fdArgv, UINT32 pmArgc, char **pmArgv, char **knobsArgv, char **allEnvp)
/*
 * Initialize the performance model. Return false
 * if error.
 */
{
    ASIM_XMSG("CMD_Init...");

    //
    // Initialize archlib support libraries to make use of them in the model
    ArchlibSupport_Init_0(knobsArgv);

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
    CMD_ACK ack = PmProcessEvent(new CMD_INIT_CLASS(fdArgc, fdArgv, pmArgc, pmArgv, allEnvp));

    pmInitialized = ack->Success();
    delete ack->WorkItem();
    delete ack;

    ArchlibSupport_Init_1();

    // Trap signals to dump ptv etc
    signal(SIGABRT, abort_handler) ;
    signal(SIGINT, abort_handler) ;
    signal(SIGSEGV, abort_handler) ;

    on_exit(DumpStatsOnExit, NULL);

    return(pmInitialized);
}


void
CONTROLLER_X86E_CLASS::CMD_EmitStats (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to emit stats of the performance model.
 */
{
    ASIM_XMSG("CMD_EmitStats...");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    
    ctrlWorkList->Add(new CMD_EMITSTATS_X86E_CLASS(trigger, n)); 
    asimSystem->SYS_Break();
}

void
CONTROLLER_X86E_CLASS::CMD_ResetStats (CMD_ACTIONTRIGGER trigger, UINT64 n)
/*
 * Create an action to reset stats of the performance model.
 */
{
    ASIM_XMSG( "CMD_ResetStats...");

    //
    // Generate the progress work item for the controller and break the
    // performance model so that it will return control to the controller
    // if it is executing.
    
    ctrlWorkList->Add(new CMD_RESETSTATS_X86E_CLASS(trigger, n)); 
    asimSystem->SYS_Break();
}

void
CONTROLLER_X86E_CLASS::CMD_Ptv (bool on, CMD_ACTIONTRIGGER trigger, UINT64 n)
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
    ctrlWorkList->Add(new CMD_PTV_X86E_CLASS(on, trigger, n)); 
    asimSystem->SYS_Break();
}

/**********************************************************************/

void
CMD_EMITSTATS_X86E_CLASS::CmdAction (void)
{
    ostringstream statsFileName;

    UINT64 currentInst = 0;
    for (UINT32 i = 0; i < asimSystem->NumCpus(); i++)
    {
        currentInst += asimSystem->SYS_CommittedInsts(i); 
    }

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


void
CMD_RESETSTATS_X86E_CLASS::CmdAction (void)
{
    ASIM_XMSG("CMD_RESETSTATS resetting all stats");

    asimSystem->ClearModuleStats(); 
    IFEEDER_BASE_CLASS::ClearAllFeederStats();
}

CMD_ACK
CMD_PTV_X86E_CLASS::PmAction (void)
{
    ASIM_XMSG("CMD PmAction PTV: Turning ptv " << (on ? "on" : "off")
              << "..." );   
    /*
    if (on) {
#if !PTV_EVENTS
        pipe_enable_collection();
#endif
        if (pipetrace_file && !pipetrace_open) {
            pipe_open_file(pipetrace_file, PT_FORMAT, PIPE_NO_RECORD);
            pipetrace_open = true;
        }
    }
    else {
#if !PTV_EVENTS
        pipe_quiet();
#endif
        if (pipetrace_file && pipetrace_open) {
            pipe_close_file(PIPE_NO_RECORD);
            pipetrace_open = false;
        }   
    }
    */
    return(new CMD_ACK_CLASS(this, true));
}
