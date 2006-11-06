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

// atalk.cpp
//
// A simple demo xmlrpc client talking to ASIM
//
// Link against xmlrpc lib and whatever socket libs your system needs

#include <iostream>

#include "XmlRpc.h"

#include "taratiClient.h"

using namespace XmlRpc;
using namespace std;

char * def_service = "client";
char * def_method = "directory";
char * def_method2 = "MethodDirectory";

static bool debug = false;
static bool quiet = false;

void
Usage (void)
{
  cerr << "Usage: TclTalk [-s server] [-p port] [-h] [-d] [-q]\n";
  cerr << "Usage: atalk [-s server] [-p port] [-h|d|q] [service [method [args...]]]\n";
  cerr << "       -h ... help (this text)\n";
  cerr << "       -d ... debug on\n";
  cerr << "       -q ... quiet\n";
  cerr << "       service: any service of server (default: "
       << def_service << ")\n";
  cerr << "       method : any method of service (default: "
       << def_method << ")\n";
  cerr << "       service name 'client' enables special methods:\n";
  cerr << "           directory ... recursive directory of server\n";
  exit (1);
}

int main(int argc, char* argv[])
{
  string server = "localhost";
  string port = "11088";
  string service = def_service;
  string method = def_method;
  bool serviceSet = false;
  bool methodSet = false;
  int extraArgs = argc;

  // a kindergarten cmdline parser
  for (int i = 1; i < argc; ) {
    string arg = argv[i++];
    if (arg == "-h") {
      Usage();
    } else if (arg == "-d") {
      debug = true;
      quiet = false;
    } else if (arg == "-q") {
      quiet = true;
      debug = false;
    } else if (arg == "-s") {
      if (i >= argc) {
        cerr << "Error: -s missing hostname" << endl;
        Usage();
      } else {
        server = argv[i++];
      }
    } else if (arg == "-p") {
      if (i >= argc) {
        cerr << "Error: -p missing port number" << endl;
        Usage();
      } else {
        port = argv[i++];
      }
    } else if (arg[0] == '-') {
      cerr << "Error: don't understand flag '" << arg << "'" << endl;
      Usage();
    } else if ( ! serviceSet) {
      service = arg;
      method = def_method2;
      serviceSet = true;
    } else if ( ! methodSet) {
      method = arg;
      methodSet = true;
    } else {
      extraArgs = i - 1;
      break;
    }
  }

  int portnum = strtol(port.c_str(), NULL, 0);

  //XmlRpc::setVerbosity(5);
  cout << "Connecting to " << server << ":" << portnum << " ... " << flush;
  Tarati::Client client(server, portnum, debug);
  cout << "done" << endl;

  XmlRpcValue noArgs, result;

  try {
    if (service == "client") {
      if (method == "directory") {
        if (extraArgs != argc) {
          cerr << "Error: too many arguments on command line" << endl;
          Usage();
        }
        string service = "Server";
        string method = "ServiceDirectory";
        cout << "Service: " << service << "::" << method << "()" << endl;
        XmlRpcValue services;
        if (client.execute(service, method, noArgs, services)) {
          for (int i = 0; i < services.size(); i++) {
            XmlRpcValue version;
            string service = services[i];
            string method = "MethodDirectory";
            client.execute(service, "Version", noArgs, version);
            cout << "    Service: "
                 << service << "(v" << version << ")::"
                 << method << "()" << endl;
            XmlRpcValue methods;
            if (client.execute(service, method, noArgs, methods)) {
              for (int i = 0; i < methods.size(); i++) {
                cout << "        Method: " << methods[i] << endl;
              }
            }
          }
        }
      } else if (method == "stats") {
        if (extraArgs != argc) {
          cerr << "Error: too many arguments on command line" << endl;
          Usage();
        }
        string service = "Stats";
        string method = "States";
        cout << "Service: " << service << "::" << method << "()" << endl;
        XmlRpcValue states;
        if (client.execute(service, method, noArgs, states)) {
          for (int i = 0; i < states.size(); i++) {
            XmlRpcValue args = states[i];
            XmlRpcValue path;
            client.execute(service, "Path", args, path);
            XmlRpcValue name;
            client.execute(service, "Name", args, name);
            XmlRpcValue desc;
            client.execute(service, "Desc", args, desc);
            XmlRpcValue value;
            client.execute(service, "Value", args, value);
            cout << "    State:" << endl;
            cout << "        Path  = " << path << endl;
            cout << "        Name  = " << name << endl;
            cout << "        Desc  = " << desc << endl;
            cout << "        Value = " << value << endl;
          }
        }
      } else {
        cerr << "Don't understand "
             << service << "::" << method << "()" << endl;
      }
    } else {
      cout << "Service: " << service << "::" << method << "()" << endl;
      int arg = 0;
      XmlRpcValue args;
      for (int i = extraArgs; i < argc; i++) {
        // guess if the argument is numeric and convert if possible
        char * endptr;
        int value = strtol(argv[i], &endptr, 0);
        if (*(argv[i]) != '\0' && *endptr == '\0') {
          // treat as numeric
          args[arg++] = value;
        } else {
          // treat as string
          args[arg++] = argv[i];
        }
      }
      if (client.execute(service, method, args, result)) {
        cout << result << endl;
      }
    }
  } catch (const XmlRpcException& fault) {
    cout << "XML-RPC Error: " << fault.getMessage() << endl;
  }
  return 0;
}
