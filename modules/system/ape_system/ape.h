/*
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
/**
 * @file
 * @author Greg Steffan (SMTE), IA64 by Artur Klauser
 *
 * @brief The APE (Asinine Plugin Exerciser) system.
 *
 * Simple processor system that allows other modules to be plugged
 * into a standalone environment and run in isolation from a
 * pipeline performance model.
 * Use when isolating and testing other ASIM components,
 * filtering instructions, or implementing other simple standalone models.
 */

#ifndef _APE_
#define _APE_

// generic
#include <time.h>
#include <stdio.h>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/state.h"
#include "asim/config.h"
#include "asim/stateout.h"

// ASIM public modules
#include "asim/provides/basesystem.h"
#include "asim/provides/hardware_context.h"


typedef class DRIVER_CLASS *DRIVER;

/*
 * Class INST_BUF
 *
 * circular buffer for holding uncommitted instructions
 */
typedef class INST_BUF_CLASS *INST_BUF;
class INST_BUF_CLASS
{
  private:
    UINT32 my_tpu_idx;
    
    ASIM_INST my_inst[MAX_INST_BUF_SZ];
    UINT32 my_head;
    UINT32 my_tail;

    UINT32 my_cur_idx;
    UINT32 my_num_entries;

    UINT32 next_idx(UINT32 idx) { return (idx+1) % MAX_INST_BUF_SZ; }
    bool is_buf_empty() { return !(my_inst[my_head]); }
    bool is_buf_full() { return !!(my_inst[my_tail]); }

    void reset();
    void free_insts_and_reset();

  public:
    INST_BUF_CLASS() { reset(); }

    void setTPUIdx(UINT32 tpu_idx) { my_tpu_idx = tpu_idx; }

    void insert(ASIM_INST inst);
    ASIM_INST removeHead();
    ASIM_INST removeHeadAndAbort();
};


/*
 * Class TPU
 *
 * holds the state for each tpu
 */
typedef class TPU_CLASS *TPU;
class TPU_CLASS
{
  private:
    UINT32 my_tpu_idx;
    ASIM_THREAD my_thread;
    SW_CONTEXT my_swc;
    HW_CONTEXT_CLASS my_hwc;

    /* true if I have no thread scheduled */
    bool my_is_free;

    /* true if I am not to issue instructions */
    bool my_stalled;

    /* this is the current fetch pc of the tpu */
    IADDR_CLASS my_pc;

    /* this is the default pc to rewind fetching to */
    IADDR_CLASS my_first_uncommitted_pc;

    UINT64 my_stalled_cycles;

    INST_BUF_CLASS my_inst_buf;

    void pc_inc() { my_pc = my_pc.Next(); }
    void pc_branch(IADDR_CLASS target) { my_pc = target; }

  public:
    TPU_CLASS(ASIM_REGISTRY reg, UINT32 cpu, UINT32 tpu);
    ~TPU_CLASS();

    void setTPUIdx(UINT32 tpu_idx) {
      my_tpu_idx = tpu_idx;
      my_inst_buf.setTPUIdx(tpu_idx);
    }
    
    /* accessor functions for scheduling threads */
    bool isFree() { return my_is_free; }
    void scheduleThread(ASIM_THREAD at);
    void unscheduleThread(ASIM_THREAD at);
    void unblockThread(ASIM_THREAD at);
    void blockThread(ASIM_THREAD at);

    ASIM_THREAD thread() { return my_thread; }
    SW_CONTEXT swc() { return my_swc; }
    IADDR_CLASS pc() { return my_pc; }

    /* fetch and execute the next instruction */
    ASIM_INST fetchAndExecute(UINT64 cycle);

    /* commit all uncommitted instructions */
    UINT32 commit(UINT64 cycle);

    bool isStalled() { return my_stalled; }
    void stall() { my_stalled = true; }

    /*  we add 1 to the cycles because of the way we clock this object */
    void stall(UINT64 cycles) {
        my_stalled = true;
        my_stalled_cycles = cycles + 1;
    }

    void wakeup() { my_stalled = false; }

    /* abort all uncommitted instructions and restart at the
     * first uncommitted pc */
    void abort(UINT64 cycle);
    void Clock();
};


/*
 * Class ASIM_APE
 *
 */
typedef class ASIM_APE_CLASS *ASIM_APE;
class ASIM_APE_CLASS
    : public ASIM_SYSTEM_CLASS
{
  private:
    // Simulator configuration
    ASIM_CONFIG config;

    /*
     * Local cycle and retired counters. We need these to count 
     * cycles/retires that happen when stats are being collected (the
     * counters in 'asimSystem' is always counting).
     */
    UINT64 statCycles;
    UINT64 statRetired;
    
    /*
     * True if the system has been interrupted by the controller during
     * this cycle.
     */
    volatile bool sysBreak;
                                                                  
    void DumpStats (STATE_OUT stateOut);
                                                                  
    TPU my_tpu[NUM_HWCS_PER_CPU];                                                                
    DRIVER my_driver;

  public:
    ASIM_APE_CLASS (const char *n);
                                                                  
    ~ASIM_APE_CLASS ();

    /********** System Interface functions *******************/
    friend ASIM_SYSTEM SYS_Init (UINT32 argc, char *argv[]);
    friend void SYS_Usage(FILE * file);
    bool SYS_ScheduleThread(ASIM_THREAD thread);
    bool SYS_UnscheduleThread(ASIM_THREAD thread);
    bool SYS_BlockThread(ASIM_THREAD thread, ASIM_INST);
    bool SYS_UnblockThread(ASIM_THREAD thread);
    bool SYS_UnhookAllThreads();
    bool SYS_HookAllThreads();
    bool SYS_Execute(UINT64 stopNanosecond, UINT64 stopCycle, UINT64 stopInst,UINT64 stopMacroInst, UINT64 stopPacket=0);
                                                                  
    void SYS_Break(void) { sysBreak = true; }
    void SYS_ClearBreak(void) { sysBreak = false; }

    /********** Driver Feedback functions ********************/

    /* stalls the tpu, so it won't issue instructions */
    void stallTPU(UINT32 tpu_id);

    /* this is not yet implemented */
    void stallTPU(UINT32 tpu_id, UINT64 cycles);

    bool isStalled(UINT32 tpu_id) {
        return my_tpu[tpu_id]->isStalled();
    }

    /* wake up a stalled tpu */
    void wakeupTPU(UINT32 tpu_id);
                                                                  
    /* commits all uncommitted instrs */
    void commitTPU(UINT32 tpu_id);

    /* aborts all uncommitted instrs 
     * restarts execution at the 
     * first uncommited instruction */
    void abortAndRestartTPU(UINT32 tpu_id);

    /* aborts all uncommitted instrs 
     * restarts execution at the given pc */
    /* not yet implemented */
    void abortAndRestartTPU(UINT32 tpu_id, IADDR_CLASS pc);
};

#endif /* _APE_ */
