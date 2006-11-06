/*
 * Copyright (C) 2004-2006 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @author Shubu Mukherjee
 * @brief ASIM's tracing interface - performance model debugging.
 *
 * @note This file contains Arana-isms (Trace_*box) which should not
 * live here in the core.
 */

#ifndef _TRACE_
#define _TRACE_

// generic
#include <stdio.h>
#include <ostream>
#include <sys/time.h>

// ASIM core
#include "asim/ioformat.h"
#include "asim/syntax.h"
#include "asim/message_handler_log.h"
#include "asim/threaded_log.h"



extern bool	debugOn;
extern bool     warningsOn;

extern bool     traceOn; 
extern UINT32 	traceMask;

void print_trace_masks(std::ostream& o);


extern long trace_start_usecs;
inline long  getStartTime(){ 
    struct timeval tv; 
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);
}
#define TTIME_INIT  long trace_start_usecs


#ifdef ASIM_ENABLE_TRACE

#define TRACEP(a) (traceOn && (traceMask & (a)))

#define TMSG(a, b) \
do { \
    if (traceOn && (traceMask & (a))) \
    { \
	cout << b << endl; \
    } \
} while(0)



#define TRACE(a, b) \
do { \
    if (traceOn && (traceMask & (a))) \
    { \
	b; \
	fflush(NULL); \
    } \
} while(0)

#define TRACE2(a, b, c) \
do { \
    if (traceOn && (traceMask & (a))) \
    { \
	b; \
	c; \
	fflush(NULL); \
    } \
} while(0)


      
/* These TT* macros are for a thread-safe tracer.  If PTHREADS mode is
   off, they'll just use cout.

   TTRACE: unconditional tracing, implied endl
   TTMSG:  conditional tracing, implied endl

*/
#if NUM_PTHREADS > 1
#define TTRACE(b) \
do { \
    get_thread_safe_log().ts() << std::dec << pthread_self() << ": " <<  b << endl; \
} while(0)

#define TTMSG(a, b) \
do { \
    if (traceOn && (traceMask & (a))) \
    { \
	get_thread_safe_log().ts() << std::dec << pthread_self() << ": "  << b << endl; \
    } \
} while(0)

#define TTIME(a)  do { \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
    get_thread_safe_log().ts() << "Time: " << std::dec \
                               <<  (tv.tv_sec*1000000 + tv.tv_usec) - trace_start_usecs \
                               << "\t" << a << endl; \
}while(0)

#else

#define TTRACE(b) do { cout << b << endl; } while(0)

#define TTMSG(a, b)  do { \
if (traceOn && (traceMask & (a))) \
{ cout << b << endl; } \
} while(0) 


#define TTIME(a)  do { \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
    cout << "Time: " << std::dec \
                               <<  (tv.tv_sec*1000000 + tv.tv_usec) - trace_start_usecs \
                               << "\t" << a << endl; \
}while(0)


#endif




#else

#define TRACEP(a) 0

/* FIXME:  these should use a "do {} while(0)" to keep the semicolon from messing things up. */
#define TRACE(a, b)     
#define TRACE2(a, b, c)
#define TMSG(a, b)
#define TTMSG(a,b)
#define TTRACE(a)
#define TTIME(a)
#endif



#define TRACE_MASK(i)	(1 << (i))

/* Convert a comma separated list of box names into a composite trace
 * action number from the defines below. */
UINT32 find_trace_opt(char* str);

#define Trace_All	 0xffffffff	
#define Trace_AllbutBP	 0xdfffffff	// Dont trace the Branch Predictor

#define Trace_Ibox	 TRACE_MASK(0)	// 0x00000001, 1
#define Trace_Fetch	 Trace_Ibox
#define Trace_Pbox	 TRACE_MASK(1)	// 0x00000002, 2
#define Trace_Expand     Trace_Pbox
#define Trace_Decode     Trace_Pbox
#define Trace_Qbox	 TRACE_MASK(2)	// 0x00000004, 4
#define Trace_Issue      Trace_Qbox
#define Trace_Ebox	 TRACE_MASK(3)	// 0x00000008, 8
#define Trace_Execute    Trace_Ebox	 
#define Trace_Mbox	 TRACE_MASK(4)	// 0x00000010, 16
#define Trace_Cbox	 TRACE_MASK(5)	// 0x00000020, 32
#define Trace_Xbox	 TRACE_MASK(6)	// 0x00000040, 64
#define Trace_Cmd	 TRACE_MASK(7)  // 0x00000080, 128
#define Trace_Sys	 TRACE_MASK(8)	// 0x00000100, 256

#define Trace_Vbox	 TRACE_MASK(10)	// 0x00000400, 1024
#define Trace_Inst      TRACE_MASK(11) // 0x00000800, 2048
#define Trace_EapOracle TRACE_MASK(12) // 0x00001000, 4096
#define Trace_Ubox      TRACE_MASK(13) // 0x00002000, 8192
#define Trace_Ring      TRACE_MASK(14) // 0x00004000, 16384
#define Trace_Zbox      TRACE_MASK(15) // 0x00008000, 32768
#define Trace_Ports     TRACE_MASK(16) // 0x00010000, 65536
#define Trace_Czu	 TRACE_MASK(17) // 0x00020000, 131072
#define Trace_Rambus	 TRACE_MASK(18)	// 0x00040000, 262334
#define Trace_Rbox	 TRACE_MASK(19)	// 0x00080000, 524668
#define Trace_Slice	 TRACE_MASK(20)	// 0x00100000, 2^20
#define Trace_Context	 TRACE_MASK(21)	// 0x00200000, 2^21
#define Trace_Mtsched   TRACE_MASK(22) // 0x00400000, 2^22
#define Trace_Micro_Feeder  TRACE_MASK(23) // 0x00400000, 2^23
#define Trace_Macro_Feeder  TRACE_MASK(24) // 0x00400000, 2^24

/* Trace_Feeder captures all the Trace_Feeder existing usages,
   plus the refinements for the two-level feeders used in x86 */
#define Trace_Feeder	 (TRACE_MASK(9)|Trace_Micro_Feeder|Trace_Macro_Feeder)

#define Trace_Retire     Trace_Rbox
#define Trace_BP	 TRACE_MASK(29) // 0x20000000, 2^29
#define Trace_Debug	 TRACE_MASK(30)	// 0x40000000, 2^30


namespace IoFormat 
{
    // formatting classes for printing node and slice numbers in trace stmts
    extern const class Format fmt_node;
    extern const class Format fmt_slice;
    extern const class Format fmt_cpu;
    extern const class Format fmt_chip;
}

#endif
