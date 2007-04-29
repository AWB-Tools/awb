/*****************************************************************************
 *
 * @brief Base class for EVENTS mecanism 
 *
 * @author Roger Gramunt 
 *
 *Copyright (C) 2003-2006 Intel Corporation
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

#include "asim/event.h"
#include "asim/smp.h"

// what is this for?
bool eventsOn = false;

bool runWithEventsOn = false;

bool firstTurnedOn = true;

DRAL_SERVER ASIM_DRAL_EVENT_CLASS::event;

// a global object that can destruct things at program termination
ASIM_DRAL_EVENT_CLASS dralCleaner;

/**
 * initialize variables
 */
ASIM_DRAL_EVENT_CLASS::ASIM_DRAL_EVENT_CLASS()
{
    event = NULL;
}

/**
 * free allocated memory
 */
ASIM_DRAL_EVENT_CLASS::~ASIM_DRAL_EVENT_CLASS()
{
    if (event)
    {
        delete event;
    }
    event = NULL;
}

void
ASIM_DRAL_EVENT_CLASS::InitEvent()
{
    if (ASIM_SMP_CLASS::GetMaxThreads() > 1)
    {
        static ATOMIC_INT32 warned(0);
        if (0 == warned++)
        {
            cerr << "DRAL Events unavailable in multi-threaded runs" << endl;
        }
    }
    else
    {
        runWithEventsOn = true;

        event = new DRAL_SERVER_CLASS("Events",1024,true);
        //eventsOn = false;
        // Send an event creating 
        // a fake node with number 0. This is necessary because port.h will connect to node 0
        // those ports whose origin or destination is unknown to "Fake Node".
        event->NewNode("Fake_Node", 0);
    }
}

/*
 * Display ID generation support.
 */
UINT64
GetUniqueDisplayId(void)
{
    static UINT64 displayId = 1;
    
    return displayId++;
} 
