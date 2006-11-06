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
//
// Author:  Srilatha Manne
//


#ifndef _PERINST_STATS_
#define _PERINST_STATS_

#include <stdio.h>
#include <memory.h>


//
// This file contains all statistics which the user would like to keep 
// per instruction.  Some of the stats are kept per instruction and 
// accumulated for all instructions, while other stats are kept globally,
// only.  The user can decide where they want to place the stats 
// based on which class they enter the stat into. 
//

typedef enum perinst_counter
{
    // Generic stats
    DYN_INST_COUNTER = 0,
    ISSUE_COUNTER,

    // Stats specific to CTRL operations
    CTRL_MP_COUNTER,

    // Stats specific to memory ops
    DC_MISS_COUNTER,
    MC_MISS_COUNTER,

    // End or array
    LAST_COUNTER
} PERINST_COUNTER;

//
// String associated with enum for printing purposes.  Must be in the
// same order as the enum. 
static
const char* PERINST_COUNTER_STRING[] = 
{
    "DYN_INST",
    "ISSUE", 
    "CTRL_MP", 
    "DC_MISS",
    "MC_MISS",
    "LAST"
};

typedef class PERINST_STATS_CLASS* PERINST_STATS;

//
// This class contains all stats you want to collect and store on a 
// per instruction basis.  These stats are accumulated and associated
// with a particular static instruction.  They are also tallied up 
// globally, for all instructions.  
//
class PERINST_STATS_CLASS {
  private:
    
    // Array for colleting stats
    UINT64 stat[LAST_COUNTER];

  public:
    // Constructor
    PERINST_STATS_CLASS();

    // Place all accessors here.
    UINT64 Get(PERINST_COUNTER c);

    // Place all modifiers here. 
    void Inc(PERINST_COUNTER c, UINT64 val = 1);
    void Set(PERINST_COUNTER c, UINT64 val);
    void Clear(PERINST_COUNTER c);
    void Dec(PERINST_COUNTER c, UINT64 val = 1);

    // Place all modifiers here. 
    void Dump();

    // Overload the + operator so that we can accumulate the stats for
    // all dynamic instances of a static instruction. 
    const PERINST_STATS_CLASS& operator+=(const PERINST_STATS_CLASS& right);

};

inline
PERINST_STATS_CLASS::PERINST_STATS_CLASS()
{
    // Initialize array
    for (UINT32 i = 0; i < LAST_COUNTER; i++)
    {
        stat[i] = 0;
    }
}

//
// Accumulate stats.
inline const PERINST_STATS_CLASS& 
PERINST_STATS_CLASS::operator+=(const PERINST_STATS_CLASS& right)
{
    for (UINT32 i = 0; i < LAST_COUNTER; i++)
    {
        if (i == DYN_INST_COUNTER)
        {
            // Increment the number of dynamic instances when we
            // accumulate stats.  The user should never increment this
            // particular counter. 
            stat[i]++;
        }
        else
        {
            stat[i] += right.stat[i];
        }
    }

    return *this;
}

// Increment a stat
inline void
PERINST_STATS_CLASS::Inc(PERINST_COUNTER c, UINT64 val)
{
    ASSERTX(c < LAST_COUNTER);
    stat[c] += val;
}

// Decrement a stat
inline void
PERINST_STATS_CLASS::Dec(PERINST_COUNTER c, UINT64 val)
{
    ASSERTX(c < LAST_COUNTER);
    ASSERTX(stat[c] >= val);
    stat[c] -= val;
}

// Set to a specific value
inline void
PERINST_STATS_CLASS::Set(PERINST_COUNTER c, UINT64 val)
{
    ASSERTX(c < LAST_COUNTER);
    stat[c] = val;
}

// Clear value
inline void
PERINST_STATS_CLASS::Clear(PERINST_COUNTER c)
{
    Set(c, 0);
}

// Get value for stat
inline UINT64
PERINST_STATS_CLASS::Get(PERINST_COUNTER c)
{
    ASSERTX(c < LAST_COUNTER);
    return stat[c];
}

inline void
PERINST_STATS_CLASS::Dump()
{
    for (UINT32 i = 0; i < LAST_COUNTER; i++)
    {
        cout << i << ": " << stat[i] << endl;
    }
}


#endif // _PERINST_STATS_
