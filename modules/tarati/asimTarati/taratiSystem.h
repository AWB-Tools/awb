/**************************************************************************
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

/**
 * @file
 * @author Artur Klauser
 * @brief Tarati server and services registration for ASIM models
 */

#ifndef _TARATI_SYSTEM_
#define _TARATI_SYSTEM_

// generic

// ASIM public modules
#include "asim/provides/system.h"
#include "asim/provides/tarati.h"
#include "asim/provides/tarati_tcl_beamer.h"
#include "asim/provides/tarati_global_state.h"
#include "asim/provides/tarati_stats.h"
#include "asim/provides/tarati_time_scheduler.h"

using namespace Tarati;

namespace AsimTarati {

//----------------------------------------------------------------------------
// Tarati - ASIM System interaction
//----------------------------------------------------------------------------
class System
{
  private:
    // members
    ASIM_SYSTEM system;
    Server * server;
    struct Service {
        TclBeamer * tclBeamer;
        GlobalState * globalState;
        Stats * stats;
        TimeScheduler * timeScheduler;
    } service;

  public:
    // methods
    System(ASIM_SYSTEM system);
    ~System();
    Server * GetServer(void) const { return server; }
    int GetPort(void) const { return server->GetPort(); }
    void Work(double timeout = 0.0) const { server->Work(timeout); }
    void Wait(void) const { server->Wait(); }
};

} // namespace AsimTarati

#endif /* _TARATI_SYSTEM_ */
