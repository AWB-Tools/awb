/*
 *Copyright (C) 2004-2006 Intel Corporation
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
 * @author Shubu Mukherjee
 * @brief ASIM's tracing interface - performance model debugging.
 */

// ASIM core
#include "asim/syntax.h"
#include "asim/trace.h"
#include "asim/threaded_log.h"
#include "asim/message_handler_log.h"


#include <stdlib.h>
#include <ostream>
#include <strings.h>
#include <string.h>
#include <iomanip>
#include <list>
using namespace std;


bool warningsOn = false;                // warning information on or off
bool debugOn = false;                   // debugging information on or off

UINT32 traceMask = Trace_AllbutBP; 	// by default, trace everything but the Branch Predictor
bool traceOn = false;   		// trace on or off


namespace IoFormat 
{
    const Format fmt_node  = Format("N", "02d", ": ");
    const Format fmt_slice = Format("S", "02d", ": ");
    const Format fmt_cpu   = Format("C", "02d", ": ");
    const Format fmt_chip  = Format("Ch", "1d", ": ");
}

class TRACE_OPTS_CLASS 
{
  public:
    char* name;
    UINT32 code;
};


static TRACE_OPTS_CLASS trace_opts[] =
{
    { "All", Trace_All  }, 
    { "AllbutBP", Trace_AllbutBP }, 
    { "Ibox", Trace_Ibox  }, 
    { "Fetch", Trace_Fetch  }, 
    { "Pbox", Trace_Pbox  }, 
    { "Expand", Trace_Expand }, 
    { "Qbox", Trace_Qbox  }, 
    { "Issue", Trace_Issue }, 
    { "Ebox", Trace_Ebox  }, 
    { "Execute", Trace_Execute }, 
    { "Mbox", Trace_Mbox  }, 
    { "Cbox", Trace_Cbox  }, 
    { "Xbox", Trace_Xbox  }, 
    { "Cmd", Trace_Cmd  }, 
    { "Sys", Trace_Sys  }, 
    { "Feeder", Trace_Feeder  }, 
    { "Micro_Feeder", Trace_Macro_Feeder  }, 
    { "Macro_Feeder", Trace_Micro_Feeder  }, 
    { "Vbox", Trace_Vbox  }, 
    { "Inst", Trace_Inst }, 
    { "EapOracle", Trace_EapOracle }, 
    { "Ubox", Trace_Ubox }, 
    { "Ring", Trace_Ring }, 
    { "Zbox", Trace_Zbox }, 
    { "Ports", Trace_Ports }, 
    { "Czu", Trace_Czu  }, 
    { "Rambus", Trace_Rambus  }, 
    { "Rbox", Trace_Rbox  }, 
    { "Slice", Trace_Slice  }, 
    { "Context", Trace_Context  }, 
    { "Retire", Trace_Retire }, 
    { "BP", Trace_BP  }, 
    { "Debug", Trace_Debug  }, 
    { "Mtsched" , Trace_Mtsched },	
    { 0, 0 } 
};

void
print_trace_masks(std::ostream& o)
{
    o << "Trace masks: ";
    for(UINT32 i=0;trace_opts[i].name ; i++)
    {
        if ((i%7) == 0)
        {
            o << endl << "\t";
        }
        o << std::setw(9) << trace_opts[i].name << " ";
    }
    o << endl;
}

static UINT32
find_opt(char* p)
{
    UINT32 i=0;
    while (trace_opts[i].name)
    {
        if (strcasecmp(trace_opts[i].name, p) == 0)
        {
            // cerr << "FOUND: " << p << endl;
            return trace_opts[i].code;
        }
        i++;
    }
    return 0;
}



typedef list<char*> CHAR_LIST;

static CHAR_LIST*
split(char* s, char delim)
{
    char* q = s;
    CHAR_LIST* l = new CHAR_LIST;
    while(q)
    {
        char* t = strchr(q,delim);
        UINT32 len = (t) ? (t-q) : strlen(q);
        if (len) // avoid the ".*,,.*" problem or the ".*,$" problem
        {
            char* z = new char[len+1];
            strncpy(z, q, len);
            z[len] = 0; 
            //cerr << "Appending [" << z << "]" << endl;
            l->push_back(z);
        }
        // skip over the delim or we get stuck!
        q = (t) ? (t+1) : 0;
    }
    if (l->empty())
    {
        cerr << "There were no \"" << delim
             << "\"-separated strings in the list of trace mask strings: ["
             << s << "]"
             << endl;
        exit(1);
    }
    return l;
}


UINT32 find_trace_opt(char* str)
{
    UINT32 total_opt = 0;
    CHAR_LIST* l = split(str,',');
    while(! l->empty() )
    {
        char* p = l->front();
        l->pop_front();
        UINT32 mask =  find_opt(p);
        //cerr << "Testing [" << p << "]" <<  endl;
        if (mask == 0)
        {
            cerr << "ERROR: Did not find a trace bit mask for ["
                 << p << "]" << endl;
            cerr << "       Use -listmasks to see the valid masks." << endl;
            exit(1);
        }
        total_opt |= mask;
    } 
    delete l;
    // cerr << hex << "TraceOpt = 0x" << total_opt << dec << endl;
    return total_opt;
}

