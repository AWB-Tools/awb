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

#ifndef _SYSTEM_MINI_
#define _SYSTEM_MINI_

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/except.h"
#include "asim/module.h"
#include "asim/config.h"
#include "asim/state.h"

/*
 * ASIM_SYSTEM
 *
 * The entire system being simulated.
 */
typedef class ASIM_SYSTEM_CLASS *ASIM_SYSTEM;
class ASIM_SYSTEM_CLASS : public ASIM_MODULE_CLASS
{
    private:
        /*
         * System wide counters for cycles and committed instructions.
         */

        UINT64 cycles;
        UINT64 *committed;	// indexed by cpu number
        UINT64 num_cpus;
        UINT64 *macroCommitted;	// indexed by cpu number
        bool hasMicroOps;


    protected:
        // Simulator configuration
        ASIM_CONFIG config;

        // 
        // Local cycle counter. We need this one to count cycles that
        // happen when stats are being collected (the cycle counter in
        // asimSystem' is always counting.
        UINT64 statCycles;

       // True if the system has been interrupted by the controller during
       // this cycle.
       volatile bool sysBreak;

    public:
        ASIM_SYSTEM_CLASS (const char *n, UINT16 ncpu = 1, ASIM_EXCEPT e =NULL);
        ~ASIM_SYSTEM_CLASS ();

        friend ASIM_SYSTEM SYS_Init(UINT32 argc, char *argv[]);

        void DumpStats(STATE_OUT state_out);

        bool InitModule();
        bool SetHasMicroOps(bool val) { hasMicroOps = val; }

        // Interface functions. The following functions represent
        // a performance models external interface.
       friend void SYS_Usage(FILE * file);


        const UINT32 NumCpus()	{ return num_cpus; }

        /*
         * Pass control to the performance model until cycle 'stopCycle'
         * or until committed instruction 'stopInst' or until commiting
         * marker 'markerID'.
         * Return false if execution is stopped (via SYS_Break()) before
         * we reach 'stopCycle', 'stopInst' or 'markerID'.
         */
        bool SYS_Execute (UINT64 stopCycle, UINT64 stopInst, UINT64 stopMacroInst, UINT64 stopPacket=0);

        UINT64& SYS_Cycle (void) { return(cycles); }
        UINT64& SYS_CommittedInsts (UINT32 cpunum) { return(committed[cpunum]); }
        UINT64* SYS_CommittedInsts() { return committed; }

        UINT64& SYS_CommittedMacroInsts (UINT32 cpunum) 
            { if (hasMicroOps)
                return (macroCommitted[cpunum]);
            else
                return  (committed[cpunum])
            }
        UINT64* SYS_CommittedMacroInsts () 
            { if (hasMicroOps)
                return (macroCommitted);
            else
                return  (committed)
            }

	UINT64  SYS_GlobalCommittedInsts()	
	{
	    UINT64 inst = 0; 

	    for (UINT32 i = 0; i < num_cpus; i++)
	    {	
		inst += committed[i]; 
	    }

	    return inst; 
	}
};


/*
 * Initialize the system, returning a pointer to it.
 */
extern ASIM_SYSTEM SYS_Init (UINT32 argc, char *argv[], char *envp[]);

/*
 * Print command-line usage for the system.
 */
extern void SYS_Usage (FILE *file);


#endif /* _SYSTEM_ */
