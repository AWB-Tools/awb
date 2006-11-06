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

// TestBase64Server.cpp : Simple XMLRPC server example. Usage: TestBase64Server serverPort
//
#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif


#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>


#include "XmlRpc.h"
using namespace XmlRpc;


// The server
XmlRpcServer s;

// No arguments, result is Base64-encoded pngnow.png data.
class TestBase64 : public XmlRpcServerMethod
{
public:
  TestBase64(XmlRpcServer* s) : XmlRpcServerMethod("TestBase64", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
    std::ifstream infile("pngnow.png", std::ios::binary);
    if (infile.fail())
      infile.open("../pngnow.png", std::ios::binary);
    if (infile.fail())
      result = "Could not open file pngnow.png";
    else {

      XmlRpcValue::BinaryData& data = result;
      int n = 0;
      for (;; ++n) {
        char c = infile.get();
        if (infile.eof()) break;
        data.push_back(c);
      }
      std::cerr << "Read " << n << " bytes from pngnow.png\n";
    }
  }
} TestBase64(&s);    // This constructor registers the method with the server



int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: TestBase64Server serverPort\n";
    return -1;
  }
  int port = atoi(argv[1]);

  //XmlRpc::setVerbosity(5);

  // Create the server socket on the specified port
  s.bindAndListen(port);

  // Wait for requests indefinitely
  s.work(-1.0);

  return 0;
}

