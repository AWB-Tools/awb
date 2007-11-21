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
 * @author David Goodwin, Shubu Mukherjee, Artur Klauser
 * @brief Printing of warnings, error messages, and assertions.
 */

// generic
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/smp.h"

using namespace std;

/* global_cycle is hack for messages so that
    we know when the asserts fire. No code should
    use this! other than the system */
UINT64 global_cycle = 0 ;

static const bool print_location=true;
static const bool doexit=true;
// instantiate some message types
//
//                          output stream
//                          |      prefix string
//                          |      |                     
ASIM_MESG_CLASS asim_warn  (cerr, "WARNING: ",           print_location, !doexit);
ASIM_MESG_CLASS asim_error (cout, "ERROR: ",             print_location, doexit);
ASIM_MESG_CLASS asim_assert(cout, "ASSERTION FAILURE: ", print_location, doexit);

pthread_mutex_t asim_mesg_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Create a message object and connect it to an existing ostream.
 */
ASIM_MESG_CLASS::ASIM_MESG_CLASS (
    ostream & _out,            ///< ostream to use for messages
    const string & _prefix,    ///< optional prefix string for each message
    const bool _printLocation, ///< prepend location file:line for messages
    const bool _terminate)     ///< terminate program after a message
    : out(_out),
      prefix(_prefix),
      printLocation(_printLocation),
      terminate(_terminate),
      privateStream(false)
{ }

/**
 * Create a message object and connect it to the file named in the
 * input.
 * @note: FilenameToOstream() in the initializer also initializes
 * privateStream.
 */
ASIM_MESG_CLASS::ASIM_MESG_CLASS (
    const string & _name,      ///< file name to open for message printing
    const string & _prefix,    ///< optional prefix string for each message 
    const bool _printLocation, ///< prepend location file:line for messages
    const bool _terminate)     ///< terminate program after a message
    : out(FilenameToOstream(_name)),
      prefix(_prefix),
      printLocation(_printLocation),
      terminate(_terminate)
{ }

/**
 * Default destructor. If this objects own the output stream, delete
 * that too.
 */
ASIM_MESG_CLASS::~ASIM_MESG_CLASS () 
{
    if (privateStream)
    {
        delete &out;
    }
}

/**
 * Prepare the output stream for printing a message and return the
 * output stream that should be used for printing.
 *
 * @note: Leave this function in the .cpp file! This allows the debugger
 * to set more reliable breakpoint for ASSERT statements.
 */
ostream &
ASIM_MESG_CLASS::Prepare (
        const char * const file, ///< file name of message
        const INT32 line)        ///< line number of message
{
    lockMesgMutex(&asim_mesg_mutex);
    
    fflush(NULL); // flush all output FILEs
    out << flush; // flush this output stream
    out << prefix; // print prefix string
    if (printLocation && file)
    {
        out << dec
            << file << ":" << line
            << " Cycle:" << global_cycle
            << endl << flush;
    }
    
    unlockMesgMutex(&asim_mesg_mutex);
    
    return out;
}


/**
 * Finish printing of a message. We performa the final output flushes
 * and conditionally terminate the program.
 */
void
ASIM_MESG_CLASS::Finish(void)
{
    out << flush;
        
    if (terminate)
    {
        out << endl << flush;
        if (ASIM_SMP_CLASS::GetRunningThreadNumber() > 0)
        {
            // Thread attempting to exit is a child thread.  exit() may
            // not work cleanly -- just give up.  We could do better by
            // passing a message to the main thread to force an exit.
            _exit(1);
        }
        else
        {
            // Cleaner exit from the main thread.
            exit(1);
        }
    }
}


/**
 * Return an output stream for filename and initialize privateStream
 * state. Filenames "stdout", "stderr", "cout", and "cerr" have their
 * predetermined obvious meaning.
 */
ostream &
ASIM_MESG_CLASS::FilenameToOstream (
    const string file)  ///< filename to use for output
{
    privateStream = false;
    if (file == "stdout" || file == "cout")
    {
        return cout;
    }
    else if (file == "stderr" || file == "cerr")
    {
        return cerr;
    }
    else
    {
        // we own our output stream and have to dispose of it eventually
        privateStream = true;
        ofstream * ofs = new ofstream(file.c_str());
        if (ofs == NULL || ofs->fail())
        {
            // we can't use ASIM's error message handling here, since
            // we are right in the middle of setting it up!
            cerr << "Unable to open message file " << file << endl;
            Finish();
        }

        return *ofs;
    }
}
