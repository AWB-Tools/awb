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

#include "asim/provides/inst_stats.h"

//
// PERINST_ENTRY
//

/**************************
 * Constructor
 **************************/


PERINST_ENTRY_CLASS::PERINST_ENTRY_CLASS(const char* des_str, UINT64 a) :
    addr(a), 
    commHist(LAST_COUNTER),
    noncommHist(LAST_COUNTER)
{

    strncpy (des, des_str, 99);
    
    //
    // Initialize row names.  We will register the stats at the end because
    // that's the only time we'll have a ASIM_REGISTRY object with which to
    // register the stats. 
    commHist.RowNames(PERINST_COUNTER_STRING);
    noncommHist.RowNames(PERINST_COUNTER_STRING);
}

/**************************
 * Stats Registration
 **************************/

void
PERINST_ENTRY_CLASS::RegisterEntry(ASIM_REGISTRY reg)
{
    // Register stats
    // Convert the address to a string. 
    ostringstream os1, os2;
    if (ENABLE_INST_PROFILE == 2)
    {
        os1 << "Committed_" << hex << addr;
        os2 << "NonCommitted_" << hex << addr;
    }
    else if (ENABLE_INST_PROFILE == 1 || ENABLE_INST_PROFILE == 0)
    {
        os1 << "Committed_" << des;
        os2 << "NonCommitted_" << des;
    }
    else 
    {
        ASSERTX("Unknown instruction profile type.");
    }

//    cout << "Address: " << os.str() << endl;

    reg->RegisterState(&commHist, os1.str().c_str(), des);
    reg->RegisterState(&noncommHist, os2.str().c_str(), des);

    // Update the histogram with the correct data.  
    for (UINT32 i = 0; i < LAST_COUNTER; i++)
    {
        commHist.AddEvent(i, 0, commInst.Get((PERINST_COUNTER)i));
        noncommHist.AddEvent(i, 0, noncommInst.Get((PERINST_COUNTER)i));
    }
}

//
// PERINST_PAGE
//

/**************************
 * Destructor
 **************************/
PERINST_PAGE_CLASS::~PERINST_PAGE_CLASS()
{
/*
    for (UINT32 i = 0; i < ENTRIES_PER_PAGE; i++) 
    {
        if (page[i] != NULL)
        {
            delete page[i];
        }
    }
*/
}


/**************************
 * Update information for an instruction. 
 **************************/

void 
PERINST_PAGE_CLASS::UpdateInst(const UINT64 addr, 
                               const char *inst_name, 
                               const PERINST_STATS_CLASS &ifs, 
                               bool commit) 
{
    UINT64 offset = addr - pageAddr;

//    cout << "Updating instruction, Addr: " << addr << ", offset: " << offset << endl;
    
    ASSERTX(offset < ENTRIES_PER_PAGE);
    if (page[offset] == NULL) 
    {
        //
        // Null pointer, which means first time inst is seen.
        page[offset] = new PERINST_ENTRY_CLASS(inst_name, addr);
        UINT32 n = numInst++;
        if (n > MAX_NUM_INSTS) 
        {
            VERIFY(false, "Exceeding maximum number of static inst: 100000\n");
        }
    }
    //
    // Update 
    ASSERTX(page[offset]->GetAddr() == addr);
        
    page[offset]->UpdateInst(ifs, commit);
}

/**************************
 * Register stats for a page
 **************************/
//
// Register a page of stats. 
void
PERINST_PAGE_CLASS::RegisterPage(ASIM_REGISTRY reg)
{
    for (UINT32 i = 0; i < ENTRIES_PER_PAGE; i++) 
    {
        if (page[i] != NULL)
        {
//            cout << "Registering page" << endl;
            page[i]->RegisterEntry(reg);
        }
    }
}



//
// PERINST_CACHE
//

/**************************
 * Destructor
 **************************/

PERINST_CACHE_CLASS::~PERINST_CACHE_CLASS()
{
    STAT_CACHE::iterator i = statCache.begin();
    STAT_CACHE::iterator prev;

    while (i != statCache.end())
    {
        delete (*i).second;
        prev = i;
        ++i;
        statCache.erase(prev);
    }
}

/**************************
 * Update stats 
 **************************/

void
PERINST_CACHE_CLASS::UpdateStats(UINT64 addr,
                                 const char* name, 
                                 const PERINST_STATS_CLASS& ifs, 
                                 bool commit)
{

    PERINST_PAGE page;
    
    //
    // Update per-inst stats.
    //
    if (lastPage && 
        (addr & (~((1 << LOG2_ENTRIES_PER_PAGE) - 1)))==lastPage->PageAddr()) 
    {
//            cout << "Using lastpage" << endl;
        page = lastPage;
    }
    else 
    {
        STAT_CACHE::iterator i = statCache.find((addr >> LOG2_ENTRIES_PER_PAGE));
        if (i != statCache.end()) {
            page = (*i).second;
            lastPage = page;
//                cout << "Finding page in stat cache" << endl;
        }
        else 
        {
            //
            // Page doesn't currently exist. 
            //
            UINT64 page_addr = addr & (~((1 << LOG2_ENTRIES_PER_PAGE) - 1));
            page = new PERINST_PAGE_CLASS(page_addr);
//                cout << "Creating new page" << endl;
            //
            // Place new page into cache
            //
            statCache[(addr >> LOG2_ENTRIES_PER_PAGE)] = page;
            lastPage = page;
        }
        }
    // Update stats per address
    page->UpdateInst(addr, name, ifs, commit);
}

/**************************
 * Register stats
 **************************/
void
PERINST_CACHE_CLASS::RegisterPerinstStats(ASIM_REGISTRY reg)
{
    STAT_CACHE::iterator i = statCache.begin();
    while (i != statCache.end())
    {
//            cout << "Registering page: " << i << endl;
        (*i).second->RegisterPage(reg);
        ++i;
    }
}
    
