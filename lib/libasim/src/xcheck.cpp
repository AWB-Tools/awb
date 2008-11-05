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
 * @author Roger Espasa
 * @brief Cross-check two live simulator runs with each other (for debugging).
 *
 * Cross checking serves to compare two executions of two different
 * ASIM simulators.  Typically, you use cross-checking when you made a
 * change to the code that you thought it had no effects on
 * performance and, suddenly, the output stats change a little. If you
 * have no clue why, one thing you can do is run two versions of ASIM:
 * the gold version (producing the correct stats) and your modified
 * version (producing ``wrong'' stats). You activate xchecking in both
 * copies of the code and then both copies generate a limited trace
 * output. Using the $ASIMDIR/tools/xcheck script you can compare
 * the two trace outputs as they are generated and flag the first
 * difference that occurs in the two partial-trace outputs.
 *
 * How to use Cross-checking:
 *
 * -#. Insert in some point of the pipeline a call (or multiple calls)
 *    to routine CROSSCHECK().  This routine takes an iostream style
 *    output line. The line can be anything you want up to a certain
 *    limit of bytes (as defined by XCHECK_MAX_STRING).
 *
 * -#. Execute version A of the simulator with the flag -xcheck
 *    <filenameA>. The 'filename' can be anything that lives in a
 *    directory where you have enough permissions to create a named pipe.
 *    The file need not exist before you invoke the simulator.
 *
 * -#. Execute version B of the simulator with the flag -xcheck
 *    <filenameB>. The 'filename' can be anything that lives in a
 *    directory where you have enough permissions to create a named pipe.
 *    The file need not exist before you invoke the simulator.
 *
 * -#. Start the $ASIMDIR/tools/xcheck with the following arguments:
 *      $ASIMDIR/tools/xcheck filenameA filenameB
 *
 * Let the thing run until program 'xcheck' flags an error.
 *
 * Customer comments: (r2r) Awesome tool. Roger rules! :)
 */

// generic
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <iostream>

#if defined(HOST_UNIX)
#include <sys/mode.h>
#endif

//
// The following define is needed on Tru64 4.0F to get the prototype of 'mknod'
// We then need to turn it of to avoid a conflict on the definition of 'MAX'
//
#define _XOPEN_SOURCE_EXTENDED 1
#include <sys/stat.h>
#undef _XOPEN_SOURCE_EXTENDED

// ASIM core
#include "asim/mesg.h"
#include "asim/xcheck.h"

using namespace std;

/// Number of "lines" we can hold in the xchecking buffer
#define XCHECK_BUF_SIZE  (XCHECK_MAX_STRING * 1024)


/// Pointer to a malloc'ed buffer holding the 
char	*xcheck_buf;

/// Index into the buffer
UINT32	xcheck_idx;

/// File descriptor for the output pipe
INT32 xcheck_fd;

/// Flag indicating whether xcheck is enabled or not
bool  xcheck_flag = false;

/**
 * Turn on cross checking.
 */
void
ActivateCrossChecking (
    const char * const file)  ///< name to use for named pipe
{
    bool   create = false;
    INT32 attr;
    INT32 ok;
    struct stat buf;

    if ( file == NULL )
    {
        ASIMERROR("ActivateCrossChecking: invalid NULL file\n"); 
    }

    //
    // Check if the file exists and, if so, whether it is a named pipe or not
    //
    ok = stat(file,&buf);
    if ( ok < 0 )
    {
        if ( errno == ENOENT )
        {
            create = true;
        }
        else if ( errno == ENOTDIR || errno == EACCES )
        {
            ASIMERROR("ActivateCrossChecking: error reaching to "
                << file << endl);
        }
        else if ( errno == ESTALE )
        {
            ASIMERROR("ActivateCrossChecking: Stale NFS filehandle for "
                << file << endl);
        }
        else
        {
            ASIMERROR("ActivateCrossChecking: 'stat' failed for "
                << file << endl);
        }
    }
    else
    {
        //
        // The file already exists; let's see if it's a named pipe
        //
        if ( ! S_ISFIFO(buf.st_mode) )
        {
            ASIMERROR("ActivateCrossChecking: file " << file
                << " exists but is not a FIFO" << endl);
        }
    }


    //
    // Create a NAMED PIPE only if necessary using the mknod syscall
    // This file is where we will dump the cross checking information
    //
    if ( create )
    {
        //
        // File attributes: we create the named pipe with permission 660 (or rw-rw----).
        //
        attr = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

        //
        // We or in the flag to make it a named pipe
        //
        attr |= S_IFIFO;

        //
        // do it (last parameter is only needed if 'attr' specifies a block/character device.
        // in our case, it is ignored).
        //
        ok = mknod(file,(mode_t)attr,(dev_t)0);

        if ( ok < 0 )
        {
            ASIMERROR("ActivateCrossChecking: could not create named pipe "
                << file << ": errno is " << errno << endl); 
        }
    }

    //
    // At this point the named pipe must exist. Let's open it and see if we can write to it
    //
    cout << endl << endl
         << "ACTIVATING CROSS CHECKING to file " << file << "...";
    cout << flush;
    xcheck_fd = open(file,O_WRONLY);
    if ( xcheck_fd < 0 )
    {
        ASIMERROR("ActivateCrossChecking: unable to open "
            << file << " for writing: errno is " << errno << endl); 
    }
    cout << "SUCCESS!" << endl << endl;

    //
    // Allocate a buffer to hold the Xchecking info
    //
    xcheck_idx = 0;
    xcheck_buf = (char *) malloc(XCHECK_BUF_SIZE);
    if ( xcheck_buf == NULL )
    {
        ASIMERROR("ActivateCrossChecking: unable to allocate "
            << XCHECK_BUF_SIZE << " bytes for xcheck buffer" << endl);
    }

    //
    // Activate xchecking
    //
    xcheck_flag = true;
}

void
full_write (
    UINT32 fd,     ///< file descriptor to write to
    char *buf,     ///< buffer that needs be written
    UINT32 nbytes) ///< number of bytes to write
{
    INT32 ok;
    char *ptr = buf;

    while ( nbytes > 0 )
    {
        ok = write(fd,ptr,nbytes);
        if ( ok < 0 ) {
            ASIMERROR("full_write: errno " << errno << endl);
        }
        nbytes -= ok;
        ptr += ok;
    }
}

/**
 * Store one line of cross check output into the internal buffer.
 * The whole contents of the buffer is output if it is nearly full.
 */
void
CrossCheck (
    const string & line) ///< output string for one crosscheck line
{
    if ( ! xcheck_flag )
    {
        return;
    }

    INT32 len = line.length();
    ASSERTX(len <= XCHECK_MAX_STRING);

    strncpy(&(xcheck_buf[xcheck_idx]), line.c_str(), len);
    
    //
    // Increase index and see if we have to dump the buffer out.
    //
    xcheck_idx += len;

    if ( xcheck_idx >= (XCHECK_BUF_SIZE - XCHECK_MAX_STRING))
    {
        //
        // Do a write onto the output named pipe. Do it with a loop
        // to add some robustness to file writes across NFS and other
        // spurious errors that might occur
        //
        full_write(xcheck_fd, xcheck_buf, xcheck_idx);

        xcheck_idx = 0;
    }
}
