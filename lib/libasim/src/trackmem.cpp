/*
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
 * @author Shubu Mukherjee
 * @brief Debugging support: track memory allocated by different routines.
 */

// ASIM core - needs to be outside of #ifdef TRACK_MEMORY_ALLOCATION
#include "asim/trackmem.h"

UINT64 trackCycle = 0; 

#ifdef TRACK_MEMORY_ALLOCATION

// generic
#include <excpt.h>
#include <obj.h>
#include <sym.h>
#include <st.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <iostream>
#include <sstream>

// ASIM core
#include "asim/mesg.h"
#include "asim/ioformat.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

// Following code is Harish Patil's stack trace code from spike

typedef struct st_obj *ST_OBJ; 
static ST_OBJ asim_st_obj;

// Pass the filaname string to initialize the trace
void StackTraceInit(char * filename)
{
    st_bool_t self_st_obj_stripped;

    /* 
     * Open the 'self' object.  For C++, name demangling is enabled
     * when an object is successfully opened.
     */

    if(st_obj_open(&asim_st_obj, filename, ST_RDONLY | ST_MANGLE_NAMES))
    {
        asim_st_obj = NULL;
        return;
    }

    st_is_obj_stripped(asim_st_obj, &self_st_obj_stripped);

    if (self_st_obj_stripped) 
    {
        asim_st_obj = NULL;
        return;
    } 
}

char *PrintProc(ST_OBJ self_st_obj, long pc)
{
    st_file_t file;
    st_proc_t proc;
    st_proc_t last_proc;
    unsigned int fcount;
    unsigned int pcount;
    char * fname;
    char * pname;
    char * last_pname;
    st_bool_t stripped;
    st_addr_t paddr;
    st_addr_t last_paddr = 0;
    st_status_t ret;
    st_status_t last_ret;
    int j;
    

    /* 
     * Get handle for first procedure for this object, and the
     * count of procedures in the object.
     */

    st_obj_proc_count(self_st_obj, &pcount);
    st_proc_sorted_start(self_st_obj, &proc);
    
    for(j=0; j < pcount; j++) 
    {
        ret = st_proc_name(self_st_obj, proc, &pname);
        st_proc_addr(self_st_obj, proc, &paddr);
        if( pc >= last_paddr && pc <= paddr)
        {
            st_bool_t stripped;
            st_line_t line=0;
            ostringstream lineNo;
            ostringstream os;
            
            st_proc_to_file(self_st_obj, last_proc, &file); 
            st_is_file_locally_stripped(self_st_obj, file, &stripped);
            if(stripped) 
            {
                /* File is locally stripped - name is unavailable */
                fname = NULL;
                lineNo << "UnKnown";
            }
            else 
            {
                st_file_name(self_st_obj, file, &fname);
                st_addr_to_line(self_st_obj, (st_addr_t)pc, &line);
                lineNo << line;
            }
            if(!last_ret)
            {
                //os << last_pname << ":\""
                //   << (fname ? fname : "NoFileName")
                //   << "\":" << lineNo << endl;
		return last_pname; 
            }
            
            else
            {
                //os << "Procedure at 0x" << fmt_p(last_paddr) << ":\""
                //   << (fname ? fname : "NoFileName")
                //   << "\":" << lineNo << endl;
            }
            //fputs(os.str().c_str(), fp);
            break;
        }
        last_paddr = paddr;
        last_pname = pname;
        last_ret = ret;
        last_proc = proc;
        st_proc_sorted_next(self_st_obj, proc, &proc);
    }
    // fflush(fp);

    return NULL; 
}

char *StackTraceParentProc()
{
    unsigned long skip_frame_count = 1; 
    unsigned int frame_no = 0;
    struct sigcontext  contextRecord;
    long current_pc = 0;
    ostringstream os;

    if(asim_st_obj == NULL)
    {
        //fputs ("NO STACKTRACE  no executable specified\n", fp);
        return NULL; 
    }

    st_proc_sort(asim_st_obj);

    /* Print a stack trace */

    exc_capture_context(&contextRecord);

    current_pc =  contextRecord.__XSE(sc_pc);

    /* Skip printing info about 'this' procedure */

    frame_no = 0;
    while(1) 
    {
        exc_virtual_unwind( 0, &contextRecord);
        current_pc =  contextRecord.__XSE(sc_pc);
        if(current_pc == 0) break;
        frame_no += 1;
        if(frame_no > skip_frame_count)
        {
            os.str(""); // clear
            os << " (0x" << fmt_x(current_pc) << "):";
            fputs(os.str().c_str(), fp);
            return PrintProc(asim_st_obj, current_pc);
        } 
    }
    // fflush(fp);

    return NULL; 
}

//
// Following is overloaded new and delete to track memory allocation 
// 


#define MaxTrackMemAlloc	1000
#define MaxStringLen		100 
#define BEGIN_TRACKCYCLE	10
#define MAX_PROCNAME_SIZE	1000

UINT64 uniqueCallSites = 0; 

class TRACK_MEMORY_ALLOC
{
  public:
    TRACK_MEMORY_ALLOC()	{ clean(); }
    void clean()		
    { 	
	bytes = 0; 
	procNameSet = false; 
	memset(procName, 0, sizeof(char) * MAX_PROCNAME_SIZE); 
    }

    bool	procNameSet; 			// name set? 
    char        procName[MAX_PROCNAME_SIZE]; 	// procedure name
    UINT64	bytes; 				// bytes allocated


} trackMemAlloc[MaxStringLen][MaxTrackMemAlloc]; 



void *operator new(size_t s)
{
    // Record malloc-ed procedure name and size
    extern bool statsOn; 

    if (trackCycle > BEGIN_TRACKCYCLE)
    {
	bool found = false; 
	char *pname = StackTraceParentProc(); 
	UINT32 fullLen = strlen(pname); 
	UINT32 plen = fullLen % MaxStringLen; 
	int i; 	

	ASSERT(fullLen < MAX_PROCNAME_SIZE, "Increase MAX_PROCNAME_SIZE\n");  
	
	for (i = 0; i < MaxTrackMemAlloc; i++)
	{
	    if (trackMemAlloc[plen][i].procNameSet == false)
	    {
		break; 
	    }
	    else if (!strcmp(pname, trackMemAlloc[plen][i].procName))
	    {
		found = true; 
		break; 
	    }
	}
	
	VERIFY(i < MaxTrackMemAlloc, "Increase MaxTrackMemAlloc\n"); 
	
	if (found)
	{
	    trackMemAlloc[plen][i].bytes += s; 
	}
	else
	{
	    strncpy(trackMemAlloc[plen][i].procName, pname, fullLen); 
	    trackMemAlloc[plen][i].procNameSet = true; 
	    trackMemAlloc[plen][i].bytes = s; 	

	    uniqueCallSites++; 
	}
    }	
    
    return malloc(s); 
}

void operator delete (void *s)
{
    free(s); 
}

void DumpMemAllocInfo(STATE_OUT stateOut)
{
    int i;
    int j; 
    int t = 0; 
    ostringstream os;

    for (i = 0; i < 116; i++)
    {
	os << "-"; 
    }

    os << endl;

    for (j = 0; j < MaxStringLen; j++)
    {
	for (i = 0; i < MaxTrackMemAlloc; i++)
	{
	    if (trackMemAlloc[j][i].procNameSet == false)
	    {
		break; 
	    }
	    
	    os << fmt("-100" << trackMemAlloc[j][i].procName)
               << " " << fmt("15", trackMemAlloc[j][i].bytes) << endl; 
	    t++; 
	}
    }

    os << "Unique Call Sites " << uniqueCallSites << ", printed " << t;

    for (i = 0; i < 116; i++)
    {
	os << "-"; 
    }

    os << endl;

    stateOut->AddText(os.str().c_str());
}

#else

char *StackTraceParentProc()
{
    return NULL; 
}
#endif
