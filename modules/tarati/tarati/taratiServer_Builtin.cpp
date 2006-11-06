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
 * @brief Tarati Server Built-in Services and Methods
 */

// generic
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string.h>

// ASIM local module
#include "taratiServer_Builtin.h"

namespace Tarati {
namespace ServerBuiltin {

//----------------------------------------------------------------------------
// Services
//----------------------------------------------------------------------------
Server::Server(Tarati::Server * server)
  : Service(server, name, version)
{
    method.directory = new ServiceDirectory(this);
    method.rusage = new Rusage(this);
}

Server::~Server()
{
    delete method.directory;
    delete method.rusage;
}

//----------------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------------
//----- ServiceDirectory -----
void
Server::ServiceDirectory::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    const Service * const service = GetService();
    const Tarati::Server * const server = service->GetServer();
    Tarati::Server::ServiceMap services = server->GetServices();

    int count = 0;
    for (Tarati::Server::ServiceMap::const_iterator it = services.begin();
         it != services.end();
         ++it)
    {
        result[count++] = it->first;
    }
}

//----- Rusage -----
void
Server::Rusage::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
  struct rusage usage;

  if (getrusage (RUSAGE_SELF, &usage) == 0) {
    result["user_time"] =
      usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
    result["system_time"] =
      usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
  } else {
    result["error"] = strerror(errno);
  }
}

} // namespace ServerBuiltin
} // namespace Tarati
