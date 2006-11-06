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

// generic
#include <unistd.h>
#include <time.h>
#include <iostream>

#include "XmlRpc.h"
#include "XmlRpcSocket.h"

#include "taratiClient.h"

using namespace XmlRpc;
using namespace std;

namespace Tarati {

Client::Client (
    const string & hostname,
    const int port,
    bool dbg)
  : XmlRpcClient (hostname.c_str(), port),
    debug(dbg)
{
  // turn xml-rpc error reporting off for a bit
  OffErrorHandler errorOff;
  XmlRpcErrorHandler * origErrorHandler;
  origErrorHandler = XmlRpcErrorHandler::getErrorHandler();
  XmlRpcErrorHandler::setErrorHandler( & errorOff);
  // wait until the server answers to a benign request
  XmlRpcValue args;
  XmlRpcValue result;
  bool ok;
  const long millisec = 1000000;
  timespec time;
  time.tv_sec = 0;
  time.tv_nsec = 100 * millisec;
  do {
    ok = XmlRpcClient::execute ("", args, result);
    if ( ! ok) {
      nanosleep(&time, NULL); // take a nap
    }
  } while ( ! ok);
  // turn xml-rpc error reporting back on
  XmlRpcErrorHandler::setErrorHandler(origErrorHandler);
}

bool
Client::execute (
    const string & service,
    const string & method,
    XmlRpcValue & args,
    XmlRpcValue & result)
{
  if (debug) {
    cout << "Service: " << service << "::" << method << "(" 
         << args << ")" << endl;
  }
  string call = service + "::" + method;
  bool ok = XmlRpcClient::execute (call.c_str(), args, result);
  if ( ! ok) {
    cerr << "Error calling " << service << "::" << method << "():"
         << endl << "  internal XML-RPC error" << endl;
  } else {
    if (isFault()) {
      ok = false;
      cerr << "Error calling " << service << "::" << method << "():"
           << endl << "  " << result << endl;
    }
  }
  return ok;
}

/**
 * Test if the server connection is still alive.
 * @returns true on success, false otherwise
 */
bool
Client::ping (void)
{
  if (debug) {
    cout << "Pinging server ... " << flush;
  }

  // turn xml-rpc error reporting off for a bit
  OffErrorHandler errorOff;
  XmlRpcErrorHandler * origErrorHandler;
  origErrorHandler = XmlRpcErrorHandler::getErrorHandler();
//  XmlRpcErrorHandler::setErrorHandler( & errorOff);
  // send a benign request to server
  XmlRpcValue args;
  XmlRpcValue result;
  bool ok;
  ok = XmlRpcClient::execute ("", args, result);
  // turn xml-rpc error reporting back on
//  XmlRpcErrorHandler::setErrorHandler(origErrorHandler);

  if (debug) {
    if (ok) {
      cout << "responded ok" << endl;
    } else {
      cout << "response failure" << endl;
    }
  }

  return ok;
}

} // namespace Tarati
