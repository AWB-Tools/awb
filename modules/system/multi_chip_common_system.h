/*****************************************************************************
 *
 * @brief Header file for Tanglewood common system driver
 *
 * @author Oscar Rosell, Ramon Matas Navarro
 *
 *Copyright (C) 2004-2006 Intel Corporation
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

#ifndef _MULTI_CHIP_COMMON_SYSTEM_
#define _MULTI_CHIP_COMMON_SYSTEM_

// generic
#include <time.h>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/state.h"
#include "asim/config.h"
#include "asim/stateout.h"
#include "asim/smp.h"

// ASIM public modules
#include "asim/provides/basesystem.h"
#include "asim/provides/context_scheduler.h"
#include "asim/provides/warmup_manager.h"
#include "asim/provides/board.h"
#include "asim/provides/power_model.h"

void RegisterSimulatorConfiguration(ASIM_REGISTRY reg);


typedef class ASIM_MULTI_CHIP_SYSTEM_CLASS* ASIM_MULTI_CHIP_SYSTEM;

class ASIM_MULTI_CHIP_SYSTEM_CLASS : public ASIM_SYSTEM_CLASS 
{
  protected:
  
    // Simulator configuration
    ASIM_CONFIG config;

    //
    // These MUST precede the myChips definition so they are constructed
    // first.
    //
    CONTEXT_SCHEDULER_CLASS myContextScheduler;
    WARMUP_MANAGER_CLASS myWarmupManager;
    POWER_MODEL_CLASS myPowerModel;
    // dtarjan 07/24/2006
    // No thermal model for the multi chip model, since we currently only have a thermal solver
    // for a single chip/package, not for a whole box. 
    // If you still want to have a thermal model, you need to change the thermal model class from 
    // a simple singleton design pattern to having multiple instances and a registry. 
    // Then you can add the thermal model class declaration here.
    
    ASIM_BOARD_CLASS myBoard;

    // Reference clock domain
    const string referenceDomain;

    // Clock server
    ASIM_CLOCK_SERVER clock;

    // 
    // Local cycle counter. We need this one to count cycles that
    // happen when stats are being collected (the cycle counter in
    // asimSystem' is always counting.
    UINT64 statClocks;
    UINT64 statCycles;
    UINT64 statBaseCycles;

    // True if the system has been interrupted by the controller during
    // this cycle.
    volatile bool sysBreak;

  public:
  
    ASIM_MULTI_CHIP_SYSTEM_CLASS(
        const char *name,
        string reference_domain,
        UINT32 feederThreads);
    virtual ~ASIM_MULTI_CHIP_SYSTEM_CLASS();

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
    
    // We need to redefine the following function
    inline UINT64 SYS_Cycle (UINT32 cpunum)
    {
        T2("\t*****Calling SYS_Cycle with cpunum = " << cpunum << endl);
        ASSERT(cpu2module[cpunum], "Base system method SYS_SetCpu2Module not called for the cpunum " << cpunum << ".");
        
        return cpu2module[cpunum]->GetCurrentCycle();
    }    
    inline UINT64& SYS_Cycle (void)
    {
        return (clock->getReferenceCycle());
    }    
    inline UINT64 SYS_Nanosecond (void)
    {
        return (clock->getNanosecond());
    }
   
    CONTEXT_SCHEDULER GetContextScheduler(void) { return &myContextScheduler; }
    WARMUP_MANAGER GetWarmupManager(void) { return &myWarmupManager; }
    
};


#endif /* _MULTI_CHIP_COMMON_SYSTEM_ */ 
