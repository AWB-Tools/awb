/*
 * *****************************************************************
 * *                                                               *
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
 * @author Joel Emer
 * @brief ASIM Main - just include and use everything from base for testing
 */

// Generic
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

// ASIM core
//
//  We include all possible includes from main that do not have an explicit
//  dependency on any provided modules
//


#include "asim/syntax.h"

// Includes of all .h files in base/ for compilation testing

//#include "asim/address.h"         // broken - obsolte?
#include "asim/agequeue.h"
#include "asim/alphaops.h"
#include "asim/arraylist.h"
//#include "asim/buffer.h"         // obsolte
#include "asim/cache.h"
//#include "asim/cache_mesi.h"     // broken
#include "asim/chip_component.h"
#include "asim/chunkedqueue.h"
#include "asim/chunk.h"
//#include "asim/cmd.h"            // obsolete
#include "asim/config.h"
#include "asim/damqueue.h"
#include "asim/deque.h"
#include "asim/disasm.h"
#include "asim/dynhashtable.h"
#include "asim/except.h"
#include "asim/fastlist.h"
#include "asim/fifo.h"
#include "asim/funnel.h"
#include "asim/hashtable.h"
#include "asim/ioformat.h"
#include "asim/mesg.h"
#include "asim/mm.h"
#include "asim/mmptr.h"
#include "asim/module.h"
#include "asim/orderedDAMQueue.h"
#include "asim/param.h"
#include "asim/port.h"
#include "asim/profile.h"
//#include "asim/pthd.h"          // obsolete
#include "asim/queue.h"
#include "asim/registry.h"
#include "asim/resource_stats.h"
#include "asim/sized_stl_queue.h"
#include "asim/stackdump.h"
#include "asim/stack.h"
#include "asim/state.h"
#include "asim/stateout.h"
//#include "asim/storage.h"        // ???
#include "asim/stripchart.h"
//#include "asim/thread.h"         // To be replaced 
#include "asim/trace.h"
#include "asim/trackmem.h"
#include "asim/traps.h"
#include "asim/utils.h"
#include "asim/vector.h"
#include "asim/xcheck.h"
#include "asim/xmlout.h"


void PrintInfo();

int
main (INT32 argc, char *argv[], char *envp[])
/*
 * Main program for standalone version of asim.
 */
{
    // shared buffers between C-style stdio and C++-style streamio
    cin.sync_with_stdio();
    cout.sync_with_stdio();
    cerr.sync_with_stdio();

    PrintInfo();


    // Tests of the varous base/ facilties should be included here...

    exit(0);

    return(0);
}


void
PrintInfo()
{
    cout << "ASIM, Copyright (c) 1999 - 2006, Intel Corporation" << endl
         << "Developed by VSSAD, Maintained and Enhanced by the AMI Group" << endl << endl;
}
