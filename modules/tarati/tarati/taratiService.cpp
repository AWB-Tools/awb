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
 * @brief Implementation of Tarati Service (externally visible service)
 */

// generic

// ASIM local module
#include "taratiServer.h"
#include "taratiService.h"

namespace Tarati {

/**
 * Create a new Tarati Service object and register it with the server.
 * Also instantiate and register all built-in Service Methods.
 */
Service::Service (
    Server * _server,
    const string & _name,
    const string & _version)
  : server (_server),
    name(_name),
    version(_version)
{
    server->ServiceRegister(this);

    // Service's built-in Methods
    method.directory = new MethodDirectory(this);
    method.version = new Version(this);
}

/**
 * Delete and cleanup
 */
Service::~Service ()
{
    // unregister all buil-in and potentially other Methods that are still
    // registered with the Server at this time;
    MethodUnregisterAll();

    // free allocated objects
    delete method.directory;
    delete method.version;

    // unregister the Service
    server->ServiceUnregister(this);
}

/**
 * Register a new method
 */
void
Service::MethodRegister (
    Method * method)
{
    // register the method locally
    methods[method->GetName()] = method;

    // register the method directly with the server to be called directly
    server->MethodRegister(method);
}

/**
 * Unregister an existing method
 */
void
Service::MethodUnregister (
    Method * method)
{
    // unregister locally
    MethodMap::iterator it = methods.find(method->GetName());
    if (it != methods.end()) {
        methods.erase(it);
    } else {
      // is this a bug?
    }

    // unregister the method from server
    server->MethodUnregister (method);
}

/**
 * Unregister all existing methods
 */
void
Service::MethodUnregisterAll (void)
{
    int count = 0;
    for (MethodMap::iterator it = methods.begin();
         it != methods.end();
         ++it)
    {
        // unregister the method from server
        Method * method = it->second;
        server->MethodUnregister (method);
    }
    
    // unregister locally
    methods.clear();
}

//----------------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------------
//----- MethodDirectory -----
void
Service::MethodDirectory::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    int count = 0;
    for (MethodMap::const_iterator it = service->methods.begin();
         it != service->methods.end();
         ++it)
    {
        result[count++] = it->first;
    }
}

//----- Version -----
void
Service::Version::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    result = service->version;
}

} // namespace Tarati
