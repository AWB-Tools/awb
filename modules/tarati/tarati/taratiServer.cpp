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
 * @brief Implementation of Tarati Server (ASIM external connection)
 */

// generic
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
  // semaphore stuff
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

// ASIM local module
#include "taratiUtil.h"
#include "taratiServer.h"
#include "taratiService.h"
#include "taratiMethod.h"
#include "taratiServer_Builtin.h"

// ASIM public modules
#include "asim/provides/tarati.h"

namespace Tarati {

// note: this does not scale to more than 1 server
static int ServerAttentionSema = -1;
/**
 * maintains the property that it is false IFF there is guaranteed that
 * there is no work available; might have false positives though;
 */
static bool ServerNeedsAttention = false;

// signal handler for getting server attention
void IoAttention(int signo, siginfo_t * info, void * context)
{
    if (false /*debug*/) {
        cerr << "IoAttention for signal " << info->si_signo << " "
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

    // increment semaphore
    struct sembuf operation;
    operation.sem_num = 0;
    operation.sem_op = 1; // increment by 1
    operation.sem_flg = 0;
    semop (ServerAttentionSema, &operation, 1);
    // note: we refrain from checking for errors, since we can't do much
    // error handling in the signal handler anyway;
    ServerNeedsAttention = true;
}

//----------------------------------------------------------------------------
// Server proper
//----------------------------------------------------------------------------
/**
 * Create a new Tarati server
 */
Server::Server()
{
    //
    // set a custom error handler for underlying XML-RPC errors
    //
    XmlRpcErrorHandler::setErrorHandler( & taratiXmlRpcErrorHandler);

    //
    // create a semaphore to indicate available work
    //
    const int permissions = 0700; // u=rw,g-a,o-a
    ServerAttentionSema = semget(IPC_PRIVATE, 1, permissions);
    if (ServerAttentionSema < 0) {
        TARATI_ERROR("can't get semaphore for server");
    }
    ServerNeedsAttention = false;

    //
    // install signal handlers for monitoring activity on file descriptors
    //
    struct sigaction action;
    action.sa_sigaction = IoAttention;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGIO);
    sigaddset(&action.sa_mask, SIGALRM);
    action.sa_flags = SA_RESTART | SA_SIGINFO;
    // SIGIO is delivered for async I/O notification
    if (sigaction(SIGIO, &action, NULL) != 0) {
        TARATI_ERROR("can't install signal handler for SIGIO");
    }
    // SIGALRM for itimer based fallback mechanism
    if (sigaction(SIGALRM, &action, NULL) != 0) {
        TARATI_ERROR("can't install signal handler for SIGIO");
    }
    // start an inteval timer, in case other notifications fail
    const int interval = 10;
    struct itimerval timeout;
    timeout.it_value.tv_sec = interval;
    timeout.it_value.tv_usec = 0;
    timeout.it_interval.tv_sec = interval;
    timeout.it_interval.tv_usec = 0;
//    setitimer(ITIMER_REAL, &timeout, NULL);

    //
    // create the XML-RPC server
    //
    xmlrpcServer = new XmlRpc::XmlRpcServer;
    if ( ! xmlrpcServer) {
        TARATI_ERROR("could not create XmlRcpServer object");
    }
    bool ok = xmlrpcServer->bindAndListen(TARATI_SERVER_PORT);
    if ( ! ok) {
        TARATI_ERROR("could not bind socket to Tarati Server port "
                     << TARATI_SERVER_PORT);
    }

    // Server's built-in Service
    service.server = new ServerBuiltin::Server(this);

    // turn on generation of async I/O events
    xmlrpcServer->setAsyncIo(SIGIO);
};

/**
 * Delete a Tarati server
 */
Server::~Server()
{
    // Server's built-in Service
    delete service.server;

    // XML-RPC
    delete xmlrpcServer;
};

/**
 * Register a new service
 */
void
Server::ServiceRegister (
    Service * service) ///< new service to register
{ 
  services[service->GetName()] = service;
}

/**
 * Unregister an existing service
 */
void
Server::ServiceUnregister (
    Service * service) ///< existing service to unregister
{
    // remove service
    ServiceMap::iterator it = services.find(service->GetName());
    if (it != services.end()) {
        services.erase(it);
    } else {
      // is this a bug?
    }
}

/**
 * Register a new method
 */
void
Server::MethodRegister (
    Method * method) ///< new method to register
{ 
    method->SetXmlName (GenerateMethodName (method));
    xmlrpcServer->addMethod (method);
}

/**
 * Unregister an existing method
 */
void
Server::MethodUnregister (
    Method * method) ///< existing method to unregister
{
    xmlrpcServer->removeMethod (method);
}

/**
 * Generate the name by which XML-RPC can call this method
 */
const string
Server::GenerateMethodName (
    Method * method)
const
{
    return (method->GetService()->GetName() + "::" + method->GetName());
}

/**
 * Check if there is something to do and do it
 */
void
Server::Work (
    double timeout) ///< number of seconds to keep waiting/working
{
    if (ServerNeedsAttention) {
        ServerNeedsAttention = false;

        // reset the semaphore
        // note: the server will pick do ALL work that is available, so it is
        //       OK to reset the sema here;
        if (semctl (ServerAttentionSema, 0, SETVAL, 0) == -1) {
            TARATI_ERROR("internal semaphore error: " << strerror(errno));
        }
        // and pick up all accumulated work
        xmlrpcServer->work(timeout);
    }
}


/**
 * Wait until there is work to do. (Might give false positives)
 */
void
Server::Wait (void)
{
    // wait until semaphore can be decremented by 1
    struct sembuf operation;
    operation.sem_num = 0;
    operation.sem_op = -1; // decrement by 1
    operation.sem_flg = 0;
    int result;
    while ((result = semop (ServerAttentionSema, &operation, 1)) != 0) {
        if (errno == EINTR) {
            // go back to wait if we woke up for unrelated reasons
            continue;
        }
        TARATI_ERROR("internal semaphore error: " << strerror(errno));
    }
}

} // namespace Tarati
