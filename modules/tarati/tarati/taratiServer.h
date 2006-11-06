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
 * @brief Implementation of Tarati Server (ASIM external connection)
 */

#ifndef _TARATI_SERVER_
#define _TARATI_SERVER_ 1

// generic
#include <string>
#include <map>

// XML-RPC low level transport library
#include <asim/provides/xmlrpc.h>

using namespace XmlRpc;
using namespace std;

namespace Tarati {

// forward declarations to avoid circular dependencies
class Service;
class Method;
namespace ServerBuiltin {
  class Server;
}

//----------------------------------------------------------------------------
// Server proper
//----------------------------------------------------------------------------
/**
 * @brief Tarati Server Object
 *
 * This class ...
 */
class Server
{
  public:
    // types
    typedef map<string, Service *> ServiceMap;

  private:
    // members
    // -- XML-RPC
    XmlRpcServer * xmlrpcServer;  ///< XML-RPC transport layer server
    // -- registered services 
    ServiceMap services;           ///< registered service to object map

    struct _service {
      ServerBuiltin::Server * server; ///< built-in server service
    } service;

  public:
    // constructors / destructors
    Server();
    ~Server();

    // registration methods
    void ServiceRegister (Service * service);
    void ServiceUnregister (Service * service);
    void MethodRegister(Method * method);
    void MethodUnregister(Method * method);

    // accessors / modifiers
    /// Accessor for registered services
    const ServiceMap & GetServices (void) const { return services; }
    int GetPort (void) const { return xmlrpcServer->getPort(); }

    // other methods
    /// check if there is work and do it
    void Work (double timeout = 0.0);
    /// wait for work to become available
    void Wait (void);
    /// Generate the low-level XML server name for a method call
    const string GenerateMethodName (Method * method) const;
};

} // namespace Tarati

#endif // _TARATI_SERVER_
