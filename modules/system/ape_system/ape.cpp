/*
 *Copyright (C) 1998-2006 Intel Corporation
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
 * APE, the Asinine Plugin Exerciser, or what used to be SMTE.
 */

// generic
#include <signal.h>
#include <time.h>
#include <cstring>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/state.h"
#include "asim/cmd.h"

// ASIM public modules
#include "asim/provides/instfeeder_interface.h"
#include "asim/provides/system.h"
#include "asim/provides/ape_driver.h"
#include "asim/provides/tarati_system.h"


static ASIM_APE ape = NULL;
AsimTarati::System * asimTaratiSystem = NULL;

/**********************************************************
* INST_BUF
***********************************************************/

void 
INST_BUF_CLASS::reset()
{
  int i;
  my_head = 0;
  my_tail = 0;
  my_num_entries = 0;
  for (i=0;i<MAX_INST_BUF_SZ;i++) {
    my_inst[i] = NULL;
  }
}

void 
INST_BUF_CLASS::free_insts_and_reset()
{
  int i = my_head;
  while (my_inst[i]) {
    my_inst[i] = NULL;
    i = next_idx(i);
  }
  my_head = 0;
  my_tail = 0;
  my_num_entries = 0;
}

void 
INST_BUF_CLASS::insert (
    ASIM_INST inst)
{
  ASSERTX(!is_buf_full());
  my_inst[my_tail] = inst;
  my_tail = next_idx(my_tail);
  my_num_entries++;
}

ASIM_INST 
INST_BUF_CLASS::removeHead()
{
  ASIM_INST inst = my_inst[my_head];
  if (is_buf_empty()) {
    return NULL;
  }
  my_inst[my_head] = NULL;
  my_head = next_idx(my_head);
  my_num_entries--;
  return inst;
}

ASIM_INST 
INST_BUF_CLASS::removeHeadAndAbort()
{
  ASSERTX(!is_buf_empty());
  ASIM_INST inst = removeHead();
  free_insts_and_reset();
  return inst;
}


/**********************************************************
* tpu_class
***********************************************************/

TPU_CLASS::TPU_CLASS(ASIM_REGISTRY reg, UINT32 cpu, UINT32 tpu) :
    my_hwc(reg, cpu, tpu)
{
  my_thread = NULL;
  my_swc = NULL;
  my_is_free = true;
  my_stalled = false;
  my_stalled_cycles = 0;
}

TPU_CLASS::~TPU_CLASS ()
{
    if (my_swc)
    {
        delete my_swc;
    }
}

void 
TPU_CLASS::scheduleThread (
    ASIM_THREAD at)
{
  ASSERTX(my_is_free);
  ASSERT((my_thread == NULL) || (my_thread == at),
         "APE only supports one thread per TPU_CLASS until someone figures out how to map threads to software contexts");

  my_thread = at;
  my_pc = at->StartVirtualPc();
  my_first_uncommitted_pc = my_pc;
  my_is_free = false;

  ASSERTX(my_swc == NULL);
  my_swc = new SW_CONTEXT_CLASS(at->IFeeder(), at->IStreamHandle());
  my_hwc.SetSWC(my_swc, 0);
  my_swc->SetHWC(&my_hwc);
}

void 
TPU_CLASS::unscheduleThread(ASIM_THREAD at)
{
  ASSERTX(!my_is_free);
  my_is_free = true;
}


void 
TPU_CLASS::unblockThread(ASIM_THREAD at)
{
  ASSERTX(my_is_free);
  my_thread = at;
  my_pc = at->StartVirtualPc();
  my_first_uncommitted_pc = my_pc;
  my_is_free = false;
}

void 
TPU_CLASS::blockThread(ASIM_THREAD at)
{
  ASSERTX(!my_is_free);
  my_is_free = true;
}

ASIM_INST
TPU_CLASS::fetchAndExecute(UINT64 cycle)
{
  ASIM_INST inst;

  inst = my_hwc.Fetch(cycle, my_pc);
  my_inst_buf.insert(inst);

  my_hwc.Issue(cycle, inst);

  my_hwc.Execute(inst);
  /* execute loads */
  /*if (inst->IsLoad())*/

  /* Federico Ardanaz notes:
  * Here we consult MemRead property instead 
  * IsLoad() in order to make the call to the
  * Feeder when we have a TTL/APE load also.
  */

  const IFEEDER_BASE feeder = my_thread->IFeeder();

  if (inst->IsMemRead() && feeder->IsCapable(IFEEDER_BASE_CLASS::IFEED_MEM_CTL))
  {
      if(!feeder->DoRead(inst))
      {
         my_hwc.Kill(inst,false,true);
         return NULL;
      }

  }
  /* change pc to point to next instruction */        
  if (inst->IsNonSequentialPC()) 
  {
      my_pc = inst->GetActualTarget();
//      my_pc = inst->GetNextVirtualPC();
      //pc_branch(inst->GetNextVirtualPC());
  }
  else if (inst->HadFault())
  { 
      cout<<"Had fault"<<endl;
      my_pc = inst->GetNextVirtualPC();
  } 
  else if (inst->IsControlOp()) 
  {
        my_pc = inst->GetActualTarget();
        //pc_branch(inst->GetActualTarget());
  } 
  else 
  {
        my_pc = inst->GetNextVirtualPC();
        //pc_inc();
  }
  return inst;
}

UINT32
TPU_CLASS::commit(UINT64 cycle)
{
  const IFEEDER_BASE feeder = my_thread->IFeeder();
  ASIM_INST inst = my_inst_buf.removeHead();
  UINT32 num_committed = 0;

  while (inst) {
  
    /* execute stores */
    /*if (inst->IsStore())*/

    /* Federico Ardanaz notes:
    * Here we consult MemWrite property instead 
    * IsStore() in order to make the call to the
    * Feeder when we have a TTL/APE load also.
    */

    if (inst->IsMemWrite() && feeder->IsCapable(IFEEDER_BASE_CLASS::IFEED_MEM_CTL))
    {
        feeder->DoSpecWrite(inst);
        feeder->DoWrite(inst);
    }

    my_hwc.Commit(cycle, inst, 1);
    num_committed++;

    inst = my_inst_buf.removeHead();
  }

  my_first_uncommitted_pc = my_pc;

  return num_committed;
}

void 
TPU_CLASS::abort(UINT64 cycle)
{
  ASIM_INST inst = my_inst_buf.removeHeadAndAbort();

  my_hwc.Kill(cycle, inst, 1, false, true);
  my_pc = my_first_uncommitted_pc;
}

void 
TPU_CLASS::Clock()
{
  if (my_stalled_cycles) {
    my_stalled_cycles--;
    if (!my_stalled_cycles) {
      wakeup();
    }
  }
}

/**********************************************************
* ASIM_APE_CLASS
***********************************************************/

ASIM_APE_CLASS::ASIM_APE_CLASS (
    const char *n)
    : ASIM_SYSTEM_CLASS(n),
      statCycles(0),
      statRetired(0),
      sysBreak(false)
{
  for (UINT32 i=0;i<NUM_HWCS_PER_CPU;i++) {
      my_tpu[i] = NULL;
  }

  my_driver = new DRIVER_CLASS(this,"DRIVER");

  //
  // Initialize the system and driver
  //
  InitModule();
  my_driver->InitModule();

  /* allow as many cpu instructions as we can possibly buffer */
  ASIM_INST_CLASS::SetMaxObjs(MAX_INST_BUF_SZ * NUM_HWCS_PER_CPU);

  // instantiate ASIM configuration
  config = new ASIM_CONFIG_CLASS(this);
  config->RegisterSimulatorConfiguration();
}


ASIM_APE_CLASS::~ASIM_APE_CLASS ()
{
  // free allocated memory
  for (UINT32 i=0;i<NUM_HWCS_PER_CPU;i++) {
      if (my_tpu[i] != NULL)
      {
          delete (my_tpu)[i];
      }
  }
  delete my_driver;
  delete config;
}


void
ASIM_APE_CLASS::DumpStats (
    STATE_OUT stateOut)
{
  ostringstream os;
  time_t tm = time(NULL);

  // get time string and remove trailing '\n'
  char* timestr = ctime(&tm);
  char* cr = strrchr(timestr, '\n');
  if (cr)
  {
      *cr = '\0';
  }

  stateOut->AddScalar("info", "date",
    "date and time this simulation was performed", timestr);
  stateOut->AddScalar("uint", "Cycles_simulated",
    "total number of cycles simulated (warmup + stats)", SYS_Cycle());
  for (UINT32 i = 0; i < NumCpus(); i++)
  {
      os.str(""); // clear
      os << "CPU_" << i
         << "_instructions_committed";
      stateOut->AddScalar("uint", os.str().c_str(),
        "number of committed instructions for this CPU", SYS_CommittedInsts(i));
  }
  stateOut->AddScalar("uint", "cycles_stats_gathered",
    "number of cycles simulated with stats on", statCycles);

  // pass down to base class
  ASIM_REGISTRY_CLASS::DumpStats(stateOut);
}


/*
* Inform the performance model that a a new thread begins.
*/
bool
ASIM_APE_CLASS::SYS_ScheduleThread (
    ASIM_THREAD thread)
{
    UINT32 i;
    
    for (i=0;i<NUM_HWCS_PER_CPU;i++)
    {
        if (my_tpu[i] == NULL)
        {
            my_tpu[i] = new TPU_CLASS(ASIM_REGISTRY(this), 0, i);
            my_tpu[i]->setTPUIdx(i);
        }

        if (my_tpu[i]->isFree())
        {
            cerr << "Scheduling APE tpu " << i << endl;
            my_tpu[i]->scheduleThread(thread);
            return true;
        }
    }
    return false;
}


/*
* Inform the performance model that a thread is ending.
*/
bool
ASIM_APE_CLASS::SYS_UnscheduleThread (
    ASIM_THREAD thread)
{
  UINT32 i;

  for (i=0;i<NUM_HWCS_PER_CPU;i++) {
    if ((my_tpu[i] != NULL) && (my_tpu[i]->thread() == thread)) {
      my_tpu[i]->unscheduleThread(thread);
      return true;
    }
  }
  return false;
}

/*
* Inform the performance model that a thread is un-blocked.
*/
bool
ASIM_APE_CLASS::SYS_UnblockThread (
    ASIM_THREAD thread)
{
  UINT32 i;

  for (i=0;i<NUM_HWCS_PER_CPU;i++) {
    if ((my_tpu[i] != NULL) && my_tpu[i]->isFree()) {
      my_tpu[i]->unblockThread(thread);
      return true;
    }
  }
  return false;
}


/*
* Inform the performance model that a thread is blocking.
*/
bool
ASIM_APE_CLASS::SYS_BlockThread (
    ASIM_THREAD thread,
    ASIM_INST inst)
{
  UINT32 i;

  for (i=0;i<NUM_HWCS_PER_CPU;i++) {
    if ((my_tpu[i] != NULL) && (my_tpu[i]->thread() == thread)) {
      my_tpu[i]->blockThread(thread);
      return true;
    }
  }
  return false;
}

// APE models are unithreaded so nothing has to be done here
bool
ASIM_APE_CLASS::SYS_HookAllThreads()
{
  return true;
}
bool
ASIM_APE_CLASS::SYS_UnhookAllThreads()
{
  return true;
}

/*
 * Pass control to the performance model until cycle 'stopCycle' or until
 * committed instruction 'stopInst' or until commiting
 * marker 'commitWatchMarker'.
 * Return false if execution is stopped (via SYS_Break()) before we reach
 * 'stopCycle', 'stopInst' or 'commitWatchMarker'.
 */
bool
ASIM_APE_CLASS::SYS_Execute (
    UINT64 stopNanosecond,
    UINT64 stopCycle,
    UINT64 stopInst,
    UINT64 stopMacroInst,
    UINT64 stopPacket)
{
  UINT32 i;
  ASIM_INST instr;
  UINT32 committed_insts = 0;
  UINT64 idle_count = MAX_IDLE_CYCLES;
  UINT64 startMarker = SYS_CommittedMarkers(); 
  UINT64 stopMarker = startMarker + 1; 
    // Note: could generalize this ^ for committing N markers rather than 1

  /* at least one tpu must be active each cycle */
  /* (this will change when delayed stall is implemented) */
  bool is_any_tpu_active = true; 

  TRACE(Trace_Sys,
      cout << SYS_Cycle() << ": SYS_Execute executing until cycle "
           << stopCycle << " or inst " << stopInst << endl); 

  while (!sysBreak && (SYS_Cycle() < stopCycle) &&
         (SYS_GlobalCommittedInsts() < stopInst) &&
         (SYS_GlobalCommittedMacroInsts() < stopMacroInst) &&

         SYS_CommittedMarkers() < stopMarker) 
  {
    TRACE(Trace_Sys,
        cout << "*************" << endl
             << SYS_Cycle() << ": " << Name() << endl);

    /* Ensure that the engine is not idle for more than a maximum number
     * of cycles. */
    if (is_any_tpu_active) {
      idle_count = MAX_IDLE_CYCLES;
    } else {
      idle_count--;
    }
    ASSERTX(idle_count);

    is_any_tpu_active = false;
    for (i=0;i<NUM_HWCS_PER_CPU;i++) {
        if (my_tpu[i] != NULL)
        {
            my_tpu[i]->Clock();
            if (my_tpu[i]->isFree() || my_tpu[i]->isStalled()) continue;
            is_any_tpu_active = true;

            instr = my_tpu[i]->fetchAndExecute(SYS_Cycle());

            if (my_driver->NextInstruction(instr,SYS_Cycle())) {
                SYS_CommittedInsts(0) += my_tpu[i]->commit(SYS_Cycle());
            }
        } 
    }
    my_driver->Clock(SYS_Cycle());
    statCycles++;
    SYS_Cycle()++;

    // check for work in Tarati server
    asimTaratiSystem->Work();
  }

  TRACE(Trace_Sys,
      cout << SYS_Cycle() << ": SYS_Execute hit marker "
           << commitWatchMarker << " "
           << SYS_CommittedMarkers() - startMarker << " times" << endl);
  if (SYS_CommittedMarkers() > stopMarker) {
      cout << "WARNING: cycle " << SYS_Cycle() << ": "
           << SYS_CommittedMarkers() - startMarker
           << " markers have been commited - scheduled only "
           << stopMarker - startMarker << endl; 
  }
  if (SYS_CommittedMarkers() >= stopMarker) {
      // watch is self-resetting when triggered
      commitWatchMarker = -1;
      // Note: we need to make sure here that the scheduler gets a
      // stop event, otherwise its going to turn around on the spot and
      // execute again, without any more action scheduled to stop it!
      CMD_Stop(ACTION_NOW);
  }

  return(!sysBreak);
}

/*
* Initialize performance model by instantiating
* a single processor.
*/
ASIM_SYSTEM
SYS_Init (
    UINT32 argc,
    char *argv[],
    char *envp[])
{
  TRACE(Trace_Sys, cout << "Initializing APE engine." << endl);

  //
  // Normally we would parse our flags here... but we don't
  // expect any.

  if (argc != 0) {
    ASIMWARNING("Unknown performance model flag, " << argv[0] << endl);
    return(NULL);
  }
  
  ape = new ASIM_APE_CLASS("APE");
  ape->InitModule();

  // instantiate APE's Tarati System
  asimTaratiSystem = new AsimTarati::System(ape);

  if (asimTaratiSystem->GetPort() >= 0) {
    cout << "Tarati Server listening on port "
         << asimTaratiSystem->GetPort() << endl;
  }

  return ape;
}

/*
* Print usage...
*/
void
SYS_Usage (
    FILE *file)
{
  //
  // We don't have any processor flags...

  fputs ("\nStand alone performance model flags: NONE\n", file);
}

void 
ASIM_APE_CLASS::stallTPU (
    UINT32 tpu_id)
{
  my_tpu[tpu_id]->stall();
};

void 
ASIM_APE_CLASS::stallTPU (
    UINT32 tpu_id,
    UINT64 cycles)
{
  my_tpu[tpu_id]->stall(cycles);
}

void 
ASIM_APE_CLASS::wakeupTPU (
    UINT32 tpu_id)
{
  my_tpu[tpu_id]->wakeup();
};								

void 
ASIM_APE_CLASS::commitTPU (
    UINT32 tpu_id)
{
  SYS_CommittedInsts(0) += my_tpu[tpu_id]->commit(SYS_Cycle());
}

void 
ASIM_APE_CLASS::abortAndRestartTPU (
    UINT32 tpu_id)
{
  my_tpu[tpu_id]->abort(SYS_Cycle());
}

void 
ASIM_APE_CLASS::abortAndRestartTPU (
    UINT32 tpu_id,
    IADDR_CLASS pc)
{
  ASSERTX(0);
}
