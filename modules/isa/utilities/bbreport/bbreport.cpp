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

// ASIM core
#include "asim/mesg.h"
#include "asim/trace.h"

// ASIM public modules
#include "asim/provides/util_bbreport.h"

BBREPORT_CLASS::BBREPORT_CLASS(void)
{
}

BBREPORT_CLASS::~BBREPORT_CLASS(void)
{
    map<IADDR_CLASS, BB_INSTR_CLASS *>::iterator i_iter;
    for (i_iter = instrTable.begin(); i_iter != instrTable.end(); i_iter++)
    {
        delete i_iter->second;
    }
    instrTable.clear();
}

void
BBREPORT_CLASS::Print(ostream & os)
{
    UINT64 total_blocks = 0;
    UINT64 total_size = 0;
    UINT64 total_count = 0;
    UINT64 total_delay = 0;

    // basic blocks sorted by retire delay
    multimap<UINT64, BB_BLOCK_CLASS *> blockTable;

    BB_BLOCK_CLASS * cur_block = new BB_BLOCK_CLASS;
    cur_block->size = 0;
    cur_block->count = 0;
    cur_block->delay = 0;

    // walk through map (already sorted) and create blocks
    map<IADDR_CLASS, BB_INSTR_CLASS *>::iterator i_iter;
    for (i_iter = instrTable.begin(); i_iter != instrTable.end(); i_iter++)
    {
        if (cur_block->size == 0)
        {
            total_blocks++;
            cur_block->addr = i_iter->second->addr;
        }

        cur_block->size++;
        cur_block->count += i_iter->second->count;
        cur_block->delay += i_iter->second->delay;

        total_size++;
        total_count += i_iter->second->count;
        total_delay += i_iter->second->delay;

        bool terminate = false;
        if (i_iter->second->breakout == true)
        {
            terminate = true;
        }
        if (++i_iter != instrTable.end())
        {
            if (i_iter->second->breakin == true)
            {
                terminate = true;
            }
        }

        --i_iter;

        if (terminate == true)
        {
            blockTable.insert(pair<UINT64, BB_BLOCK_CLASS *>(cur_block->delay, cur_block));

            cur_block = new BB_BLOCK_CLASS;
            cur_block->size = 0;
            cur_block->count = 0;
            cur_block->delay = 0;
        }
    }

    // walk backwards through map (already sorted) and print blocks
    os << endl;
    os << "Unique basic blocks: " << total_blocks << endl;
    os << "Unique instructions: " << total_size << endl;
    os << "Total instructions : " << total_count << endl;
    os << "Total retire delay : " << total_delay << endl;

    multimap<UINT64, BB_BLOCK_CLASS *>::reverse_iterator b_iter;
    UINT64 BB_sort_index = 0;
    for (b_iter = blockTable.rbegin(); b_iter != blockTable.rend(); b_iter++)
    {
        if (BB_sort_index++ < BBREPORT_SIZE)
        {
            os << "Addr: " << b_iter->second->addr
               << " Count: " << b_iter->second->count
               << " Size: " << b_iter->second->size
               << " Delay: " << b_iter->second->delay
               << " (" << fmt("5.2f", ((double) 100.0 * b_iter->second->delay / total_delay)) << "%)" << endl;
            IADDR_CLASS addr = b_iter->second->addr;
            for (UINT64 i = 0; i < b_iter->second->size; i++)
            {
                i_iter = instrTable.find(addr);
                ASSERTX(i_iter != instrTable.end());

                os << "  [" << addr << "] "
                   << fmt("-50", i_iter->second->name)
                   << " count " << fmt("6", i_iter->second->count)
                   << " delay " << fmt("6", i_iter->second->delay) << endl;

                addr = addr.Next();
            }
            os << endl;
        }

        delete b_iter->second;
    }
    blockTable.clear();
}

void
BBREPORT_CLASS::Commit(const IADDR_CLASS & addr, const string name, UINT64 delay)
{
    TRACE(Trace_Sys, cout << "BBREPORT::Commit" << endl);
    TRACE(Trace_Sys, cout << "\tInstruction: Addr " << addr << endl);

    bool split = (addr != lastAddr.Next());

    map<IADDR_CLASS, BB_INSTR_CLASS *>::iterator iter = instrTable.find(addr);

    if (iter != instrTable.end())
    {
        TRACE(Trace_Sys, cout << "\tAddr match found, updating" << endl);
        iter->second->count += 1;
        iter->second->delay += delay;
    }
    else
    {
        TRACE(Trace_Sys, cout << "\tCreating new instance" << endl);
        BB_INSTR_CLASS * cur_instr = new BB_INSTR_CLASS;
        cur_instr->addr = addr;
        cur_instr->name = name;
        cur_instr->count = 1;
        cur_instr->delay = delay;
        cur_instr->breakin = false;
        cur_instr->breakout = false;

        iter = instrTable.insert(pair<IADDR_CLASS, BB_INSTR_CLASS *>(cur_instr->addr, cur_instr)).first;
    }

    if (split == true)
    {
        iter->second->breakin = true;

        iter = instrTable.find(lastAddr);
        if (iter != instrTable.end())
        {
            iter->second->breakout = true;
        }
    }
    lastAddr = addr;
}
