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

/*
 * *****************************************************************
 * *                                                               *

/**
 * @file
 * @author David Goodwin
 * @brief
 */


#ifndef _schedule_h_
#define _schedule_h_

// ASIM core
#include "asim/syntax.h"

// ASIM public modules
#include "asim/provides/controller.h"

typedef class CMD_SCHEDULE_CLASS *CMD_SCHEDULE;

/*
 * CMD_SCHEDULE
 *
 * Schedule of events and the cycle or committed instruction that they
 * should occur at.
 */
class CMD_SCHEDULE_CLASS
{
    private:
        /*
         * Four lists of work items containing actions waiting for a
         * certain cycle, a certain number of committed instructions,
         * certain number of nanoseconds and packets. All of them are sorted.
         */        
        CMD_WORKLIST cycleList;
        CMD_WORKLIST instList;
        CMD_WORKLIST nanosecondList;        
        CMD_WORKLIST packetList;
        CMD_WORKLIST macroInstList;

        
    public:
        // constructors / destructors
        CMD_SCHEDULE_CLASS ();
        ~CMD_SCHEDULE_CLASS ();

        /*
         * Return the next event item that should be handled. Return NULL
         * if there is no action ready.
         */
        CMD_WORKITEM ReadyEvent (UINT64 currentNanosecond, UINT64 currentCycle, UINT64 currentInst, UINT64 currentMacroInst, UINT64 currentPacket);

        /*
         * Schedule 'item' as specified by 'trig' and 'cnt'.
         */
        void Schedule (CMD_WORKITEM item, UINT64 currentNanosecond, UINT64 currentCycle, UINT64 currentInst, UINT64 currentMacroInst, UINT64 currentPacket);

        /*
         * Return the cycles or number of committed instructions at which
         * the next event should occur.
         */
        UINT64 NanosecondForNextEvent (UINT64 currentNanosecond);
        UINT64 CycleForNextEvent (UINT64 currentCycle);
        UINT64 InstForNextEvent (UINT64 currentInst);
        UINT64 PacketForNextEvent (UINT64 currentInst);
        UINT64 MacroInstForNextEvent (UINT64 currentMacroInst);

        /*
         * Clear progress work items from the schedule.
         */
        void ClearNanosecondProgress (void);
        void ClearCycleProgress (void);
        void ClearInstProgress (void);
        void ClearPacketProgress (void);
        void ClearMacroInstProgress (void);

};

#endif
