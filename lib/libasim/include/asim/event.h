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


#ifndef _EVENT_
#define _EVENT_

// ASIM core
#include "asim/syntax.h"
#include "asim/dralServer.h"

using namespace std;

/*
 * If events are not enabled, then no event method invocations will
 * be compiled into the model.
 */
#ifdef ASIM_ENABLE_EVENTS
#define DRALEVENT(E)          ASIM_DRAL_EVENT_CLASS::event->E
#define DRALEVENT_GUARDED(E)  if (ASIM_DRAL_EVENT_CLASS::event) {ASIM_DRAL_EVENT_CLASS::event->E;}
#define EVENT(E)        E
#else
#define DRALEVENT(E)
#define DRALEVENT_GUARDED(E)
#define EVENT(E)
#endif

// FEDE: IN ORDER TO AVOID REPLICATING OCCUPANCY STRINGS WE DEFINE THEM HERE
#define DRAL1_STD_OCCUPANCY_TAG "_OCCUPANCY"
#define DRAL2_STD_OCCUPANCY_TAG "OCCUPANCY"

extern bool eventsOn; 
extern bool runWithEventsOn;
extern bool firstTurnedOn;

/*
 * Display ID generation support.
 *
 * We need a GetUniqueDisplayId() function to globally return new, unique
 * display ids to be used in the performance model. This has nothing to do
 * with DRAL, so I've decided to put it here, outside the DRAL definition
 * and implementation classes --- Julio Gago @ BSSAD.
 *
 */
UINT64 GetUniqueDisplayId(void);

/*
 * Class ASIM_EVENT
 *
 * Base class for event management.
 *
 */
typedef class ASIM_DRAL_EVENT_CLASS *ASIM_DRAL_EVENT;
class ASIM_DRAL_EVENT_CLASS 
{
  public:
    // members
    static DRAL_SERVER event;

    // constructors / destructors
    ASIM_DRAL_EVENT_CLASS();
    ~ASIM_DRAL_EVENT_CLASS();

    static void InitEvent();
};

#endif /* _EVENT_ */
