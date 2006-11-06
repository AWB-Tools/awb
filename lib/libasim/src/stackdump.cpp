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
 * @author David Goodwin
 * @brief Debug routines - support stack trace dumping on Tru64 Unix
 */

#if defined(HOST_DUNIX)
// generic
#include <signal.h>
#include <pdsc.h>
#include <excpt.h>
#include <pthread_exception.h>
#include <machine/fpu.h>
#include <st.h>
#include <sym.h>
#include <syms.h>
#include <cmplrs/demangle_string.h>
#include <iostream>
#endif

// ASIM core
#include "asim/stackdump.h"
#ifdef HOST_DUNIX
 #include "asim/mesg.h"
#endif

/* Variables used for stack trace dumping support on Tru64 Unix */
#if defined(HOST_DUNIX)
static st_status_t ret;
static st_obj_t *self_st_obj;
#endif



/* Install signal handlers for stack trace dumping support on Tru64 Unix */
#if defined(HOST_DUNIX)

static bool debug_print_current_procedure_info(FILE *fp, st_obj_t *self_st_obj, long pc)
{
    st_file_t file;
    st_proc_t proc;
    st_proc_t last_proc = 0;
    int fcount;
    UINT32 pcount;
    char * fname;
    char * pname;
    char * last_pname = NULL;
    st_bool_t stripped;
    st_addr_t paddr;
    st_addr_t last_paddr = 0;
    st_status_t ret;
    st_status_t last_ret = 0;
    UINT32 j;
    

    /* 
     * Get handle for first procedure for this object, and the
     * count of procedures in the object.
     */

    st_obj_proc_count(self_st_obj, &pcount);
    st_proc_sorted_start(self_st_obj, &proc);
    
    for(j=0; j < pcount; j++) 
    {
        ret = st_proc_name(self_st_obj, proc, &pname);
        if(ret != 0) continue;
        ret = st_proc_addr(self_st_obj, proc, &paddr);
        if(ret != 0) continue;
        if( pc >= (long)last_paddr && pc <= (long)paddr)
        {
            st_bool_t stripped;
            st_line_t line=0;
            ostringstream lineNo;
            ostringstream os;
            
            st_proc_to_file(self_st_obj, last_proc, &file); 
            st_is_file_locally_stripped(self_st_obj, file, &stripped);
#if defined(__GNUC__)
  // Line number with gnu not reliable?
            lineNo << "UnKnown";
            if(stripped) 
            {
                /* File is locally stripped - name is unavailable */
                fname = NULL;
            }
            else 
            {
                st_file_name(self_st_obj, file, &fname);
                st_addr_to_line(self_st_obj, (st_addr_t)pc, &line);
            }
#else
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
#endif //defined(__GNUC__)
            if(!last_ret)
            {
                os << last_pname << ":"
                   << "\"" << (fname ? fname : "NoFileName") << "\":"
                   << lineNo;
            }
            
            else
            {
                os << "Procedure at 0x" << fmt_x(last_paddr) << ":"
                   << "\"" << (fname ? fname : "NoFileName") << "\":"
                   << lineNo;
            }
            fputs (os.str().c_str(), fp);
            break;
        }
        last_paddr = paddr;
        last_pname = pname;
        last_ret = ret;
        last_proc = proc;
        st_proc_sorted_next(self_st_obj, proc, &proc);
        fflush(fp);
    }
    if(strncmp(last_pname, "PmEventLoop", strlen("PmEventLoop")) == 0) return true; 
    return false;
}

extern void DEBUG_DumpStackTrace(int skip_frame_count)
{
    
    int frame_no = 0;
    struct sigcontext  contextRecord;
    long current_pc = 0;
    bool PmEventLoopReached = false;

    if(self_st_obj == NULL)
    {
        cerr << "NO STACKTRACE POSSIBLE" << endl;
        return;
    }

    st_proc_sort(self_st_obj);

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
            cerr << " (0x" << fmt_x(current_pc) << "):";
            PmEventLoopReached = debug_print_current_procedure_info(stderr,self_st_obj,current_pc);
            cerr << endl;
            if (PmEventLoopReached == true) break;
        } 
    }
    fflush(stderr);
}

void accvio_handler(int arg)
{
    static int has_executed_before = 0;
    if( has_executed_before ) 
        return;
    else
        has_executed_before = 1;

    cerr << "A memory access violation (bus error or segmentation fault)"
         << "has occurred." << endl
         << "Please submit a problem report." << endl;
    DEBUG_DumpStackTrace(1);
    exit(-1);
}

void InstallSignalHandlers()
{
    signal(SIGBUS, accvio_handler);
    signal(SIGSEGV, accvio_handler);    
}

/*
 * Initialization
 */
extern void StackDumpInit(char * filename)
{
    st_bool_t self_st_obj_stripped;

    InstallSignalHandlers();
    /* 
     * Open the 'self' object.  For C++, name demangling is disabled
     * when an object is successfully opened.
     */

    // FIXME: As of 3/28/2000 if name demangling is not disabled we
    //  get a seg fault in libmld when DEBUG_DumpStackTrace is called.
    //  So disabling demangling by using the flag 'ST_MANGLE_NAMES'
    //if( st_obj_open(&self_st_obj, filename, ST_RDONLY|ST_MANGLE_NAMES) )
    //if( st_obj_open(&self_st_obj, filename, ST_RDONLY) )
    if( st_obj_open(&self_st_obj, filename, ST_MANGLE_NAMES) )
    {
        self_st_obj = NULL;
        return;
    }

    st_is_obj_stripped(self_st_obj, &self_st_obj_stripped);
    if (self_st_obj_stripped) 
    {
        self_st_obj = NULL;
        return;
    } 
}
#else

extern void StackDumpInit(char * filename)
{
    return;
}

#endif
