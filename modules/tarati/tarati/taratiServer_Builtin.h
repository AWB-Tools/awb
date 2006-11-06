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

#ifndef _TARATI_SERVER_BUILTIN_
#define _TARATI_SERVER_BUILTIN_ 1

// generic
#include <string>
#include <map>

// ASIM local module
#include "taratiServer.h"
#include "taratiService.h"
#include "taratiMethod.h"

namespace Tarati {
namespace ServerBuiltin {

//----------------------------------------------------------------------------
// ASIM Tarati Service: Server
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Services
//----------------------------------------------------------------------------
/**
 * @brief Tarati Service of the server proper
 */
class Server
  : public Service
{
  private:
    static char * const name    = "Server";
    static char * const version = "0.1";

    //------------------------------------------------------------------------
    // Methods
    //------------------------------------------------------------------------
    /**
     * @brief Tarati Server directory of registered services
     */
    class ServiceDirectory
      : public Method
    {
      public:
        ServiceDirectory(Service * service)
          : Method(service, "ServiceDirectory") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /**
     * @brief Tarati Server Rusage
     */
    class Rusage
      : public Method
    {
      public:
        Rusage(Service * service)
          : Method(service, "Rusage") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    struct _method {
      ServiceDirectory * directory;
      Rusage * rusage;
    } method;

  public:
    Server(Tarati::Server * server);
    ~Server();
};

} // namespace ServerBuiltin
} // namespace Tarati

#endif // _TARATI_SERVER_BUILTIN_
