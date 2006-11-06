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
 * @brief ASIM Tarati Service: Global State
 */

#ifndef _TARATI_GLOBAL_STATE_
#define _TARATI_GLOBAL_STATE_

// generic

// ASIM public modules
#include "asim/provides/system.h"
#include "asim/provides/tarati.h"

using namespace Tarati;

namespace AsimTarati {

//----------------------------------------------------------------------------
// ASIM Tarati Service: GlobalState
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Services
//----------------------------------------------------------------------------
/**
 * @brief Tarati Service: Global state of model
 */
class GlobalState
  : public Service
{
  private:
    static char * const name    = "GlobalState";
    static char * const version = "0.1";

    //------------------------------------------------------------------------
    // Methods
    //------------------------------------------------------------------------
    /**
     * @brief Tarati Method: Global State Uptime
     */
    class Uptime
      : public Method
    {
      private:
        ASIM_SYSTEM system;

      public:
        Uptime(Service * service, ASIM_SYSTEM _system)
          : Method(service, "Uptime"),
            system(_system)
        {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /**
     * Note: temporary hack just for testing purposes - this is not to stay
     * @brief Tarati Method: Hello
     */
    class Hello
      : public Method
    {
      public:
        Hello(Service * service)
          : Method(service, "Hello") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    struct _method {
        Uptime * uptime;
        Hello * hello;
    } method;

  public:
    GlobalState(Server * server, ASIM_SYSTEM system);
    ~GlobalState();
};

} // namespace AsimTarati

#endif /* _TARATI_GLOBAL_STATE_ */
