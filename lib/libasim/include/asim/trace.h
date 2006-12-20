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

#ifndef TRACE_H
#define TRACE_H

#include <list>
#include <string>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <assert.h>
#include <asim/regexobj.h>

// Include support for the old trace format.
#include <asim/trace_legacy.h>

extern bool     printTraceNames;

// The following TRACE macros should be called from the member function
// of a Traceable object or a decendant of a Traceable object.

//
// The _KEEP macros emit trace messages even in optimized code.  You
// should normally use the versions without the _KEEP suffix, defined
// below.
//

#define WARNING_KEEP(out) \
do { \
    std::ostringstream __traceBuf; \
    __traceBuf << out; \
    (this)->Trace(__traceBuf); \
} while(0)

#define T1_KEEP(out) \
if((this)->traceOnArr[0]) { \
    std::ostringstream __traceBuf; \
    __traceBuf << out; \
    (this)->Trace(__traceBuf); \
}

#define T2_KEEP(out) \
if((this)->traceOnArr[1]) { \
    std::ostringstream __traceBuf; \
    __traceBuf << out; \
    (this)->Trace(__traceBuf); \
}

#define T2_COND_KEEP(out) \
if((this)->traceOnArr[1]) { \
    out; \
    fflush(NULL); \
}

#define WARNING_AS_KEEP(this, out) \
do { std::ostringstream __traceBuf; \
    __traceBuf << out; \
    (this)->Trace(__traceBuf); \
} while(0)

#define T1_AS_KEEP(this, out) \
if((this)->traceOnArr[0]) { \
    std::ostringstream __traceBuf; \
    __traceBuf << out; \
    (this)->Trace(__traceBuf); \
}

#define T2_AS_KEEP(this, out) \
if((this)->traceOnArr[1]) { \
    std::ostringstream __traceBuf; \
    __traceBuf << out; \
    (this)->Trace(__traceBuf); \
}

#define T2_AS_COND_KEEP(out) \
if((this)->traceOnArr[1]) { \
    out; \
    fflush(NULL); \
}

#define T1_UNCOND_KEEP(out) T1_AS(unconditionalTraceable, out);

#ifdef ASIM_ENABLE_TRACE

#define WARNING(out) WARNING_KEEP(out)
#define T1(out) T1_KEEP(out)
#define T2(out) T2_KEEP(out)
#define T2_COND(out) T2_COND_KEEP(out)
#define TRACING(level)          (((level>=1)||(level<=2)) ? (this)->traceOnArr[level-1] : false)
#define WARNING_AS(this, out) WARNING_AS_KEEP(this, out)
#define T1_AS(this, out) T1_AS_KEEP(this, out)
#define T2_AS(this, out) T2_AS_KEEP(this, out)
#define T2_AS_COND(out) T2_AS_COND_KEEP(out)
#define TRACING_AS(this,level)	(((level>=1)||(level<=2)) ? (this)->traceOnArr[level-1] : false)
#define T1_UNCOND(out) T1_UNCOND_KEEP(out)
#define BUILT_WITH_TRACE_FLAGS true

#else /* ASIM_ENABLE_TRACE */

#define WARNING(out)
#define T1(out) 
#define T2(out) 
#define T2_COND(out)
#define TRACING(level) false
#define WARNING_AS(this, out)
#define T1_AS(this, out) 
#define T2_AS(this, out) 
#define T2_COND(out)
#define T1_UNCOND(out) 
#define TRACING_AS(this, level) false
#define BUILT_WITH_TRACE_FLAGS false
 

#endif /* ASIM_ENABLE_TRACE */

// Store a regular expression and a trace level so that
// we can reapply it later.
struct TRACEABLE_REGEX_CLASS;
typedef TRACEABLE_REGEX_CLASS *TRACEABLE_REGEX;
struct TRACEABLE_REGEX_CLASS
{
    Regex *regex;
    int level;
    TRACEABLE_REGEX_CLASS(Regex *r, int l) : regex(r), level(l) 
    {
    }
    ~TRACEABLE_REGEX_CLASS() 
    {
        delete regex;
    }
};
typedef list<TRACEABLE_REGEX> TRACEABLE_REGEX_LIST;

// Support the functionality of applying some number of
// regular expression to enable traces, packaging them
// all up, turning off the traces enabled by those regular
// expressions, and applying them at a later time. This 
// allows the user to turn on the desired traceable objects
// in the normal way and then decide to delay them all 
// until a later time.
struct TRACEABLE_DELAYED_ACTION_CLASS;
typedef TRACEABLE_DELAYED_ACTION_CLASS *TRACEABLE_DELAYED_ACTION;
struct TRACEABLE_DELAYED_ACTION_CLASS
{
    // The regular expressions to apply at a later time.
    TRACEABLE_REGEX_LIST *savedRegexes;
    
    // Create a new traceable delayed action. This will
    // turn off any enabled traces and save them to be
    // applied at a later time.
    TRACEABLE_DELAYED_ACTION_CLASS();

    // Create a delayed action out of a single regular
    // expression and level.
    TRACEABLE_DELAYED_ACTION_CLASS(string regex, int level);

    ~TRACEABLE_DELAYED_ACTION_CLASS() 
    {
        if(savedRegexes) 
        {
            delete(savedRegexes);
        }
    }
    
    // Apply the regexes in this action.
    void go();
};

class TRACEABLE_CLASS;
typedef TRACEABLE_CLASS *TRACEABLE;
class TRACEABLE_CLASS 
{
    friend struct TRACEABLE_DELAYED_ACTION_CLASS;
    
  protected:
    // The instance name of the traceable object. For modules, this is
    // the fully qualified name of the object in the instance hierarchy.
    std::string objectName;
    // Used by the TRACE macro to decide whether to call the trace 
    // member function.
    bool myTraceOn;
    // Controls the verbosity of tracing.
    // 0 = no tracing, same as myTraceOn == false
    // 1 = high level information, of interest to other people
    // 2 = detailed information for debugging
    int traceLevel;
    
    // The maximum traceLevel allowed.
    static const int maxTraceLevel = 2;

    // Keep a list of all traceable objects.
    static std::list<TRACEABLE> *traceables;
    static pthread_mutex_t traceablesMutex;

    // An iterator pointing to the location of
    // this traceable object in the traceables list.
    std::list<TRACEABLE>::iterator myTraceablesEntry;

    // The regular expressions in effect.
    static TRACEABLE_REGEX_LIST *regexes;
    static pthread_mutex_t regexesMutex;

    // Compute the value of traceOnArr.
    void RecomputeTraceArr();

    // Apply a regex to a trace object.
    void EnableByRegex(Regex *regex, int level);

    // Setting this to false will disable the thread safety features
    // of traceable.
    static bool enableThreadProtection;

  public:
    // Derived from myTraceOn and traceLevel, provides fast lookup
    // to see if traces are on for a particular level. This should
    // not be modified directly, only through calls that set myTraceOn
    // and traceLevel.
    bool traceOnArr[maxTraceLevel];

  protected:
    /// Copy constructor. 
    TRACEABLE_CLASS(const TRACEABLE_CLASS& t);

  public:
    TRACEABLE_CLASS();
    ~TRACEABLE_CLASS();

    // Set the trace name of this object. This name is used to control
    // whether traces are on or off during runtime. This should be
    // called by the inheritting class once it's name is known.
    void SetTraceableName(std::string name);

    // By disabling thread protection there may be a performance
    // increase. Thread protection should be disabled in non-
    // multithreaded programs only.
    static void SetThreadProtection(bool enabled) {
        enableThreadProtection = enabled;
    }

    // Set the trace level of any traceable object whose objectName
    // matches regex. For more information on the format of regex,
    // see man 3 regcomp and look under REG_EXTENDED.
    //
    // Returns true if successful, false if there was an error.
    static bool EnableTraceByRegex(TRACEABLE_DELAYED_ACTION regex);
    static bool EnableTraceByRegex(TRACEABLE_REGEX regex);
    static bool EnableTraceByRegex(std::string regex, int level);
protected:
    static bool EnableTraceByRegex(std::string regex, int level, bool saveRegex);
    static bool EnableTraceByRegex(Regex *regex, int level, bool saveRegex);
public:

    // Print the names of the traceable objects.
    static void PrintNames();

    // Set the trace object.
    void SetObjectName(std::string _n);

    // Set the trace level. High numbers provide more traces.
    void SetTraceLevel(int level);
    int GetTraceLevel(void) const { return traceLevel; };

    // Set the myTraceOn variable to enable or disable all traces from
    // this class.
    void SetTraceOn(bool enabled);
    bool GetTraceOn(void) const { return myTraceOn; };

    // Safely send out to an output stream. Depending on usage,
    // this may have to be virtual, which would incur some additional
    // (but small) runtime cost.
    void Trace(std::ostringstream &out) const;

};

inline void TRACEABLE_CLASS::RecomputeTraceArr() 
{
    for(int i = 0; i < maxTraceLevel; i++) 
    {
        traceOnArr[i] = myTraceOn && (traceLevel >= (i + 1));
    }
}

inline void TRACEABLE_CLASS::SetObjectName(std::string _n) 
{
    objectName = _n;
}

inline void TRACEABLE_CLASS::SetTraceLevel(int level) 
{
    assert(level <= maxTraceLevel);
    traceLevel = level;
    RecomputeTraceArr();
}

inline void TRACEABLE_CLASS::SetTraceOn(bool enabled) 
{
    myTraceOn = enabled;
    RecomputeTraceArr();
}

inline void TRACEABLE_CLASS::Trace(std::ostringstream &out) const
{
#if NUM_PTHREADS > 1
    get_thread_safe_log().ts() << std::dec << pthread_self() << ": " <<  out.str() << endl;
#else
    std::cout << out.str() << std::endl;
#endif
}

inline bool TRACEABLE_CLASS::EnableTraceByRegex(std::string regex, int level)
{
    return(EnableTraceByRegex(regex, level, true));
}

inline bool TRACEABLE_CLASS::EnableTraceByRegex(TRACEABLE_REGEX r)
{
    return(EnableTraceByRegex(r->regex, r->level, true));
}

// Make a class that will be used for unconditional trace calls.
class UNCONDITIONAL_TRACEABLE_CLASS : public TRACEABLE_CLASS 
{
  public:
    UNCONDITIONAL_TRACEABLE_CLASS();
};
typedef UNCONDITIONAL_TRACEABLE_CLASS *UNCONDITIONAL_TRACEABLE;

extern UNCONDITIONAL_TRACEABLE unconditionalTraceable;

#endif
