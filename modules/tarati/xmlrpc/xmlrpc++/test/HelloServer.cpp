/*
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

// HelloServer.cpp : Simple XMLRPC server example. Usage: HelloServer serverPort
//
#include "XmlRpc.h"

#include <iostream>
#include <stdlib.h>

using namespace XmlRpc;

// The server
XmlRpcServer s;

// No arguments, result is "Hello".
class Hello : public XmlRpcServerMethod
{
public:
  Hello(XmlRpcServer* s) : XmlRpcServerMethod("Hello", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
    result = "Hello";
  }

  std::string help() { return std::string("Say hello"); }

} hello(&s);    // This constructor registers the method with the server


// One argument is passed, result is "Hello, " + arg.
class HelloName : public XmlRpcServerMethod
{
public:
  HelloName(XmlRpcServer* s) : XmlRpcServerMethod("HelloName", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
    std::string resultString = "Hello, ";
    resultString += std::string(params[0]);
    result = resultString;
  }
} helloName(&s);


// A variable number of arguments are passed, all doubles, result is their sum.
class Sum : public XmlRpcServerMethod
{
public:
  Sum(XmlRpcServer* s) : XmlRpcServerMethod("Sum", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
    int nArgs = params.size();
    double sum = 0.0;
    for (int i=0; i<nArgs; ++i)
      sum += double(params[i]);
    result = sum;
  }
} sum(&s);


int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: HelloServer serverPort\n";
    return -1;
  }
  int port = atoi(argv[1]);

  XmlRpc::setVerbosity(5);

  // Create the server socket on the specified port
  s.bindAndListen(port);

  // Enable introspection
  s.enableIntrospection(true);

  // Wait for requests indefinitely
  s.work(-1.0);

  return 0;
}

