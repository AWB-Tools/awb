/**
 *Copyright (C) 2002-2006 Intel Corporation
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
 * @author Jim Vash
 * @brief Basic Block Reporter
 */

#ifndef _BBREPORT_
#define _BBREPORT_

// generic (C++/STL)
#include <map>

// ASIM core
#include "asim/syntax.h"

// ASIM public modules -- BAD! in asim-core
#include "asim/provides/isa.h"

class BBREPORT_CLASS
{
  private:

    // info about current block
    IADDR_CLASS lastAddr;

    // hash table for instructions
    class BB_INSTR_CLASS
    {
      public:
        IADDR_CLASS addr;
        string name;
        UINT64 count;
        UINT64 delay;

        bool breakin;
        bool breakout;
    };

    // sorted by IP
    map<IADDR_CLASS, BB_INSTR_CLASS *> instrTable;

    // hash table for basic blocks
    class BB_BLOCK_CLASS
    {
      public:
        IADDR_CLASS addr;
        UINT64 size;
        UINT64 count;
        UINT64 delay;
    };

  public:

    BBREPORT_CLASS();
    ~BBREPORT_CLASS();

    void Print(ostream & os);
    void Commit(const IADDR_CLASS & addr, const string name, UINT64 delay);

};

#endif /* _BBREPORT_ */
