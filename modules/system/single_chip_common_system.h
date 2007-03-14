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

// Container for Common System functions

#ifndef _SINGLE_CHIP_COMMON_SYSTEM_
#define _SINGLE_CHIP_COMMON_SYSTEM_

// generic
#include <time.h>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/state.h"
#include "asim/config.h"
#include "asim/stateout.h"

// ASIM public modules
#include "asim/provides/basesystem.h"
#include "asim/provides/chip.h"
#include "asim/provides/context_scheduler.h"
#include "asim/provides/warmup_manager.h"
#include "asim/provides/power_model.h"

void RegisterSimulatorConfiguration(ASIM_REGISTRY reg);



typedef class ASIM_COMMON_SYSTEM_CLASS * ASIM_COMMON_SYSTEM;

class ASIM_COMMON_SYSTEM_CLASS : public ASIM_SYSTEM_CLASS 
{
  protected: 
    // Simulator configuration
    ASIM_CONFIG config;

    //
    // These MUST precede the myChip definition so they are constructed
    // first.
    //
    CONTEXT_SCHEDULER_CLASS myContextScheduler;
    WARMUP_MANAGER_CLASS myWarmupManager;
    POWER_MODEL_CLASS myPowerModel;
    THERMAL_MODEL_CLASS * myThermalModel;
    
    ASIM_CHIP_CLASS myChip;

    // 
    // Local cycle counter. We need this one to count cycles that
    // happen when stats are being collected (the cycle counter in
    // asimSystem' is always counting.
    UINT64 statCycles;

    // True if the system has been interrupted by the controller during
    // this cycle.
    volatile bool sysBreak;

  public:
    ASIM_COMMON_SYSTEM_CLASS(const char *name, UINT32 feederThreads);

    virtual ~ASIM_COMMON_SYSTEM_CLASS();

    virtual void DumpStats(STATE_OUT state_out);

    bool InitModule();

    // Interface functions. The following functions represent
    // a performance models external interface.
    friend void SYS_Usage(FILE * file);
    bool SYS_ScheduleThread(ASIM_THREAD thread);
    bool SYS_UnscheduleThread(ASIM_THREAD thread);
    bool SYS_BlockThread(ASIM_THREAD thread, ASIM_INST inst);
    bool SYS_UnblockThread(ASIM_THREAD thread);
    bool SYS_HookAllThreads();
    bool SYS_IsCpuActive(UINT32 cpunum) const; 
    bool SYS_UnhookAllThreads();
    bool SYS_Execute(const UINT64 stop_nanosecond, const UINT64 stop_cycle, const UINT64 stop_inst, const UINT64 stop_macroinst, const UINT64 stop_packet=0);
    void SYS_Break();
    void SYS_ClearBreak();

    CONTEXT_SCHEDULER GetContextScheduler(void) { return &myContextScheduler; }
    WARMUP_MANAGER GetWarmupManager(void) { return &myWarmupManager; }
};


#endif /* _SINGLE_CHIP_COMMON_SYSTEM_ */ 
