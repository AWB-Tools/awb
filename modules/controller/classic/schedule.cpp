/*
 * Copyright (c) Compaq Computer Corporation, 1999
 * Copyright (C) 2002-2006 Intel Corporation
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @author David Goodwin
 * @brief
 */

// generic
#include <iostream>

// ASIM core
#include "asim/syntax.h"

// ASIM public modules
#include "asim/provides/controller.h"

/*******************************************************************/


CMD_SCHEDULE_CLASS::CMD_SCHEDULE_CLASS ()
/*
 * Initialize...
 */
{
    cycleList = new CMD_WORKLIST_CLASS;
    nanosecondList = new CMD_WORKLIST_CLASS;
    instList = new CMD_WORKLIST_CLASS;
    macroInstList = new CMD_WORKLIST_CLASS;

    packetList = new CMD_WORKLIST_CLASS;
}

CMD_SCHEDULE_CLASS::~CMD_SCHEDULE_CLASS ()
/*
 * Destruct
 */
{
    if (cycleList) {
        delete cycleList;
    }
    if (nanosecondList) {
        delete nanosecondList;
    }    
    if (instList) {
        delete instList;
    }
    if (packetList) {
        delete packetList;
    }
    if(macroInstList){
        delete macroInstList;
    }
}


CMD_WORKITEM
CMD_SCHEDULE_CLASS::ReadyEvent (UINT64 currentNanosecond, UINT64 currentCycle, UINT64 currentInst, UINT64 currentMacroInst, UINT64 currentPacket)
/*
 * Return the next event item that should be handled. Return NULL
 * if there is no action ready.
 */
{
    //
    // 'cycleList' is ordered so we only need to read the head item.

    CMD_WORKITEM head = cycleList->Head();
    if (head != NULL)
    {
        if ((head->Trigger() == ACTION_NOW)
            || (((head->Trigger() == ACTION_CYCLE_ONCE)
                 || (head->Trigger() == ACTION_CYCLE_PERIOD))
                && (head->ReadyCycle() == currentCycle)))
        {
            CMD_WORKITEM ht = cycleList->Remove();
            VERIFYX(ht == head);
            return(head);
        }

        VERIFYX(((head->Trigger() == ACTION_CYCLE_ONCE)
                 || (head->Trigger() == ACTION_CYCLE_PERIOD))
                && (head->ReadyCycle() > currentCycle));
    }

    //
    // 'nanosecondList' is ordered so we only need to read the head item.

    head = nanosecondList->Head();
    if (head != NULL)
    {
        if ((head->Trigger() == ACTION_NOW)
            || (((head->Trigger() == ACTION_NANOSECOND_ONCE)
                 || (head->Trigger() == ACTION_NANOSECOND_PERIOD))
                && (head->ReadyNanosecond() == currentNanosecond)))
        {
            CMD_WORKITEM ht = nanosecondList->Remove();
            VERIFYX(ht == head);
            return(head);
        }

        VERIFYX(((head->Trigger() == ACTION_NANOSECOND_ONCE)
                 || (head->Trigger() == ACTION_NANOSECOND_PERIOD))
                && (head->ReadyNanosecond() > currentNanosecond));
    }    
    
    //
    // 'instList' is ordered so we only need to read the head item.

    head = instList->Head();
    if (head != NULL)
    {
        
        if (((head->Trigger() == ACTION_INST_ONCE) ||
             (head->Trigger() == ACTION_INST_PERIOD))
            && (head->ReadyInst() <= currentInst))
        {
            CMD_WORKITEM ht = instList->Remove();
            VERIFYX(ht == head);
            return(head);
        }

        VERIFYX(((head->Trigger() == ACTION_INST_ONCE) ||
                 (head->Trigger() == ACTION_INST_PERIOD))
                && (head->ReadyInst() > currentInst));
    }
    //
    // 'macroInstList' is ordered so we only need to read the head item.

    head = macroInstList->Head();
    if (head != NULL)
    {
        
        if (((head->Trigger() == ACTION_MACROINST_ONCE) ||
             (head->Trigger() == ACTION_MACROINST_PERIOD))
            && (head->ReadyMacroInst() <= currentMacroInst))
        {
            CMD_WORKITEM ht = macroInstList->Remove();
            VERIFYX(ht == head);
            return(head);
        }

        VERIFYX(((head->Trigger() == ACTION_MACROINST_ONCE) ||
                 (head->Trigger() == ACTION_MACROINST_PERIOD))
                && (head->ReadyMacroInst() > currentMacroInst));
    }
    //
    // 'packetList' is ordered so we only need to read the head item.

    head = packetList->Head();
    if (head != NULL)
    {
        
        if (((head->Trigger() == ACTION_PACKET_ONCE))
            && (head->ReadyPacket() <= currentPacket))
        {
            CMD_WORKITEM ht = packetList->Remove();
            VERIFYX(ht == head);
            return(head);
        }

        VERIFYX(((head->Trigger() == ACTION_PACKET_ONCE))
                && (head->ReadyPacket() > currentPacket));
    }

    return(NULL);
}


void
CMD_SCHEDULE_CLASS::Schedule (CMD_WORKITEM item, UINT64 currentNanosecond, UINT64 currentCycle, UINT64 currentInst, UINT64 currentMacroInst, UINT64 currentPacket)
/*
 * Schedule 'item' as specified by its trigger.
 */
{
    CMD_ACTIONTRIGGER trig = item->Trigger();
    
    //
    // We should never be asked to schedule an ACTION_NEVER item.

    VERIFY(trig != ACTION_NEVER, "Can't schedule ACTION_NEVER item.\n");

    //
    // For periodic actions, set the action's cycle or instruction using
    // the action's period and the current cycle or instruction. We align
    // all periodic actions.

    if (trig == ACTION_CYCLE_PERIOD)
    {
        item->ReadyCycle() = ((currentCycle + 1 + item->Period()) /
                              item->Period()) * item->Period();
        ASSERTX(item->ReadyCycle() != currentCycle);
    }
    else if (trig == ACTION_INST_PERIOD)
    {
        item->ReadyInst() = ((currentInst + 1 + item->Period()) /
                              item->Period()) * item->Period();
        ASSERTX(item->ReadyInst() != currentInst);
    }
    else if (trig == ACTION_MACROINST_PERIOD)
    {
        item->ReadyMacroInst() = ((currentMacroInst + 1 + item->Period()) /
                                  item->Period()) * item->Period();
        ASSERTX(item->ReadyMacroInst() != currentMacroInst);
    }
    else if (trig == ACTION_NANOSECOND_PERIOD)
    {
        item->ReadyNanosecond() = ((currentNanosecond + item->Period()) /
                              item->Period()) * item->Period();
        ASSERTX(item->ReadyNanosecond() != currentNanosecond);        
    }
    
    //
    // Put action on appropriate list.

    if ((trig == ACTION_NOW) || (trig == ACTION_CYCLE_ONCE) || (trig == ACTION_CYCLE_PERIOD))
    {
        cycleList->InsertOrdered(item);
    }
    else if ((trig == ACTION_INST_ONCE) || (trig == ACTION_INST_PERIOD))
    {
        instList->InsertOrdered(item);
    }
    else if ((trig == ACTION_MACROINST_ONCE) || (trig == ACTION_MACROINST_PERIOD))
    {
        macroInstList->InsertOrdered(item);
    }
    else if ((trig == ACTION_NANOSECOND_ONCE) || (trig == ACTION_NANOSECOND_PERIOD))
    {
        nanosecondList->InsertOrdered(item);
    }
    else
    {
        VERIFYX((trig == ACTION_PACKET_ONCE));
        packetList->InsertOrdered(item);
    }
}


UINT64
CMD_SCHEDULE_CLASS::CycleForNextEvent (UINT64 currentCycle)
/*
 * Return the cycle in which the next event should occur.
 */
{
    //
    // 'cycleList' is ordered so we only need to read the head item.

    CMD_WORKITEM head = cycleList->Head();
    if (head == NULL)
        return(UINT64_MAX);
    else if (head->Trigger() == ACTION_NOW)
        return(currentCycle);

    VERIFYX((head->Trigger() == ACTION_CYCLE_ONCE) || (head->Trigger() == ACTION_CYCLE_PERIOD));
    return(head->ReadyCycle());
}


UINT64
CMD_SCHEDULE_CLASS::InstForNextEvent (UINT64 currentInst)
/*
 * Return the number of committed instructions at which the next event
 * should occur.
 */
{
    //
    // 'instList' is ordered so we only need to read the head item.

    CMD_WORKITEM head = instList->Head();
    if (head == NULL)
        return(UINT64_MAX);

    VERIFYX((head->Trigger() == ACTION_INST_ONCE) || (head->Trigger() == ACTION_INST_PERIOD));
    return(head->ReadyInst());
}

UINT64
CMD_SCHEDULE_CLASS::MacroInstForNextEvent (UINT64 currentMacroInst)
/*
 * Return the number of committed instructions at which the next event
 * should occur.
 */
{
    //
    // 'macroinstList' is ordered so we only need to read the head item.

    CMD_WORKITEM head = macroInstList->Head();
    if (head == NULL)
        return(UINT64_MAX);

    VERIFYX((head->Trigger() == ACTION_MACROINST_ONCE) || (head->Trigger() == ACTION_MACROINST_PERIOD));
    return(head->ReadyMacroInst());
}

UINT64
CMD_SCHEDULE_CLASS::PacketForNextEvent (UINT64 currentPacket)
/*
 * Return the number of received packets at which the next event
 * should occur.
 */
{
    //
    // 'packetList' is ordered so we only need to read the head item.

    CMD_WORKITEM head = packetList->Head();
    if (head == NULL)
        return(UINT64_MAX);

    VERIFYX((head->Trigger() == ACTION_PACKET_ONCE));
    return(head->ReadyPacket());
}


UINT64
CMD_SCHEDULE_CLASS::NanosecondForNextEvent (UINT64 currentNanosecond)
/*
 * Return the nanosecond in which the next event should occur.
 */
{
    //
    // 'nanosecondList' is ordered so we only need to read the head item.

    CMD_WORKITEM head = nanosecondList->Head();
    if (head == NULL)
        return(UINT64_MAX);
    else if (head->Trigger() == ACTION_NOW)
        return(currentNanosecond);

    VERIFYX((head->Trigger() == ACTION_NANOSECOND_ONCE) || (head->Trigger() == ACTION_NANOSECOND_PERIOD));
    return(head->ReadyNanosecond());
}


void
CMD_SCHEDULE_CLASS::ClearCycleProgress (void)
{
    cycleList->ClearProgress();
}

void
CMD_SCHEDULE_CLASS::ClearInstProgress (void)
{
    instList->ClearProgress();
}

void
CMD_SCHEDULE_CLASS::ClearMacroInstProgress (void)
{
    macroInstList->ClearProgress();
}
void
CMD_SCHEDULE_CLASS::ClearPacketProgress (void)
{
    packetList->ClearProgress();
}

void
CMD_SCHEDULE_CLASS::ClearNanosecondProgress (void)
{
    nanosecondList->ClearProgress();
}

