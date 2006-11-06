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
 * @brief ASIM Tarati Service: TCL Beamer
 * @note: this is highly intertwined with how the controller works and
 * will easily break with changes to the controller;
 */

#ifndef _TARATI_TCL_BEAMER_
#define _TARATI_TCL_BEAMER_

// generic
#include <signal.h>
#include <map>

// ASIM public modules
#include "asim/provides/tarati.h"

using namespace Tarati;

namespace AsimTarati {

// tclBeamer is globally visible
class TclBeamer;
extern TclBeamer * tclBeamer;

// command function type
typedef int (* CommandFunction)(int, const char**, string &);

//----------------------------------------------------------------------------
// ASIM Tarati Service: TCL Beamer
//----------------------------------------------------------------------------
/**
 * @brief Tarati Service: TCL Beamer - move TCL interactions across Tarati
 * client-server interface.
 */
class TclBeamer
  : public Service
{
  private:
    static char * const name    = "TclBeamer";
    static char * const version = "0.1";

    // types
    typedef map<string, CommandFunction> CommandMap;

    //------------------------------------------------------------------------
    // Methods
    //------------------------------------------------------------------------
    /**
     * @brief Tarati Method: Args()
     * Provides a number of parsed command line arguments to the TclBeamer
     * client, so the client can start up with the right environment.
     */
    class Args
      : public Method
    {
      public:
        Args(TclBeamer * _service)
          : Method(_service, "Args") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);

      private:
        /// Read a file and put its data in result
        void ReadFile(const string & filename, XmlRpcValue & result);
    };

    /**
     * @brief Tarati Method: NotifyChannelOpen()
     * Opens a notification channel from server to client. Sends back
     * the port number to client (so it can connect).
     */
    class NotifyChannelOpen
      : public Method
    {
      public:
        NotifyChannelOpen(TclBeamer * _service)
          : Method(_service, "NotifyChannelOpen")
        {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /**
     * @brief Tarati Method: NotifyChannelClose()
     * Closes an open notification channel.
     */
    class NotifyChannelClose
      : public Method
    {
      public:
        NotifyChannelClose(TclBeamer * _service)
          : Method(_service, "NotifyChannelClose")
        {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /**
     * @brief Tarati Method: Command(name, args)
     * Execute a (Tcl) command that is registered with this service.
     */
    class Command
      : public Method
    {
      private:
        TclBeamer * tclBeamer; ///< Service pointer to access command map

      public:
        Command(TclBeamer * _service)
          : Method(_service, "Command"),
            tclBeamer(_service)
        {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };
    friend class Command; // to access commandMap

    /**
     * @brief Tarati Method: CommandDirectory()
     * List all registered commands.
     */
    class CommandDirectory
      : public Method
    {
      private:
        TclBeamer * tclBeamer; ///< Service pointer to access command map

      public:
        CommandDirectory(TclBeamer * _service)
          : Method(_service, "CommandDirectory"),
            tclBeamer(_service)
        {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };
    friend class CommandDirectory; // to access commandMap

    //------------------------------------------------------------------------
    // Service
    //------------------------------------------------------------------------

    struct _method {
        Args * args;
        NotifyChannelOpen * notifyChannelOpen;
        NotifyChannelClose * notifyChannelClose;
        Command * command;
        CommandDirectory * commandDirectory;
    } method;

    static struct _notify {
      int listen;         ///< port we are listening on
      int connected;      ///< connected port
    } notifyChannel;      ///< notifiaction channel ports

    CommandMap commands;  ///< commands that are registered

    // consts
    /// notification signal to send
    static const int signalNotify = SIGUSR1;
    /// handup (HUP) signal to send
    static const int signalHUP = SIGPIPE;
    /// xml-rpc attention signal to send
    static const int signalServerAttention = SIGIO;

  public:
    TclBeamer(Server * server);
    ~TclBeamer();

    void RegisterCommand (const string & name, CommandFunction command);
    static bool NotifyAccept(void);
    static int NotifyClient(void);
    static void NotifyAttention(int signo, siginfo_t * info, void * context);
};

} // namespace AsimTarati

#endif /* _TARATI_TCL_BEAMER_ */
