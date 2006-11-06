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
 * @brief ASIM Tarati Service: Global State
 */

// ASIM core
#include "asim/syntax.h"

// ASIM local modules
#include "taratiGlobalState.h"

namespace AsimTarati {

//----------------------------------------------------------------------------
// Tarati Services
//----------------------------------------------------------------------------
GlobalState::GlobalState(
    Server * server,
    ASIM_SYSTEM system)
  : Service(server, name, version)
{
    method.uptime = new Uptime(this, system);
    method.hello = new Hello(this);
}

GlobalState::~GlobalState()
{
    delete method.uptime;
    delete method.hello;
}

//----------------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------------
//----- Uptime -----
void
GlobalState::Uptime::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    result["cycles"] = (int) system->SYS_Cycle();
    result["instructions"] = (int) system->SYS_GlobalCommittedInsts();
}

//----- Hello -----
void
GlobalState::Hello::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    result = "Hello from GlobalState";
}

} // namespace AsimTarati
