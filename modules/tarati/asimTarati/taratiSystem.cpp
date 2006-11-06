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

// generic

// ASIM public modules

// ASIM local modules
#include "taratiSystem.h"

namespace AsimTarati {

//----------------------------------------------------------------------------
// Tarati - System interaction
//----------------------------------------------------------------------------
System::System(ASIM_SYSTEM system)
{
    this->system = system;

    // create a tarati server object
    server = new Server;

    // add ASIM tarati services
    service.tclBeamer = new TclBeamer(server);
    service.globalState = new GlobalState(server, system);
    service.stats = new Stats(server, system);
    service.timeScheduler = new TimeScheduler(server);
}

System::~System()
{
    // delete services
    delete service.tclBeamer;
    delete service.globalState;
    delete service.stats;
    delete service.timeScheduler;

    // delete server
    delete server;
}

} // namespace AsimTarati
