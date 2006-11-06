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
 * @brief Implementation of Tarati Method (externally callable)
 */

#ifndef _TARATI_SERVICE_METHOD_
#define _TARATI_SERVICE_METHOD_ 1

// generic
#include <string>
#include <iostream>

// XML-RPC low level transport library
#include <asim/provides/xmlrpc.h>

using namespace XmlRpc;

namespace Tarati {

// forward declarations to avoid circular dependencies
class Server;
class Service;

/**
 * @brief Tarati Method Object
 *
 * This class ...
 */
class Method
  : public XmlRpcServerMethod
{
  private:

  protected:
    Service * service;
    string name;

  public:
    // contructors / destructors
    Method(Service * service, const string & name);
    //~Method();

    // accessors / modifiers
    /// Get associated service
    const Service * GetService (void) const { return service; }
    /// Set associated service
    //void SetService (Service * service) { this->service = service; }
    /// Get method name
    const string & GetName (void) const { return name; }
    /// Set method name
    //void SetName (const string & name) { this->name = name; }
    //
    /// Get name from base class
    const string & GetXmlName (void) const { return _name; }
    /// Change name in base class
    void SetXmlName (const string & name) { _name = name; }
    //
    /// Stub for the execute virtual function
    virtual void execute(XmlRpc::XmlRpcValue &, XmlRpc::XmlRpcValue &)
    {
      XmlRpcException error(
        "Tarati::Method::execute() not implemented for method " + name);
      throw(error);
    }
};

} // namespace Tarati

#endif // _TARATI_SERVICE_METHOD_
