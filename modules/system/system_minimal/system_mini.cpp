/*****************************************************************************
 *
 * @brief Header file for Tanglewood common system driver
 *
 * @author Eric Borch, Pritpal Ahuja, Joel Emer
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

// Minimialisti system - should become basesystem for simple systems....

// generic
#include <signal.h>
#include <unistd.h>
#include <sstream>

// ASIM core
#include "asim/ioformat.h"
#include "asim/port.h"
#include "asim/trace.h"

// ASIM public modules
#include "asim/provides/system.h"

static ASIM_SYSTEM common_system = NULL;
bool statsOn = TRUE;

ASIM_SYSTEM_CLASS::ASIM_SYSTEM_CLASS(
    const char *name,
    UINT16 ncpu,
    ASIM_EXCEPT e
    ) :
    ASIM_MODULE_CLASS(NULL, name, e), cycles(0), num_cpus(ncpu)

{
    committed = new UINT64[ncpu];
    memset(committed, 0, sizeof(UINT64) * ncpu); 

    HeadDumpStripCharts();

    RegisterState(&statCycles, "Cycles", "Simulation cycles completed");

    // connect all buffers together
    ConfigPort::ConnectAll();

    config = new ASIM_CONFIG_CLASS(this);
    config->RegisterSimulatorConfiguration();
}

ASIM_SYSTEM_CLASS::~ASIM_SYSTEM_CLASS()
{
    delete [] committed;
    delete config;
}

bool
ASIM_SYSTEM_CLASS::InitModule()
{
//  if (myChip.InitModule() == false) 
//  {
//      ASIMERROR("The chip in asim system model did not initialize properly\n");
//  }

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
ASIM_SYSTEM_CLASS::DumpStats(STATE_OUT state_out)
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
//    state_out->AddScalar("uint", "Cycles_simulated",
//                         "total number of cycles simulated (warmup + stats w/o drain)",
//                         SYS_nonDrainCycles());

//    state_out->AddScalar("uint", "cycles_stats_gathered",
//                        "number of cycles simulated with stats on", statCycles);

//    DumpMemAllocInfo(state_out); 

//    myChip.DumpStats(state_out, statCycles, SYS_CommittedInsts());
}


/*
 * Pass control to the performance model until cycle 'stopCycle' or until
 * committed instruction 'stopInst' or until commiting
 * marker 'commitWatchMarker'.
 * Return false if execution is stopped (via SYS_Break()) before we reach
 * 'stopCycle', 'stopInst' or 'commitWatchMarker'.
 */
bool
ASIM_SYSTEM_CLASS::SYS_Execute(
    const UINT64 stop_cycle, 
    const UINT64 stop_inst, 
    const UINT64 stop_MacroInst, 

    const UINT64 stop_packet)
{
    static bool is_stats_on = false; 

    TRACE(Trace_Sys, cout << "Executing until cycle " << stop_cycle << " or inst " << stop_inst << endl); 
    
    while (!sysBreak && (SYS_Cycle() < stop_cycle) &&
           (SYS_GlobalCommittedInsts() < stop_inst) &&
           (SYS_GlobalCommittedMacroInsts() < stop_MacroInst) ) 
    {
        if (is_stats_on != statsOn)
        {
            cerr << "Turned statistics collection " << (statsOn ? "on" : "off") << " @ cycle " << SYS_Cycle() << endl;
            is_stats_on = statsOn;
        }

//        myChip.Clock(SYS_Cycle());

        //
        // Call the strip chart routines to dump the data if it is required.
        //
        DumpStripCharts(SYS_Cycle());
        
        // increment the system clock here
        SYS_Cycle()++; 
        
        statCycles++;

    }

    return(!sysBreak);
}


// Initialize performance model by instantiating common_system
ASIM_SYSTEM
SYS_Init(
    UINT32 argc, 
    char *argv[], 
    char *envp[])
{
    TRACE(Trace_Sys, cout << "Initializing performance model.\n"); 

    // Normally we would parse our flags here... but we don't expect any.
    if (argc != 0) 
    {
        ASIMWARNING("Unknown performance model flag, " << argv[0] << endl);
        return(NULL);
    }

    ASIM_SMP_CLASS::Init(MAX_PTHREADS, LIMIT_PTHREADS);

    common_system = new ASIM_SYSTEM_CLASS("COMMON_SYSTEM");
    common_system->InitModule();
    return(common_system);
}
