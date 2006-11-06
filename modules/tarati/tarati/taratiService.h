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

#ifndef _TARATI_SERVICE_
#define _TARATI_SERVICE_ 1

// generic
#include <string>
#include <iostream>

// XML-RPC low level transport library
#include "asim/provides/xmlrpc.h"

// ASIM local module
#include "taratiMethod.h"

using namespace XmlRpc;
using namespace std;

namespace Tarati {

// forward declarations to avoid circular dependencies
class Server;
class TaratiService_Directory;
class TaratiService_Version;

/**
 * @brief Tarati Service Object
 *
 * This class ...
 */
class Service
{
  private:
    // types
    typedef map<string, Method *> MethodMap;

    // members
    Server * const server; ///< server this service belongs to
    const string name;           ///< name of this service
    MethodMap methods;           ///< registered method to object map
    const string version;        ///< version number service interface

    //------------------------------------------------------------------------
    // Methods
    //------------------------------------------------------------------------
    /**
     * @brief Tarati Service: directory of registered methods
     */
    class MethodDirectory
      : public Method
    {
      public:
        MethodDirectory(Service * service)
          : Method(service, "MethodDirectory") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /**
     * @brief Tarati Service: version number of Service interface
     */
    class Version
      : public Method
    {
      public:
        Version(Service * service)
          : Method(service, "Version") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /// directory of registered methods
    struct _method {
        MethodDirectory * directory;
        Version * version;
    } method;

  public:
    // constructors / destructors
    Service(Server * server, const string & name,
        const string & version);
    ~Service();

    // registration methods
    void MethodRegister (Method * method);
    void MethodUnregister (Method * method);
    void MethodUnregisterAll (void);

    // accessors / modifiers
    Server * GetServer(void) const { return server; }
    const string & GetName(void) const { return name; }
};

} // namespace Tarati

#endif // _TARATI_SERVICE_
