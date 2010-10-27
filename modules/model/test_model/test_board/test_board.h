/*****************************************************************************
 *
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

#ifndef _BOARD_CLASS_
#define _BOARD_CLASS_

// ASIM core
#include "asim/syntax.h"
#include "asim/module.h"
#include "asim/port.h"

// ASIM messages

/**
 * Class ASIM_BOARD_CLASS
 *
 *
 */

typedef class ASIM_BOARD_CLASS * ASIM_BOARD;

class ASIM_BOARD_CLASS : public ASIM_MODULE_CLASS
{

  public:

    // Constructor
    ASIM_BOARD_CLASS(ASIM_MODULE parent, const char * const name);

    // Destructor
    ~ASIM_BOARD_CLASS();

    // Required by ASIM
    bool InitModule();

    // Do a cycle of work...
    void Clock (UINT64 cycle);

    // Additional ASIM_BOARD public methods
    void DumpStats(STATE_OUT state_out, UINT64 statCycles, UINT64 * committed_insn);

};

#endif /* _BOARD_CLASS_ */
