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

#ifndef _INST_PROFILE_
#define _INST_PROFILE_

#include <stdio.h>
#include <memory.h>
#include <map>
#include "asim/registry.h"
#include "asim/atomic.h"
#include "asim/restricted/perinst_stats.h"

static UID_GEN32 numInst = 0;

#define ENTRIES_PER_PAGE (1 << LOG2_ENTRIES_PER_PAGE)


/* This class provides per static instruction profile information.
   The amount of memory required to run this is a function of the
   number of unique static instructions in the code.  For example,
   you require a minimum of 10Mbytes of space to store 40,000 static
   inst profile.  Hence be careful when you use this profiling 
   algorithm. 

   The number of maximum static instructions the data structure can
   handle is 
   
   MAX_RECORDING_POINTS * (24 bytes/recording_point) * ENTRIES_PER_PAGE *
           MAX_NUM_PAGES
   
   Note that there will be an assertion failure if the number of pages exceeds
   781 even if each page has not reached the 2048 maximum number of inst. 
   
*/

typedef class PERINST_ENTRY_CLASS* PERINST_ENTRY;
typedef class PERINST_PAGE_CLASS* PERINST_PAGE;
typedef class PERINST_CACHE_CLASS* PERINST_CACHE;
//
// Data structure for each static inst. or instset the program encounters.  
class PERINST_ENTRY_CLASS {
  private:
    // 
    // Structures for holding event count information for committed and
    // non-committed instructions. 
    //
    PERINST_STATS_CLASS commInst, noncommInst;
    
    // Unique address of instruction we're storing
    UINT64 addr;

    // Description of instruction 
    char des[100];

    // Histograms used to print out final information
    HISTOGRAM_TEMPLATE<1> commHist;
    HISTOGRAM_TEMPLATE<1> noncommHist;
  
  public:
    //
    // NULL constructor.
    PERINST_ENTRY_CLASS();
    PERINST_ENTRY_CLASS (const char *name, UINT64 addr);
    ~PERINST_ENTRY_CLASS();
    
    // 
    // Do we have any events currently? 
    bool EmptyEvents();

    // 
    // Update the entry with info from latest dynamic instruction. 
    void UpdateInst(const PERINST_STATS_CLASS& ifs, bool commit); 

    // 
    // Register the histogram for this instruction. 
    void RegisterEntry(ASIM_REGISTRY reg);

    // Accessors
    UINT64 GetAddr();
};

//
// Null constructor
//
inline 
PERINST_ENTRY_CLASS::PERINST_ENTRY_CLASS() :
    addr(0)
{}

inline 
PERINST_ENTRY_CLASS::~PERINST_ENTRY_CLASS()
{
}

//
// Method returns info stating whether this instruction has any stats. 
//
/*
inline bool 
PERINST_ENTRY_CLASS::EmptyEvents() 
{ 
    return (commInst.stat[DYN_INST_COUNTER] == 0 && 
            noncommInst.stat[DYN_INST_COUNTER] == 0); 
}
*/

// 
// This method not only copies the information from in flight stats, 
// but it also does the calculations of the deltas between events.
inline void 
PERINST_ENTRY_CLASS::UpdateInst(const PERINST_STATS_CLASS &ifs, 
           bool commit)
{
    if (commit) 
    {
//        cout << "Accumulating committed inst stats" << endl;
        commInst += ifs;
//        commInst.Dump();
    }
    else 
    {
//        cout << "Accumulating non-committed inst stats" << endl;
        noncommInst += ifs;
//        noncommInst.Dump();
    }
}

inline UINT64
PERINST_ENTRY_CLASS::GetAddr()
{
    return addr;
}
    
// Data structure for one page of instructions. 
class PERINST_PAGE_CLASS
{
  private:
    PERINST_ENTRY page[ENTRIES_PER_PAGE];
    UINT64 pageAddr;
    
  public:
    
    PERINST_PAGE_CLASS(UINT64 a);
    ~PERINST_PAGE_CLASS();

    // Accessors
    UINT64 PageAddr();

    // Modifiers
    void UpdateInst(const UINT64 addr, const char* inst_name, 
                    const PERINST_STATS_CLASS& ifs, 
                    bool commit);
    void RegisterPage(ASIM_REGISTRY reg);

};

inline
PERINST_PAGE_CLASS::PERINST_PAGE_CLASS(UINT64 a) : pageAddr(a) 
{
    for (UINT32 i = 0; i < ENTRIES_PER_PAGE; i++) 
    {
        page[i] = NULL;
    }
}

inline UINT64
PERINST_PAGE_CLASS::PageAddr() 
{ 
    return pageAddr; 
}


/*
 * Class perinst_cache_class contains all perinst stats for the program. It
 * contains a map whose entries are pages. Each page contains a number of
 * static instructions.
 */
class PERINST_CACHE_CLASS
{
  private:
    PERINST_PAGE lastPage;
  
    // This map contains all stats accumulated per static
    // instruction.
    typedef map<UINT64, PERINST_PAGE> STAT_CACHE;
    STAT_CACHE statCache;

 public:
    PERINST_CACHE_CLASS();
    ~PERINST_CACHE_CLASS();

    void UpdateStats(UINT64 addr, const char *name, 
                     const PERINST_STATS_CLASS &ifs, 
                     bool commit);

    void RegisterPerinstStats(ASIM_REGISTRY reg);
};


inline
PERINST_CACHE_CLASS::PERINST_CACHE_CLASS() : lastPage(NULL)
{
    ASSERT(ENABLE_INST_PROFILE <= 2, "Legal values are 0, 1, and 2");
}

#endif // _INST_PROFILE_
