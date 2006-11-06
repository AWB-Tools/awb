/*
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
/**
 * @file
 * @author David Goodwin
 * @brief Base class for ASIM system top-level object.
 */

#ifndef _BASESYSTEM_
#define _BASESYSTEM_

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/except.h"
#include "asim/module.h"
#include "asim/state.h"
#include "asim/thread.h"

// ASIM public modules -- BAD! in asim-core
#include "asim/provides/isa.h"

/*
 * ASIM_SYSTEM
 *
 * The entire system being simulated.
 */
typedef class CONTEXT_SCHEDULER_CLASS *CONTEXT_SCHEDULER;
typedef class WARMUP_MANAGER_CLASS *WARMUP_MANAGER;


typedef class ASIM_SYSTEM_CLASS *ASIM_SYSTEM;
class ASIM_SYSTEM_CLASS : public ASIM_MODULE_CLASS
{
    private:
        /*
         * System wide counters for cycles and committed instructions.
         */

        UINT64 receivedPkt; // for the network simulator
        UINT64 cycles;      // When using clockserver, number of clockserver clocks called
        UINT64 base_cycles; // When using clockserver, real base frequency cycle
        UINT64 *committed;	// indexed by cpu number
        UINT64 committedMarkers;
        UINT64 *macroCommitted;	// indexed by cpu number
        bool hasMicroOps;


        UINT64 nonDrainCycles; /* cycles not counting the time that the pipeline is being drained inbetween samples */
        bool pipeDraining;     /* is the pipeline being drained */

        /*
         * System wide counter indicating total number of cpu's in the system. This counter usually gets set
         * from files that "%provide system", such as uni.h and multi.h. In the uni.h case, the default value
         * of '1' is used, while multi.h sets this field to NUM_CPUS
         */
        UINT32 num_cpus;

        /*
         * Some models may vary the number of structures based on the number
         * of feeder threads.
         */
        const UINT32 num_feeder_threads;

        /// Initialize DRAL event stream
        void InitEvents (void);
        
    protected:
    
        INT32 commitWatchMarker;
        
        ASIM_MODULE *cpu2module;    // indexed by cpu number

    public:
        ASIM_SYSTEM_CLASS(
            const char *n,
            UINT16 ncpu = 1,
            ASIM_EXCEPT e = NULL,
            UINT32 feederThreads = 0);
        virtual ~ASIM_SYSTEM_CLASS () { delete [] committed; delete [] macroCommitted; delete [] cpu2module; }

        UINT32 NumCpus() const { return num_cpus; }
        UINT32 NumFeederThreads() const { return num_feeder_threads; };

        /*
         * Interface functions. The following functions represent
         * a performance models external interface.
         *
         * Start a 'thread' running on the performance model.
         * Return false if unable to start running thread.
         */
        virtual bool SYS_ScheduleThread (ASIM_THREAD thread) =0;

        /*
         * Stop and remove a 'thread' from the performance model.
         * Return false is unable to remove thread.
         */
        virtual bool SYS_UnscheduleThread (ASIM_THREAD thread) =0;
        /*
         * Block a 'thread' on the performance model.
         * Return false if unable to block thread.
         */
        virtual bool SYS_BlockThread (ASIM_THREAD thread, ASIM_INST) =0;
        /*
         * Unblock a 'thread' in the performance model.
         * Return false is unable to un-block thread.
         */
        virtual bool SYS_UnblockThread (ASIM_THREAD thread) =0;

        virtual bool SYS_UnhookAllThreads()=0;
        virtual bool SYS_HookAllThreads()=0;
        void SYS_beginDrain()
        {
          pipeDraining = true;
          SYS_UnhookAllThreads(); /* unhook all threads before draining the pipe */
        }
        void SYS_endDrain()
        {
          pipeDraining = false;
          SYS_HookAllThreads(); /* hook back all threads after draining the pipe */
        }

        virtual bool SYS_IsCpuActive(UINT32 cpunum) const {return true;}; /* Tells if a cpu is active */
        /* Do not count CommittedInsts and nonDrainCycles while the pipe is being drained */
        void inc_CommittedInsts(UINT32 cpunum) { if (! pipeDraining) {++committed[cpunum];}}
        void inc_nonDrainCycles() { if (! pipeDraining) {++nonDrainCycles;}}

        /*
         * Pass control to the performance model until cycle 'stopCycle'
         * or until committed instruction 'stopInst' or until commiting
         * marker 'markerID'.
         * Return false if execution is stopped (via SYS_Break()) before
         * we reach 'stopCycle', 'stopInst' or 'markerID'.
         */
        virtual bool SYS_Execute (UINT64 stopNanosecond, UINT64 stopCycle, UINT64 stopInst,UINT64 stopMacroInst, UINT64 stopPacket=0) =0; //for the network simulator

        /*
         * Interrupt the system, so that it will return control to the
         * controller at the end of the current cycle. Also method to clear
         * the interrupt.
         */
        virtual void SYS_Break (void) =0;
        virtual void SYS_ClearBreak (void) =0;

        /*
         * Return the current cycle, or the number of committed
         * instructions.
         */
        virtual UINT64& SYS_Cycle (void) { return(cycles); }
        virtual UINT64& SYS_BaseCycle (void) { return(base_cycles); }        
        virtual UINT64 SYS_Cycle (UINT32 cpunum) { return cycles; }        

        UINT64& SYS_CommittedInsts (UINT32 cpunum) { return(committed[cpunum]); }
        UINT64* SYS_CommittedInsts () { return committed; }

        UINT64& SYS_CommittedMacroInsts (UINT32 cpunum) 
        { 
            if (hasMicroOps)
            {
                return (macroCommitted[cpunum]);
            }
            else
            {
                return  (committed[cpunum]);
            }
        }

        UINT64* SYS_CommittedMacroInsts () 
        { 
            if (hasMicroOps)
            {
                return (macroCommitted);
            }
            else
            {
                return  (committed);
            }
        }

        UINT64  SYS_GlobalCommittedMacroInsts()
        {           
            if (!hasMicroOps)
            {
                return SYS_GlobalCommittedInsts();
            }
            else
            {
                UINT64 inst = 0;
                
                for (UINT32 i = 0; i < num_cpus; i++)
                {
                    inst += macroCommitted[i];
                }                
                return inst;
            }
        }

        void SYS_SetCpu2Module (UINT32 cpunum, ASIM_MODULE module) { cpu2module[cpunum] = module; }

        UINT64 SYS_nonDrainCycles (void) { return nonDrainCycles; }

        UINT64  SYS_GlobalCommittedInsts()	
        {
            UINT64 inst = 0; 
    
            for (UINT32 i = 0; i < num_cpus; i++)
            {	
                inst += committed[i]; 
            }
    
            return inst; 
        }

        UINT64& SYS_CommittedMarkers (void) { return(committedMarkers); }
        INT32& SYS_CommitWatchMarker (void) { return(commitWatchMarker); }
        
        virtual UINT64 SYS_Nanosecond (void) { return 0; }
        
        // for the network simulator
        UINT64& SYS_ReceivedPkt(void)  { return(receivedPkt); }

        //for the multithreaded sim
        virtual void SYS_StopPThreads() {}

        /// overloaded InitModule to also init other local things
        virtual bool InitModule (void);
        virtual void SYS_CanHaveMicroOps(bool val) { hasMicroOps = val; }

        /*
         * The following objects may be defined by a system.  Callers
         * must check for NULL in case the system doesn't define them.
         */
        virtual CONTEXT_SCHEDULER GetContextScheduler(void) { return NULL; }
        virtual WARMUP_MANAGER GetWarmupManager(void) { return NULL; }
        
};


/*
 * Initialize the system, returning a pointer to it.
 *
 * feederThreads is the number of threads created by the feeder.  Some
 * models may wish to size structures dynamically based on the number
 * of threads that will run.
 */
extern ASIM_SYSTEM SYS_Init(
    UINT32 argc,
    char *argv[],
    char *envp[],
    UINT32 feederThreads);

/*
 * Print command-line usage for the system.
 */
extern void SYS_Usage (FILE *file);


#endif /* _SYSTEM_ */
