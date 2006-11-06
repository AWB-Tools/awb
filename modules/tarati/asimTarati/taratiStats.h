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
 * @brief ASIM Tarati Service: Stats
 */

#ifndef _TARATI_STATS_
#define _TARATI_STATS_

// ASIM core
#include "asim/state.h"

// ASIM public modules
#include "asim/provides/system.h"
#include "asim/provides/tarati.h"

using namespace Tarati;

namespace AsimTarati {

//----------------------------------------------------------------------------
// ASIM Tarati Service: Stats
//----------------------------------------------------------------------------
/**
 * @brief Tarati Service: Stats of model
 */
class Stats
  : public Service
{
  private:
    static char * const name    = "Stats";
    static char * const version = "0.1";

    //------------------------------------------------------------------------
    // Methods
    //------------------------------------------------------------------------
    /**
     * @brief Tarati Method: States(path)
     */
    class States
      : public Method
    {
      private:
        Stats * stats;

      public:
        States(Stats * _stats)
          : Method(_stats, "States"),
            stats(_stats) {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };
    friend class States;

    /**
     * @brief Tarati Method: Find(name)
     */
    class Find
      : public Method
    {
      private:
        Stats * stats;

      public:
        Find(Stats * _stats)
          : Method(_stats, "Find"),
            stats(_stats) {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };
    friend class Find;

    /**
     * @brief Tarati Method: Path(handle)
     */
    class Path
      : public Method
    {
      public:
        Path(Stats * _stats)
          : Method(_stats, "Path") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /**
     * @brief Tarati Method: Name(handle)
     */
    class Name
      : public Method
    {
      public:
        Name(Stats * _stats)
          : Method(_stats, "Name") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /**
     * @brief Tarati Method: Desc(handle)
     */
    class Desc
      : public Method
    {
      public:
        Desc(Stats * _stats)
          : Method(_stats, "Desc") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    /**
     * @brief Tarati Method: Value(handle)
     */
    class Value
      : public Method
    {
      public:
        Value(Stats * _stats)
          : Method(_stats, "Value") {};

        /// RPC call method
        void execute(XmlRpcValue& params, XmlRpcValue& result);
    };

    struct _method {
        States * states;
        Find * find;
        Path * path;
        Name * name;
        Desc * desc;
        Value * value;
    } method;

    //------------------------------------------------------------------------
    // Service
    //------------------------------------------------------------------------
    ASIM_SYSTEM    pmSystem; ///< root of the PM (system module)
    ASIM_STATELINK pmState;  ///< root of linked list of all PM state

  public:
    Stats(Server * server, ASIM_SYSTEM system);
    ~Stats();
};

} // namespace AsimTarati

#endif /* _TARATI_STATS_ */
