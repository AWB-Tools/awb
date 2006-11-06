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
 * @brief ASIM Tarati Service: Time Scheduler
 */

// generic

// ASIM local modules
#include "taratiTimeScheduler.h"

namespace AsimTarati {

//----------------------------------------------------------------------------
// Tarati Services
//----------------------------------------------------------------------------
TimeScheduler::TimeScheduler(
    Server * server)
  : Service(server, "TimeScheduler", "0.1")
{
    method.hello = new Hello(this);
}

TimeScheduler::~TimeScheduler()
{
    delete method.hello;
}

//----------------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------------
//----- Hello -----
void
TimeScheduler::Hello::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    result = "Hello from TimeScheduler";
}

} // namespace AsimTarati
