/*****************************************************************************
 *
 * @brief Header file for Tanglewood common clockserver system driver
 *
 * @author Eric Borch, Pritpal Ahuja, Ken Barr
 *
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

// generic
#include <signal.h>
#include <unistd.h>
#include <sstream>
#include <iostream>

// ASIM core
#include "asim/trace.h"
#include "asim/trackmem.h"
#include "asim/cmd.h"
#include "asim/ioformat.h"
#include "asim/event.h"
#include "asim/port.h"

// ASIM public modules
#include "asim/provides/instfeeder_interface.h"
#include "asim/provides/system.h"

/* global_cycle is from mesg.cpp. It is for use by the ASSERT macros. */
extern UINT64 global_cycle;

static ASIM_COMMON_SYSTEM common_system = NULL;

ASIM_COMMON_SYSTEM_CLASS::ASIM_COMMON_SYSTEM_CLASS(
    const char *name,
    string reference_domain,
    UINT32 feederThreads
    ) 
    : ASIM_SYSTEM_CLASS(name, TOTAL_NUM_CPUS, NULL, feederThreads),
      myContextScheduler(this, "CONTEXT_SCHEDULER"),
      myWarmupManager(this, "WARMUP_MANAGER"),
      myChip(this, "CHIP"),
      referenceDomain(reference_domain),
      statClocks(0)
{
    HeadDumpStripCharts();

    RegisterState(&statClocks, "Clocks", "Number of calls to clockserver's Clock");

    // connect all buffers together
    ConfigPort::ConnectAll();

    config = new ASIM_CONFIG_CLASS(this);
    config->RegisterSimulatorConfiguration();

    // We obtain the system wide clock server and set the clocking random seed
    clock = ASIM_CLOCKABLE_CLASS::GetClockServer();
    clock -> SetRandomClockingSeed(RANDOM_CLOCKING_SEED);
    clock -> SetDumpProfile(DUMP_CLOCKING_PROFILE);
    clock -> SetThreadedClocking(THREADED_CLOCKING == 1);

    // Initialize single instance of thermal model
    myThermalModel = THERMAL_MODEL_CLASS::Instance();
    
}


ASIM_COMMON_SYSTEM_CLASS::~ASIM_COMMON_SYSTEM_CLASS()
{
    // We dump the profile information before the modules get destroyed
    clock->DumpProfile();

    delete config;

}


bool
ASIM_COMMON_SYSTEM_CLASS::InitModule()
{
    
    if (!ASIM_SYSTEM_CLASS::InitModule()) 
    {
        ASIMERROR("System module did not initialize properly\n");
    }

    if (myChip.InitModule() == false) 
    {
        ASIMERROR("The chip module did not initialize properly\n");
    }

    // We initialize the clockserver structures
    // IMPORTANT! It must be done after the modules initialization!
    clock->InitClockServer();

    // Set the reference clock domain (-rd option)
    clock->SetReferenceClockDomain(referenceDomain);    
    
    return true;
}


void
SYS_Usage(FILE *file)
/*
 * Print usage...
 */
{    
    fprintf(file, "\nCommon system model flags:\n");
    fprintf(file, "\t-rd <name>\t\tSet the clock domain <name> as the reference clock.\n");
}


void
ASIM_COMMON_SYSTEM_CLASS::DumpStats(STATE_OUT state_out)
{
    ostringstream os;
    
    // get time string and remove trailing '\n'
    time_t tm = time(NULL);
    char* timestr = ctime(&tm);
    char* cr = strrchr(timestr, '\n');
    if (cr)
    {
        *cr = '\0';
    }

    state_out->AddScalar("info", "date",
                         "date and time this simulation was performed", timestr);

    state_out->AddScalar("uint", "cycles_stats_gathered",
                         "number of cycles simulated with stats on (@ reference frequency)", SYS_Cycle());

    state_out->AddScalar("uint", "base_cycles_stats_gathered",
                         "number of base cycles simulated with stats on (@ clockserver frequency)", SYS_BaseCycle());

    state_out->AddScalar("uint", "clockserver_base_frequency_in_MHz",
                         "clockserver base frequency in MHz", clock->getBaseFrequency());                         

    state_out->AddScalar("uint", "total_nanoseconds_simulated",
                         "total number of nanoseconds simulated", SYS_Nanosecond());

    // Dump clockserver stats
    clock->DumpStats(state_out, SYS_BaseCycle());
  
    // Pass the base clockserver frequency cycles to the board, each component can access their
    // real own cycles using the clockable functions
    myChip.DumpStats(state_out, SYS_Cycle(), SYS_CommittedInsts());
    
    DumpMemAllocInfo(state_out); 
}


/*
 * Start 'thread' running on the performance model.
 */
bool
ASIM_COMMON_SYSTEM_CLASS::SYS_ScheduleThread(
    ASIM_THREAD thread)
{
    return myContextScheduler.StartThread(thread, SYS_Cycle());
}


/*
 * Stop and remove 'thread' from the performance model.
 */
bool
ASIM_COMMON_SYSTEM_CLASS::SYS_UnscheduleThread(
    ASIM_THREAD thread)
{
    return myContextScheduler.RemoveThread(thread, SYS_Cycle());
}


/*
 * Block 'thread' in the performance model.
 */
bool
ASIM_COMMON_SYSTEM_CLASS::SYS_BlockThread(
    ASIM_THREAD thread, 
    ASIM_INST inst)
{
//    return myContextScheduler.BlockThread(thread);
    return myContextScheduler.BlockThread();
}


/*
 * Un-block 'thread' from the performance model.
 */
bool
ASIM_COMMON_SYSTEM_CLASS::SYS_UnblockThread(
    ASIM_THREAD thread)
{
//    return myContextScheduler.UnblockThread(thread);
    return myContextScheduler.UnblockThread();
}


/*
 * Hook all 'threads' to the performance model.
 */
bool
ASIM_COMMON_SYSTEM_CLASS::SYS_HookAllThreads()
{
    return myContextScheduler.HookAllThreads();
}


/*
 * Unhook all 'threads' from the performance model.
 */
bool
ASIM_COMMON_SYSTEM_CLASS::SYS_UnhookAllThreads()
{
    return myContextScheduler.UnhookAllThreads();
}

bool
ASIM_COMMON_SYSTEM_CLASS::SYS_IsCpuActive(UINT32 cpunum) const
{
    bool active = false;
    for (UINT32 hwcnum = cpunum * NUM_HWCS_PER_CPU; hwcnum < (cpunum + 1) * NUM_HWCS_PER_CPU; hwcnum++)
    {
        active |= myContextScheduler.IsHWCActive(hwcnum);
    }
    return active;
}


/*
 * Pass control to the performance model until cycle 'stopCycle' or until
 * committed instruction 'stopInst' or until commiting
 * marker 'commitWatchMarker'.
 * Return false if execution is stopped (via SYS_Break()) before we reach
 * 'stopCycle', 'stopInst' or 'commitWatchMarker'.
 */
bool
ASIM_COMMON_SYSTEM_CLASS::SYS_Execute(
    const UINT64 stop_nanosecond,
    const UINT64 stop_cycle, 
    const UINT64 stop_inst, 
    const UINT64 stop_macroinst,
    const UINT64 stop_packet)
{

    static bool is_stats_on = false; 
    static bool is_events_on = false; 
    extern UINT64 trackCycle; 
    UINT64 start_marker = SYS_CommittedMarkers(); 
    UINT64 stop_marker = start_marker + 1; 
    // Note: could generalize this ^ for committing N markers rather than 1

    TRACE(Trace_Sys, cout << "Warming up" << endl); 
    myWarmupManager.DoWarmUp();

    TRACE(Trace_Sys, cout << "Executing until cycle " << stop_cycle << " or inst " << stop_inst << " or nanosecond " << stop_nanosecond << endl);

    UINT64 sys_cycle = SYS_Cycle();
    
    while (!sysBreak && 
           (SYS_Nanosecond() < stop_nanosecond) &&
           (sys_cycle < stop_cycle) &&
           (SYS_GlobalCommittedInsts() < stop_inst) &&
           (SYS_GlobalCommittedMacroInsts() < stop_macroinst) &&
           (SYS_CommittedMarkers() < stop_marker)) 
    {
        
        if (is_stats_on != statsOn)
        {
            cerr << "Turned statistics collection " << (statsOn ? "on" : "off") << " @ cycle " << sys_cycle << endl;
            is_stats_on = statsOn;
        }

        if (is_events_on != eventsOn)
        {
            cerr << "Turned events collection " << (eventsOn ? "on" : "off") << " @ cycle " << sys_cycle << endl;
            is_events_on = eventsOn;
        }

        // We clock the clockserver
        UINT64 bf_cycle_increment = clock->Clock();
        sys_cycle = SYS_Cycle();

        // IMPORTANT! All modules are clocked by the clockserver 
        // myChip.Clock(SYS_Cycle());        
        
        myContextScheduler.Clock(sys_cycle);

        // Calling the thermal model at this level to avoid problems with DFVS, sleep states 
        // and similar potential problems.
        myThermalModel->UpdateTemperature(sys_cycle);
        
        //
        // Call the strip chart routines to dump the data if it is required.
        // FIX ME: the capacity option is currently broken. By now strip charts are using
        // the reference cycle, but they should use the local cycle instead.
        DumpStripCharts(sys_cycle);

        // increment the system clock here
        SYS_BaseCycle() += bf_cycle_increment; // Cycle counter @ clockserver base frequency
        
        // Global clock doesn't need to be incremented as it is mantained by the clockserver
        // SYS_Cycle()++; 
         
        // setting global_cycle is a convenience for the ASSERT macro mesgs
        global_cycle = sys_cycle; 
         
        statClocks++;
        
        trackCycle = sys_cycle;
        
    }

    myPowerModel.PowerPostProcessing(); // compute power at end of interval

    TRACE(Trace_Sys, printf(FMT64U": SYS_Execute hit marker "FMT32D
                            " "FMT64D" times\n", SYS_Cycle(),
                            commitWatchMarker, SYS_CommittedMarkers() - start_marker));
    
    if (SYS_CommittedMarkers() > stop_marker)
    {
        printf("WARNING: cycle "FMT64U": "FMT64D" markers have been commited -"
               " scheduled only "FMT64D"\n", SYS_Cycle(),
               SYS_CommittedMarkers() - start_marker, stop_marker - start_marker); 
    }
    
    if (SYS_CommittedMarkers() >= stop_marker)
    {
        // watch is self-resetting when triggered
        commitWatchMarker = -1;
        // Note: we need to make sure here that the scheduler gets a
        // stop event, otherwise its going to turn around on the spot and
        // execute again, without any more action scheduled to stop it!
        CMD_Stop(ACTION_NOW);
    }
    
    return(!sysBreak);
}


void 
ASIM_COMMON_SYSTEM_CLASS::SYS_Break()
{
    sysBreak = true;
}


void 
ASIM_COMMON_SYSTEM_CLASS::SYS_ClearBreak()
{
    sysBreak = false;
}


// Initialize performance model by instantiating common_system
ASIM_SYSTEM
SYS_Init(
    UINT32 argc, 
    char *argv[], 
    char *envp[],
    UINT32 feederThreads)
{
    TRACE(Trace_Sys, cout << "Initializing performance model.\n"); 

    string reference_domain = "";
    
    for (UINT32 i = 0; i < argc; i++) 
    {
        // -rd <d>       Set the model reference domain
        if ((strcmp(argv[i], "-rd") == 0) && (argc > (i+1))) 
        {
            reference_domain = argv[++i];
        }
        else 
        {
            ASIMWARNING("Unknown flag, " << argv[i] << endl);
            return(false);
        }
    }

    ASIM_SMP_CLASS::Init(MAX_PTHREADS, LIMIT_PTHREADS);

    common_system = new ASIM_COMMON_SYSTEM_CLASS("COMMON_SYSTEM",
                                                 reference_domain,
                                                 feederThreads);
    common_system->InitModule();
    
    return(common_system);
}

