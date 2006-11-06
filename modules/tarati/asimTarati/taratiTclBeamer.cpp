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
 * @brief ASIM Tarati Service: TCL Beamer - remote TCL service
 * @note: this is highly intertwined with how the controller works and
 * will easily break with changes to the controller;
 */

// generic
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <fstream>

// ASIM core

// ASIM other modules
#include "asim/provides/workbench.h"
#include "asim/provides/awb_stub.h"

// ASIM local module
#include "taratiTclBeamer.h"

namespace AsimTarati {

// this is awfully ugly, but we need to make the TclBeamer globally
// visible such that awb_taratiTclBeamer can find it; but we at least
// constrain it to this namespace;
TclBeamer * tclBeamer = NULL;

// define static (per class) storage
TclBeamer::_notify TclBeamer::notifyChannel = {-1, -1};

//----------------------------------------------------------------------------
// Tarati Services
//----------------------------------------------------------------------------
/**
 * Initialize all components of TclBeamer Tarati Service and register Tarati
 * Methods provided by this service.
 */
TclBeamer::TclBeamer(
    Server * server)
  : Service(server, name, version)
{
    // make globally visible
    tclBeamer = this;

    // instantiate and register methods
    method.args = new Args(this);
    method.notifyChannelOpen = new NotifyChannelOpen(this);
    method.notifyChannelClose = new NotifyChannelClose(this);
    method.command = new Command(this);
    method.commandDirectory = new CommandDirectory(this);
}

/**
 * Unregister all Tarati Methods associated with this service and clean up
 * locally allocated memory.
 */
TclBeamer::~TclBeamer()
{
    // make globally invisible again
    tclBeamer = NULL;

    // delete methods
    delete method.args;
    delete method.notifyChannelOpen;
    delete method.notifyChannelClose;
    delete method.command;
    delete method.commandDirectory;
}

/**
 * Register a command function with this TclBeamer.
 */
void
TclBeamer::RegisterCommand (
    const string & name,
    CommandFunction command)
{
    commands[name] = command;
}

/**
 * Wait until a client is connected on the notification channel
 */
bool
TclBeamer::NotifyAccept (void)
{
    if (notifyChannel.listen < 0) {
        return false;
//        ASIMERROR("TclBeamer::NotifyAccept: notify channel not created");
    }
    if (notifyChannel.connected >= 0) {
        ASIMERROR("TclBeamer::NotifyAccept: notify channel already connected");
    }

    notifyChannel.connected = XmlRpcSocket::accept(notifyChannel.listen);
    if (notifyChannel.connected < 0) {
        if (errno == EAGAIN) {
            // no connection available yet
            return false;
        }
        ASIMERROR("TclBeamer::NotifiactionAccept: accept " << endl
            << "errno " << errno << " : " << strerror(errno));
    }

    //
    // install signal handler for monitoring activity on connected
    // notifyChannel
    //
    struct sigaction action;
    action.sa_sigaction = NotifyAttention;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, signalNotify);
    action.sa_flags = SA_RESTART | SA_SIGINFO;
    // signalNotify is delivered for async I/O notification
    if (sigaction(signalNotify, &action, NULL) != 0) {
        ASIMERROR("TclBeamer::NotifiactionAccept: " 
                  << "can't install signal handler for signal "
                  << strsignal(signalNotify) << " " << strerror(errno));
    }

    // turn on async I/O signals for this socket
    if (! XmlRpcSocket::setAsyncIo(notifyChannel.connected, signalNotify)) {
        ASIMERROR("TclBeamer::NotifiactionAccept: setAsyncIo "
                  << strerror(errno));
    }

//    if (! XmlRpcSocket::setNonBlocking(notifyChannel.connected)) {
//        ASIMERROR("TclBeamer::NotifiactionAccept: setNonBlocking "
//                  << strerror(errno));
//    }
    return true;
}

/**
 * Notify a client that we are waiting for it
 */
int
TclBeamer::NotifyClient (void)
{
    if (notifyChannel.connected < 0) {
        ASIMERROR("TclBeamer::NotifyClient: notify channel not connected");
    }

    const int sendsize = 1;
    char dummy[sendsize] = "";
    int result = send (notifyChannel.connected, &dummy, sendsize, MSG_NOSIGNAL);
    return result;
}

/**
 * Signal handler for activity on notify channel
 */
void
TclBeamer::NotifyAttention(int signo, siginfo_t * info, void * context)
{
    if (false /*debug*/) {
        cerr << "NotifyAttention for signal " << info->si_signo << " "
             << "'" << strsignal(info->si_signo) << "' "
             << "errno " << info->si_errno << " "
             << "code " << info->si_code << " "
             << "'" << (
               info->si_code == SI_USER  ? "SI_USER" :
               info->si_code == POLL_IN  ? "poll-in" :
               info->si_code == POLL_OUT ? "poll-out" :
               info->si_code == POLL_MSG ? "poll-msg" :
               info->si_code == POLL_ERR ? "poll-err" :
               info->si_code == POLL_PRI ? "poll-pri" :
               info->si_code == POLL_HUP ? "poll-hup" :
               "unknown")
             << "' "
             << "on fd " << info->si_fd << endl << flush;
    }

    // Note: the only reasons why we should see signals coming from this
    // socket are connection/disconnection attempts;
    //
    // figure out if a HUP happened on the channel
    if (notifyChannel.connected >= 0) {
        // we have to try send something to see if the socket was HUPed
        int result = NotifyClient();
        if (result < 0 && (errno == EPIPE || errno == ECONNRESET)) {
            cerr << "Error: TCL controller connection broken" << endl;
            // fabricate a signalHUP to indicate that connection is broken
            kill (getpid(), signalHUP);
        }
    }

    // fabricate a signalServerAttention to inform Tarati server of activity
    kill (getpid(), signalServerAttention);
}

//----------------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------------
//----- Args -----
void
TclBeamer::Args::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    // get params
    if (params.getType() != XmlRpcValue::TypeInvalid) {
        throw XmlRpcException("wrong number of arguments in method call");
    }

    if (overrideWorkbench) {
        ReadFile (overrideWorkbench, result["overrideWorkbench"]);
    }
    char *awbWorkbench = AWB_WbInit();
    result["awbWorkbench"] = awbWorkbench; // note: this is BIG
    delete[] awbWorkbench;
    ReadFile (awbCmdFile, result["awbCmdFile"]);
}

void
TclBeamer::Args::ReadFile(
    const string & filename,
    XmlRpcValue & result)
{
    string line;
    string data;

    if (filename != "") {
        // open input file
        ifstream in(filename.c_str());
        if ( ! in) {
            ASIMERROR ("Error: can't open input file " << filename);
        }

        // read
        while ( ! in.eof()) {
            getline (in, line);
            if (line.empty() && in.eof()) {
                break; // also eof
            }
            data += line + "\n";
        }

        // cleanup
        in.close();
    }

    result["filename"] = filename;
    result["data"] = data;
}

//----- OpenNotifyChannel -----
void
TclBeamer::NotifyChannelOpen::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    // get params
    if (params.getType() != XmlRpcValue::TypeInvalid) {
        throw XmlRpcException("wrong number of arguments in method call");
    }

    if (notifyChannel.listen >= 0) {
        throw XmlRpcException("notification channel already created");
    }

    notifyChannel.listen = XmlRpcSocket::socket();
    if (notifyChannel.listen == -1) {
        throw XmlRpcException(string("socket ") + strerror(errno));
    }
    if (! XmlRpcSocket::setNonBlocking(notifyChannel.listen)) {
        throw XmlRpcException(string("setNonBlocking ") + strerror(errno));
    }
    if (! XmlRpcSocket::setReuseAddr(notifyChannel.listen)) {
        throw XmlRpcException(string("setReuseAddr ") + strerror(errno));
    }
    if (! XmlRpcSocket::setAsyncIo(notifyChannel.listen,
                                   signalServerAttention))
    {
        throw XmlRpcException(string("setAsyncIo ") + strerror(errno));
    }
    if (! XmlRpcSocket::bind(notifyChannel.listen, 0)) {
        throw XmlRpcException(string("bind ") + strerror(errno));
    }
    if (! XmlRpcSocket::listen(notifyChannel.listen, 1)) {
        throw XmlRpcException(string("listen ") + strerror(errno));
    }
    int port = XmlRpcSocket::getPort(notifyChannel.listen);
    if (port < 0) {
        throw XmlRpcException(string("getPort ") + strerror(errno));
    }

    result = port;
}

//----- CloseNotifyChannel -----
void
TclBeamer::NotifyChannelClose::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    // get params
    if (params.getType() != XmlRpcValue::TypeInvalid) {
        throw XmlRpcException("wrong number of arguments in method call");
    }

    if (notifyChannel.listen < 0) {
        throw XmlRpcException("notification channel not created");
    }

    // close both sockets of notification channel
    if (notifyChannel.connected >= 0) {
        close (notifyChannel.connected);
        notifyChannel.connected = -1;
    }
    if (notifyChannel.listen >= 0) {
        close (notifyChannel.listen);
        notifyChannel.listen = -1;
    }
}

//----- Command -----
void
TclBeamer::Command::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    CommandFunction command = NULL;

    // get params
    if (params.getType() != XmlRpcValue::TypeArray ||
        params.size() < 1)
    {
        throw XmlRpcException("wrong number of arguments in method call");
    }
    const string & name = params[0];

    // find command
    CommandMap::iterator it = tclBeamer->commands.find(name);
    if (it != tclBeamer->commands.end()) {
        command = it->second;
    } else {
        ostringstream os;
        os << "TclBeamer: command '" << name << "' not found";
        throw XmlRpcException(os.str());
    }

    if (command == NULL) {
        throw XmlRpcException("TclBeamer: internal command error");
    }

    // invariant: we've got a valid command

    // prepare arguments for command call
    int argc = params.size();
    const char * argv[argc + 1];
    // command name is argv[0], args are argv[1..argc-1], argv[argc] = NULL
    for (int i = 0; i < argc; i++) {
        argv[i] = string(params[i]).c_str();
    }
    argv[argc] = NULL;
    // now call the command
    string res;
    int status = (*command)(argc, argv, res);
    // marshal return values
    result["status"] = status;
    result["result"] = res;
}

//----- CommandDirectory -----
void
TclBeamer::CommandDirectory::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    // get params
    if (params.getType() != XmlRpcValue::TypeInvalid) {
        throw XmlRpcException("wrong number of arguments in method call");
    }

    int count = 0;
    for (CommandMap::const_iterator it = tclBeamer->commands.begin();
         it != tclBeamer->commands.end();
         ++it)
    {
        result[count++] = it->first;
    }
}

} // namespace AsimTarati
