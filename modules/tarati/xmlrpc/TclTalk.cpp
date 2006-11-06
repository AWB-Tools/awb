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
 * @brief TCL client for ASIM's TclBeamer Tarati Service
 * @note: TCL part based on David Goodwin's original TCL interface code.
 */

// generic
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <iostream>
#include <sstream>

#include "XmlRpc.h"
#include "XmlRpcSocket.h"

#include "taratiClient.h"

using namespace XmlRpc;
using namespace std;

// USE_NON_CONST is to remove some incompatibility between tcl 8.3 and
// tcl 8.4 with respect to use of char ** vs. const char ** for
// command arguments;
#define USE_NON_CONST
extern "C" {
#include <tcl.h>
}
#undef USE_NON_CONST

// simple replacement definition of the real thing
#define ASIMERROR(a) cerr << "TclTalk::Error: " << a << endl; exit(1);

static bool debug = false;
static bool quiet = false;
static bool exiting = false;

// args from the other side over there (TclBeamer Server side)
struct RemoteFile {
  string filename;
  string data;
};

struct RemoteArgs {
  string awbWorkbench;
  RemoteFile overrideWorkbench;
  RemoteFile awbCmdFile;
};

//----------------------------------------------------------------------------
// below is code lifted from awb.cpp
//----------------------------------------------------------------------------

// The tcl interpreter running the awb workbench
Tcl_Interp *awbInterp = NULL;

/**
 * Notify the workbench of the progress events that are pending,
 * and wait for it to acknowledge that it has processed all the
 * events.
 */
void
AWB_InformProgress (string & pendingProgress)
{
    if ( ! pendingProgress.empty()) {
        if (debug) {
          cout << "Progress: " << pendingProgress << endl;
        }
        ostringstream os;
        os << "BatchProgress { " << pendingProgress << " }";
    
        int eval = Tcl_Eval(awbInterp, const_cast<char*>(os.str().c_str()));

        if ( eval != TCL_OK) {
            ASIMERROR("AWB_InformProgress: "
                << Tcl_GetStringResult(awbInterp) << endl); 
        }
    }
}

/**
 * Exit awb...
 */
void
AWB_Exit (void)
{
    Tcl_DeleteInterp(awbInterp);
    awbInterp = NULL;
    Tcl_Finalize();
}

/**
 * Interface to TclBeamer Tarati Service, ie. remote commands.
 */
int
TclBeamer (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    Tarati::Client & client = * (Tarati::Client*)clientData;

    // marshal arguments
    XmlRpcValue args, result;
    for (int i = 0; i < argc; i++) {
      args[i] = argv[i];
    }

    // Beam me up, Scotty
    if ( ! client.execute("TclBeamer", "Command", args, result)) {
      return TCL_ERROR;
    }

    // unmarshal result
    char * res = const_cast<char*>(string(result["result"]).c_str());
    Tcl_SetResult(interp, res, TCL_VOLATILE);

    return int(result["status"]);
}

/**
 * Local TCL command telling us to exit
 */
int
TclExit (ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    Tarati::Client & client = * (Tarati::Client*)clientData;

    exiting = true;

    if (debug) {
      cout << "TclExit executing" << endl;
    }

    XmlRpcValue noArgs;
    XmlRpcValue result;
    if ( ! client.execute("TclBeamer", "NotifyChannelClose", noArgs, result)) {
      ASIMERROR("notification channel shutdown failure");
    }

    return TCL_OK;
}

/**
 * Start TCL interpreter, Tarati Client, and register all functions.
 */
bool
AwbStartInterpreter (
    Tarati::Client & client,
    RemoteArgs & there,
    XmlRpcValue & commands)
{
    // Create the tcl interpreter
    awbInterp = Tcl_CreateInterp();

    // Initialize tcl
    Tcl_Preserve(awbInterp);

    if (Tcl_Init(awbInterp) != TCL_OK)
    {
        ASIMERROR("AwbStartInterpreter: Tcl_Init "
            << Tcl_GetStringResult(awbInterp) << endl);
    }

    // locally register remote commands
    for (int i = 0; i < commands.size(); i++) {
      Tcl_CreateCommand(awbInterp,
                        const_cast<char*>(string(commands[i]).c_str()),
                        TclBeamer, &client, NULL);
    }
    // register special local command with which TCL tells us to exit
    Tcl_CreateCommand(awbInterp, "TclExit", TclExit, &client, NULL);
    
    // Load script for the default workbench configured for this
    // model. After loading the script, we can read "awbNamespace" to get
    // the workbench's namespace and then import the init procedure.
    if (there.overrideWorkbench.filename.empty()) {
        if (Tcl_GlobalEval(awbInterp,
            const_cast<char*>(there.awbWorkbench.c_str())) != TCL_OK)
        {
            ASIMERROR("AwbStartInterpreter: Tcl_GlobalEval for awbWorkbench: "
                << Tcl_GetStringResult(awbInterp) << endl
                << "Command:" << endl << there.awbWorkbench.c_str()); 
        }
    } else {
        if (Tcl_GlobalEval(awbInterp,
            const_cast<char*>(there.overrideWorkbench.data.c_str())) != TCL_OK)
        {
            ASIMERROR("AwbStartInterpreter: Tcl_GlobalEval "
                << Tcl_GetStringResult(awbInterp)); 
        }
    }

    char *ns = Tcl_GetVar(awbInterp, "AwbNamespace",
                          TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);
    if (ns == NULL) {
        ASIMERROR("AwbStartInterpreter: Tcl_GetVar "
            << Tcl_GetStringResult(awbInterp)); 
    }

    // localize temps
    {
        ostringstream os;
        char * buf;

        os << "namespace import "
           << ns << "::AwbInit "
           << ns << "::BatchProgress";
        buf = strdup(os.str().c_str());
        if (Tcl_GlobalEval(awbInterp, buf) != TCL_OK) {
            ASIMERROR("AwbStartInterpreter: Tcl_GlobalEval for namespace import: "
                << Tcl_GetStringResult(awbInterp)); 
        }
        free(buf);

        // Initialize the workbench telling it to use the root window "."
        // ahh yeah, some ancient TK remnants are still standing ...
        os.str(""); // clear
        os << "AwbInit . " << "batch"
           << " {" << there.awbCmdFile.filename << "}"
           << " {" << there.awbCmdFile.data << "}";
        buf = strdup(os.str().c_str());
        if (Tcl_GlobalEval(awbInterp, buf) != TCL_OK) {
            ASIMERROR("AwbStartInterpreter: Tcl_GlobalEval for AwbInit: "
                << Tcl_GetStringResult(awbInterp)); 
        }
        free(buf);
    }

    Tcl_Release(awbInterp);
    
    return true;
}


//----------------------------------------------------------------------------
// main stuff
//----------------------------------------------------------------------------

void
Usage (void)
{
  cerr << "Usage: TclTalk [-s server] [-p port] [-h] [-d] [-q]\n";
  cerr << "       -h ... help (this text)\n";
  cerr << "       -d ... debug on\n";
  cerr << "       -q ... quiet\n";
  exit (1);
}

int main(int argc, char* argv[])
{
  RemoteArgs there;
  string server = "localhost";
  string port = "11088";

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
    } else {
      cerr << "Error: excess command line argument '" << arg << "'" << endl;
      Usage();
    }
  }

  int portnum = strtol(port.c_str(), NULL, 0);

  //XmlRpc::setVerbosity(5);
  if ( ! quiet) {
    cout << "Connecting to " << server << ":" << portnum << " ... " << flush;
  }
  Tarati::Client client(server, portnum, debug);
  if ( ! quiet) {
    cout << "done" << endl;
  }

  XmlRpcValue noArgs, result;

  try {
    //
    // lets do some sanity checks on the server
    //
    // server version check
    if (client.execute("Server", "Version", noArgs, result)) {
      // any verion is OK for now
      if ( ! quiet) {
        cout << "Tarati Server (Version " << result << " OK)" << endl;
      }
    } else {
      exit(1);
    }
    // we need a TclBeamer service
    XmlRpcValue services;
    if (client.execute("Server", "ServiceDirectory", noArgs, services)) {
      bool found = false;
      for (int i = 0; i < services.size(); i++) {
        if (services[i] == "TclBeamer") {
          if (client.execute("TclBeamer", "Version", noArgs, result)) {
            // any verion is OK for now
            if ( ! quiet) {
              cout << "Tarati Service: TclBeamer (Version " << result << " OK)"
                   << endl;
            }
            found = true;
            break;
          } else {
            exit(1);
          }
        }
      }
      if ( ! found) {
        ASIMERROR("missing Tarati Service TclBeamer");
      }
    } else {
      exit(1);
    }

    //
    // get the args from there, so we can start up TCL here
    //
    if ( ! quiet) {
      cout << "downloading remote execution environment ... " << flush;
    }
    XmlRpcValue args;
    if (client.execute("TclBeamer", "Args", noArgs, args)) {
      there.awbWorkbench = args["awbWorkbench"];
      if (args.hasMember("overrideWorkbench")) {
        there.overrideWorkbench.filename =
          args["overrideWorkbench"]["filename"];
        there.overrideWorkbench.data =
          args["overrideWorkbench"]["data"];
      }
      there.awbCmdFile.filename = args["awbCmdFile"]["filename"];
      there.awbCmdFile.data = args["awbCmdFile"]["data"];
    } else {
      exit(1);
    }

    //
    // get the remote command directory for local command registration
    //
    XmlRpcValue commands;
    if ( ! client.execute("TclBeamer", "CommandDirectory", noArgs, commands)) {
      exit(1);
    }
    if ( ! quiet) {
      cout << "done" << endl << flush;
    }

    //
    // ready for some TCL action
    //
    if ( ! quiet) {
      cout << "starting local TCL interpreter ... " << flush;
    }
    if ( ! AwbStartInterpreter(client, there, commands)) {
      ASIMERROR("TCL interpreter startup failed");
    }
    if ( ! quiet) {
      cout << "done" << endl << flush;
    }

    //
    // tell server to open a notification channel
    //
    if ( ! quiet) {
      cout << "opening notification channel to server ... " << flush;
    }
    XmlRpcValue notify;
    if ( ! client.execute("TclBeamer", "NotifyChannelOpen", noArgs, notify)) {
      exit(1);
    }

    int notifyChannel = XmlRpcSocket::socket();
    if (notifyChannel == -1) {
        ASIMERROR(string("socket ") + strerror(errno));
    }
    if (! XmlRpcSocket::connect(notifyChannel, server, notify)) {
        ASIMERROR(string("connect ") + strerror(errno));
    }

    if ( ! quiet) {
      cout << "done" << endl << flush;
    }

    //
    // main loop
    //
    if ( ! quiet) {
      cout << "script execution commencing" << endl;
    }
    XmlRpcValue getProgressCmd;
    XmlRpcValue pendingProgress;
    getProgressCmd[0] = "PmControl";
    getProgressCmd[1] = "getprogress";
    XmlRpcValue doneProgressCmd;
    doneProgressCmd[0] = "PmControl";
    doneProgressCmd[1] = "doneprogress";

    while ( ! exiting) {
      // yield CPU until we receive a progress notification
      if ( ! exiting) {
        const int recvsize = 1;
        char dummy[recvsize];
        int recved;
        do {
          //
          // we wait in this loop for a server notification, but also check
          // for the server disappearing due to a crash;
          //
          const int timeout = 600; // just in case a server crashes badly
          fd_set inFd, outFd, excFd;
	  FD_ZERO(&inFd);
	  FD_ZERO(&outFd);
	  FD_ZERO(&excFd);
          FD_SET(notifyChannel, &inFd);
          int nEvents;
          int maxFd = notifyChannel;
          if (timeout >= 0) {
            struct timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            nEvents = select(maxFd+1, &inFd, &outFd, &excFd, &tv);
          } else {
            nEvents = select(maxFd+1, &inFd, &outFd, &excFd, NULL);
          }
          if (nEvents == -1 && errno == EINTR) {
            // ignore interrupted syscall
            continue;
          }
          if (nEvents == 0) {
            // timeout expired - check connection
            if (client.ping()) {
              // ping OK - continue waiting
              recved = 0;
            } else {
              // ping failed - sneak in an error indication
              recved = -1;
              errno = EPIPE;
            }
          } else {
            recved = recv(notifyChannel, &dummy, recvsize, 0);
            if (recved == 0) {
              // if select says there is something to read, but recv says
              // there are 0 bytes read, we're at end-of-file; make up an
              // error indication;
              recved = -1;
              errno = EPIPE;
            }
          }
        } while (recved == 0);
        if (recved < 0) {
          if (errno == EPIPE) {
            ASIMERROR("Lost connection to performance model");
          } else {
            ASIMERROR("notification channel: " << strerror(errno));
          }
        }
      }

      // process the progress message
      if (! client.execute("TclBeamer", "Command",
                           getProgressCmd, pendingProgress))
      {
        ASIMERROR("PmControl getprogress failed");
      }
      string message = string(pendingProgress["result"]);
      if (message.size() > 0) {
        // push progress down to TCL
        AWB_InformProgress (message);
        // we need to inform the other side that we're done with this
        if (! client.execute("TclBeamer", "Command", doneProgressCmd, result))
        {
          ASIMERROR("PmControl doneprogress failed");
        }
      }
    }

    //
    // shutdown
    //
    AWB_Exit();
  } catch (const XmlRpcException& fault) {
    cerr << "XML-RPC Error: " << fault.getMessage() << endl;
  }
  return 0;
}
