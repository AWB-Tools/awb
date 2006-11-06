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
 * @brief Common Tarati Client code
 */

#ifndef _TARATI_CLIENT_
#define _TARATI_CLIENT_ 1

// generic
#include <unistd.h>
#include <time.h>
#include <iostream>

#include "XmlRpc.h"
#include "XmlRpcSocket.h"

using namespace XmlRpc;
using namespace std;

namespace Tarati {

/**
 * @brief Tarati Client Object
 *
 * This class ...
 */
class Client : public XmlRpcClient
{
  private:
    bool debug;

    class OffErrorHandler : public XmlRpcErrorHandler
    {
      public:
        void error(const char* msg) { /* nada */ };
    };

  public:
    Client (const string & hostname, const int port, bool dbg = false);
    //~Client();

    /// Do a RPC call
    bool execute (const string & service, const string & method,
                  XmlRpcValue & args, XmlRpcValue & result);
    /// Check if connection is still alive
    bool ping (void);
};

} // namespace Tarati

#endif // _TARATI_CLIENT_

