/*****************************************************************************
 *
 * @brief Header file for Tanglewood common system driver
 *
 * @author Eric Borch, Pritpal Ahuja
 *
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

// generic
#include <signal.h>
#include <unistd.h>
#include <sstream>

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
    UINT32 feederThreads
    ) 
    : ASIM_SYSTEM_CLASS(name, TOTAL_NUM_CPUS, NULL, feederThreads),
      myContextScheduler(this, "CONTEXT_SCHEDULER"),
      myWarmupManager(this, "WARMUP_MANAGER"),
      myChip(this, "CHIP"),
      statCycles(0)
{
    HeadDumpStripCharts();

    RegisterState(&statCycles, "Cycles", "Simulation cycles completed");

    // connect all buffers together
    ConfigPort::ConnectAll();

    config = new ASIM_CONFIG_CLASS(this);
    config->RegisterSimulatorConfiguration();
    
    // Initialize single instance of thermal model
    myThermalModel = THERMAL_MODEL_CLASS::Instance();
}

ASIM_COMMON_SYSTEM_CLASS::~ASIM_COMMON_SYSTEM_CLASS()
{
    delete config;
}

bool
ASIM_COMMON_SYSTEM_CLASS::InitModule()
{
    if ( ! ASIM_SYSTEM_CLASS::InitModule()) 
    {
        ASIMERROR("System module did not initialize properly\n");
    }

    if (myChip.InitModule() == false) 
    {
        ASIMERROR("The chip module did not initialize properly\n");
    }

    return true;
}


void
SYS_Usage(FILE *file)
/*
 * Print usage...
 */
{
    // We don't have any processor flags...
    fprintf(file, "\nCommon system model flags: NONE\n");
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

    // Count only nonDrainCycles as "Cycles_simulated". 
    // When sampling is turned on, SYS_Cycle() =  SYS_nonDrainCycles() + 
    // number of cycles that the pipe is being drained.
    state_out->AddScalar("uint", "Cycles_simulated",
                         "total number of cycles simulated (warmup + stats w/o drain)",
                         SYS_nonDrainCycles());

    state_out->AddScalar("uint", "cycles_stats_gathered",
                         "number of cycles simulated with stats on", statCycles);

    DumpMemAllocInfo(state_out); 

    myChip.DumpStats(state_out, statCycles, SYS_CommittedInsts());
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

/*
 * Tells if a cpu has a thread assigned.
 */

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

    T1("Warming up"); 
    myWarmupManager.DoWarmUp();

    T1("Executing until cycle " << stop_cycle << " or inst " << stop_inst); 
    
    while (!sysBreak && (SYS_Cycle() < stop_cycle) &&
           (SYS_GlobalCommittedInsts() < stop_inst) &&
           (SYS_GlobalCommittedMacroInsts() < stop_macroinst) &&
           SYS_CommittedMarkers() < stop_marker) 
    {
        if (is_stats_on != statsOn)
        {
            cerr << "Turned statistics collection " << (statsOn ? "on" : "off") << " @ cycle " << SYS_Cycle() << endl;
            is_stats_on = statsOn;
        }

        if (is_events_on != eventsOn)
        {
            cerr << "Turned events collection " << (eventsOn ? "on" : "off") << " @ cycle " << SYS_Cycle() << endl;
            is_events_on = eventsOn;
        }

        // cycle event notification
        DRALEVENT(Cycle(SYS_Cycle()));

        myChip.Clock(SYS_Cycle());

        myContextScheduler.Clock(SYS_Cycle());

        // Calling the thermal model at this level to avoid problems with DFVS, sleep states 
        // and similar potential problems.
        myThermalModel->UpdateTemperature(SYS_Cycle());

        //
        // Call the strip chart routines to dump the data if it is required.
        //
        DumpStripCharts(SYS_Cycle());
        
        // increment the system clock here
        SYS_Cycle()++; 

        /* setting global_cycle is a convenience for the ASSERT macro
         * mesgs */
        global_cycle = SYS_Cycle(); 
         
        statCycles++;

        // inc_nonDrainCycles() will examine a flag to decide whether need to ++nonDrainCycles;
        inc_nonDrainCycles();
        
        trackCycle = SYS_Cycle(); 
    }

    myPowerModel.PowerPostProcessing(); // compute power at end of interval

    T1(SYS_Cycle() << ": SYS_Execute hit marker " << commitWatchMarker <<
          " " << SYS_CommittedMarkers() - start_marker << " times");
    if (SYS_CommittedMarkers() > stop_marker) {
        printf("WARNING: cycle "FMT64U": "FMT64D" markers have been commited -"
               " scheduled only "FMT64D"\n", SYS_Cycle(),
               SYS_CommittedMarkers() - start_marker, stop_marker - start_marker); 
    }
    if (SYS_CommittedMarkers() >= stop_marker) {
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
    // Normally we would parse our flags here... but we don't expect any.
    if (argc != 0) 
    {
        ASIMWARNING("Unknown performance model flag, " << argv[0] << endl);
        return(NULL);
    }

    common_system = new ASIM_COMMON_SYSTEM_CLASS("COMMON_SYSTEM", feederThreads);
    T1_AS(common_system, "Initializing performance model."); 
    common_system->InitModule();
    return(common_system);
}
