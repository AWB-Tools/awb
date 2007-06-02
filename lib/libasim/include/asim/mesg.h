/*
 * Copyright (C) 2003-2006 Intel Corporation
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
 * @author David Goodwin, Shubu Mukherjee, Artur Klauser
 * @brief Printing of warnings, error messages, and assertions.
 */

#ifndef ASIM_MESG_H
#define ASIM_MESG_H

// generic
#include <iostream>
#include <fstream>

#include <pthread.h>

// Un-comment the following line in case you want thread-safe ASIM_MESG
//#define THREAD_SAFE_ASIM_MESG

#ifdef THREAD_SAFE_ASIM_MESG
    #define lockMesgMutex(p)   pthread_mutex_lock(p)
    #define unlockMesgMutex(p) pthread_mutex_unlock(p)
#else
    #define lockMesgMutex(p)
    #define unlockMesgMutex(p)
#endif

// ASIM core
#include "asim/syntax.h"
#include "asim/ioformat.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

/*
 * Class ASIM_MESG
 *
 * Class for outputing varargs strings.
 *
 */
typedef class ASIM_MESG_CLASS *ASIM_MESG;
class ASIM_MESG_CLASS 
{
  private:
    // variables
    ostream & out;             ///< output file stream for messages
    const string prefix;       ///< prefix string for each message
    const bool printLocation;  ///< prepend message location file:line
    const bool terminate;      ///< abort program after message
    bool privateStream;        ///< this object owns the out stream

    // constructors / destructors / initializers

    /// Calling the default constructor on this object is meaningless,
    /// so we protect against it by making it private.
    ASIM_MESG_CLASS ();

    /// Return an output stream to use for filename.
    ostream & FilenameToOstream (const string file);

  public:
    // constructors / destructors / initializers

    /// ostream constructor
    ASIM_MESG_CLASS (
        ostream & _out,
        const string & _prefix = "",
        const bool _printLocation = false,
        const bool _terminate = false);
    
    /// filename constructor
    ASIM_MESG_CLASS (
        const string & _name,
        const string & _prefix = "",
        const bool _printLocation = false,
        const bool _terminate = false);
    
    ~ASIM_MESG_CLASS ();
    
    // message print methods

    /// prepare printing of a message
    ostream & Prepare (const char * const file = NULL, const INT32 line = -1);
    /// finish printing of a message
    void Finish(void);
};

// Global ASIM message objects for errors, warnings, and assertions
extern ASIM_MESG_CLASS asim_warn;
extern ASIM_MESG_CLASS asim_error;
extern ASIM_MESG_CLASS asim_assert;

// Global ASIM message mutex for thread safe logging
extern pthread_mutex_t asim_mesg_mutex;

/*
 * Assertion macros. When 'condition' is not true,
 * these print the file and line number and exit.
 * ASSERT is checked only when compiled with ASIM_ENABLE_ASSERTIONS defined.
 * VERIFY is always checked.
 */
#ifdef ASIM_ENABLE_ASSERTIONS

#define WARNX(condition) \
    if (! (condition)) { \
        asim_warn.Prepare(__FILE__,__LINE__); \
        asim_warn.Finish(); \
    }

#define WARN(condition,mesg) \
    if (! (condition)) { \
        asim_warn.Prepare(__FILE__,__LINE__) << mesg << endl; \
        asim_warn.Finish(); \
    }

#define ASSERTX(condition) \
    if (! (condition)) { \
        asim_assert.Prepare(__FILE__,__LINE__); \
        asim_assert.Finish(); \
    }

#define ASSERT(condition,mesg) \
    if (! (condition)) { \
        asim_assert.Prepare(__FILE__,__LINE__) << mesg; \
        asim_assert.Finish(); \
    }

#else // ASIM_ENABLE_ASSERTIONS

#define ASSERTX(condition)
#define ASSERT(condition,mesg)

#endif // ASIM_ENABLE_ASSERTIONS

// same as ASSERT macros but can't be turned off
#define VERIFYX(condition) \
    if (! (condition)) { \
        asim_assert.Prepare(__FILE__,__LINE__); \
        asim_assert.Finish(); \
    }

#define VERIFY(condition,mesg) \
    if (! (condition)) { \
        asim_assert.Prepare(__FILE__,__LINE__) << mesg; \
        asim_assert.Finish(); \
    }

/*
 * Macros for errors and warnings.
 */
#define ASIMERROR(mesg) \
    { \
        asim_error.Prepare(__FILE__,__LINE__) << mesg; \
        asim_error.Finish(); \
    }

#define ASIMWARNING(mesg) \
    { \
        asim_warn.Prepare(__FILE__,__LINE__) << mesg; \
        asim_warn.Finish(); \
    }

#endif /* ASIM_MESG_H */
