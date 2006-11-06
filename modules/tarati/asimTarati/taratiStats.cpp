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

// generic

// ASIM local modules
#include "taratiStats.h"

namespace AsimTarati {

//----------------------------------------------------------------------------
// Tarati Services
//----------------------------------------------------------------------------
/**
 * Initialize all components of Stats Tarati Service and register Tarati
 * Methods provided by this service.
 * @note: For now, this interface is exactly the same as is presented to
 * the internal TCL client via PmStateCmd().
 */
Stats::Stats(
    Server * server,
    ASIM_SYSTEM system)
  : Service(server, name, version)
{
    // setup knowledge about PM
    pmSystem = system;
    pmState = NULL;

    // create a linked list of all state of the PM
    STATE_ITERATOR_CLASS iter(system, true);
    ASIM_STATE state;
    while ((state = iter.Next()) != NULL) {
        pmState = new ASIM_STATELINK_CLASS(state, pmState, false);
    }

    // instantiate and register methods
    method.states = new States(this);
    method.find = new Find(this);
    method.path = new Path(this);
    method.name = new Name(this);
    method.desc = new Desc(this);
    method.value = new Value(this);
}

/**
 * Unregister all Tarati Methods associated with this service and clean up
 * locally allocated memory.
 */
Stats::~Stats()
{
    // delete methods
    delete method.states;
    delete method.find;
    delete method.path;
    delete method.name;
    delete method.desc;
    delete method.value;

    // forget what we know about PM
    pmSystem = NULL;

    // delete allocated memory
    while (pmState) {
        ASIM_STATELINK nextState = pmState->next;
        delete pmState;
        pmState = nextState;
    }
}

//----------------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------------
//----- States -----
void
Stats::States::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    const char * path;
    int results = 0;

    // get params
    if (params.getType() == XmlRpcValue::TypeInvalid) {
        path = NULL;
    } else {
        ASSERTX(params.getType() == XmlRpcValue::TypeArray);
        if (params.size() != 1) {
            throw XmlRpcException("wrong number of arguments in method call");
        }
        path = ((string) params[0]).c_str();
    }

    ASIM_STATELINK scan = stats->pmState;
    while (scan != NULL) {
        ASIM_STATE state = scan->state;

        if ((path == NULL) ||
            (strcasecmp(state->Path(), path) == 0))
        {
            // If 'path' equals 'state's path, then return 'state's descriptor.
            result[results] = (int) state;
            // result[results]["path"] = state->Path();
            // result[results]["name"] = state->Name();
            // result[results]["desc"] = state->Description();
            results++;
        }

        scan = scan->next;
    }
}

//----- Find -----
void
Stats::Find::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    const char * name;

    // get params
    if (params.getType() != XmlRpcValue::TypeArray ||
        params.size() != 1)
    {
        throw XmlRpcException("wrong number of arguments in method call");
    }
    name = ((string) params[0]).c_str();

    ASIM_STATELINK scan = stats->pmState;
    while (scan != NULL) {
        ASIM_STATE state = scan->state;

        if (strcasecmp(state->Name(), name) == 0) {
            result[0] = (int) state;
            return;
        }

        scan = scan->next;
    }
}

static inline
ASIM_STATE
ParamToState (
    XmlRpcValue & params)
{
    // get params
    if (params.getType() != XmlRpcValue::TypeArray ||
        params.size() != 1)
    {
        throw XmlRpcException("wrong number of arguments in method call");
    }
    
    return (ASIM_STATE) ((int) params[0]);
}

//----- Path -----
void
Stats::Path::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    result = ParamToState(params)->Path();
}

//----- Name -----
void
Stats::Name::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    result = ParamToState(params)->Name();
}

//----- Desc -----
void
Stats::Desc::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    result = ParamToState(params)->Description();
}

//----- Value -----
void
Stats::Value::execute(
    XmlRpcValue & params,
    XmlRpcValue & result)
{
    ASIM_STATE state = ParamToState(params);

    // bluntly copied from awb.cpp:PmStateValue()
    if (state != NULL) {
        // Iterate through 'state's values, appending them to the result list
        switch (state->Type()) {
          case STATE_UINT:
            for (UINT32 i = 0; i < state->Size(); i++) {
                result[i] = (int) state->IntValue(i);
            }
            break;
          case STATE_FP:
            for (UINT32 i = 0; i < state->Size(); i++) {
                result[i] = state->FpValue(i);
            }
          case STATE_HISTOGRAM:
            result = "state type STATE_HISTOGRAM not supported";
            break;
          case STATE_RESOURCE:
            result = "state type STATE_RESOURCE not supported";
            break;
          case STATE_THREE_DIM_HISTOGRAM:
            result = "state type STATE_THREE_DIM_HISTOGRAM not supported";
            break;
          default:
            ostringstream os;
            os << "unknown state type " << state->Type();
            result = os.str();
        }
    }
}

} // namespace AsimTarati
