/*****************************************************************************
 *
 * @brief clockserver test board 
 *
 * @author Ramon Matas Navarro
 *
 *Copyright (C) 2005-2006 Intel Corporation
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

// Generic C++
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>

#include "asim/syntax.h"
#include "asim/provides/board.h"

/**
 * Instantiate module and submodules.
 */
ASIM_BOARD_CLASS::ASIM_BOARD_CLASS (
    ASIM_MODULE parent, ///< parent module
    const char * const name)   ///< name of this module
    : ASIM_MODULE_CLASS(parent, name)
{
    
    ostringstream os;
    
    for(UINT32 i = 0; i < NUM_DOMAINS; i++)
    {
        os.str("");                                
        os << "TEST_" << i;
        newClockDomain(os.str(), (float)i + 1.0);
    }
    
}

/**
 * Destroy module and submodules
 */
ASIM_BOARD_CLASS::~ASIM_BOARD_CLASS ()
{

}

/**
 * Initialize module and submodules
 */
bool
ASIM_BOARD_CLASS::InitModule ()
{
    bool ok = true;

    TRACE(Trace_Debug, cout << "0: Init Module " << Name() << endl);
    ostringstream os;
    
    for(UINT32 i = 0; i < NUM_THREADS;)
    {
        for(UINT32 j = 0; j < NUM_DOMAINS; j++)
        {
            os.str("");                                
            os << "TEST_" << j;            
            RegisterClock(os.str(), newCallback(this, &ASIM_BOARD_CLASS::Clock));
            
            i++;
        }
    }
    
    if ( !ok ) 
    {
        cout << Name() << ": InitModule failed" << endl;
    }

    return ok;
}

/**
 * Perform one cycle of work.
 */
void
ASIM_BOARD_CLASS::Clock (
    UINT64 cycle)        ///< current cycle
{
    
}

void
ASIM_BOARD_CLASS::DumpStats (
    STATE_OUT state_out, UINT64 statCycles, UINT64 * committed_inst)
{
}

