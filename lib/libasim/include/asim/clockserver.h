/*
 * **********************************************************************
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
 * @author Santi Galan, Ken Barr, Ramon Matas Navarro
 **/


// ************************************************************************
// WARNING!!! (FOR PHASED CLOCKING USERS ONLY)
// ************************************************************************
// The usual behaviour of the clockserver's Clock method is to carry out one
// phase of work for one and only one domain+skew before returning.
//
// Anyway, I have implemented an optimization that is applied when the model
// creates only one clock domain. It is activated by default and, as a
// backside effect, the usual behaviour of Clock method is changed: it performs
// a complete cycle of work (high and low phases) before returning.
//
// This may cause problems to some hybrid systems and for this reason the
// optimization can be deactivated using the "SetUniqueDomainOptimization"
// method.
//
// (Ramon Matas Navarro)
// ************************************************************************


#ifndef _CLOCK_SERVER_
#define _CLOCK_SERVER_

// C++/STL
#include <list>
#include <vector>
#include <deque>

// Base
#include <iostream>
#include <pthread.h>

// ASIM core
#include "asim/stateout.h"
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/trace.h"
#include "asim/atomic.h"
#include "asim/smp.h"
#include "asim/event.h"
#include "asim/phase.h"
#include "asim/dynamic_array.h"

using namespace std;


// Forward declaration of the CLOCKABLE classes
typedef class ASIM_CLOCKABLE_CLASS *ASIM_CLOCKABLE;
typedef class ClockRegistry CLOCK_REGISTRY_CLASS, *CLOCK_REGISTRY;
typedef class RateMatcher RATE_MATCHER_CLASS, *RATE_MATCHER;

/**
 * Callback interface and template
 **/
 
typedef class ClockCallBackInterface
{    

  public:

    UINT64 currentCycle;
    
    CLOCK_REGISTRY cReg;
    
    virtual void Clock() = 0;
    virtual CLK_EDGE getClkEdge() = 0;
    virtual void setClkEdge(CLK_EDGE ed) = 0;
    virtual bool withPhases() = 0;
    
    virtual ~ClockCallBackInterface() { };

    void setClockRegistry(CLOCK_REGISTRY _cReg)
    {
        cReg = _cReg;
    }

  protected:

    #ifdef HOST_LINUX_X86

    /** Profile support methods */
    UINT32 nCycleBeforeHigh;
    UINT32 nCycleBeforeLow;

    /* 
     * Set *hi and *lo to the high and low order bits of the cycle counter.
     * Implementation requires assembly code to use the rdtsc instruction.  
     */
    inline void accessCounter(UINT32 *hi, UINT32 *lo)
    {
         asm volatile("rdtsc; movl %%edx, %0; movl %%eax, %1" /* read cycle counter */
                      : "=r" (*hi), "=r" (*lo)                /* and move results to */
                      : /* No input */                        /* the two outputs */
                      : "%edx", "%eax");
    }

    /* Record the current value of the cycle counter. */
    inline void startCounter()
    {
         accessCounter(&nCycleBeforeHigh, &nCycleBeforeLow);
    }

    /* 
     * Computes the number of cycles since the last call to start_counter. 
     * @return If the result has wrapped around
     */
    inline bool getCounter(UINT64 &result)
    {
         UINT32 nCycleAfterHigh, nCycleAfterLow;
         UINT32 hi, lo, borrow;
    
         /* get cycle counter */
         accessCounter(&nCycleAfterHigh, &nCycleAfterLow);   
         //cerr << "nCycleBeforeHigh = " << nCycleBeforeHigh <<
         //     " nCycleBeforeLow = " << nCycleBeforeLow <<
         //     " nCycleAfterHigh = " << nCycleAfterHigh <<
         //     " nCycleAfterLow = " << nCycleAfterLow << endl;
         /*  double precision subtraction */
         lo = nCycleAfterLow - nCycleBeforeLow;
         borrow = lo > nCycleAfterLow;
         hi = nCycleAfterHigh - nCycleBeforeHigh - borrow;
         result = static_cast<UINT64>(hi * (1 << 30) * 4 + lo);
         return false;
    }

    #elif HOST_DUNIX

    UINT64 nCycleBefore;
    inline void accessCounter(UINT64 *cycle)
    {
        asm volatile("rpcc $0; sll $0, 32, $1; addq $0, $1, $0; srl $0, 32, %0;"
                     : "=r"(*cycle));
    }

    inline void startCounter()
    {
         accessCounter(&nCycleBefore);
    }

    inline bool getCounter(UINT64 &result)
    {
        bool bWrapped = false;
        UINT64 nCycleAfter = 0;
        asm volatile("rpcc $0; sll $0, 32, $1; addq $0, $1, $0; srl $0, 32, %0;"
                     : "=r"(nCycleAfter));
        if(nCycleAfter < nCycleBefore)
        {
            result = (0xFFFFFFFF - nCycleBefore) + nCycleAfter;
            bWrapped = true;
        }
        else
        {
            result = nCycleAfter - nCycleBefore + 1;
        }
        return bWrapped;
    }
    
    #endif
   
} CLOCK_CALLBACK_INTERFACE_CLASS, *CLOCK_CALLBACK_INTERFACE;

template <class Class>
class ClockCallBack: public ClockCallBackInterface
{

  public:
  
    typedef void (Class::*Method)(UINT64);
    
  private:

    Class*  class_instance;
    Method  method;
    
  public:      
  
    ClockCallBack(Class* _class_instance, Method _method)
    {
        class_instance = _class_instance;
        method = _method;
    };
    virtual ~ClockCallBack() {};

    bool withPhases()
    {
        return false;
    }
    
    void setClkEdge(CLK_EDGE ed)
    {
        ASSERT(false, "Not defined in this callback!");
    }    
    
    CLK_EDGE getClkEdge()
    {
        return HIGH;
    }    

    void Clock()
    {           
        #ifdef ASIM_ENABLE_PROFILE
        #if defined(HOST_DUNIX) | defined(HOST_LINUX_X86)
        startCounter();
        #endif
        #endif
                  
        (class_instance->*method)(currentCycle);        

        #ifdef ASIM_ENABLE_PROFILE
        #if defined(HOST_DUNIX) | defined(HOST_LINUX_X86)
        UINT64 result = 0;
        if(getCounter(result)) // Wrapped 
        {
            class_instance->IncWrapAround(result);
        }
        class_instance->IncCyclesSpent(result);
        #endif
        #endif
    };

};

template <class Class>
class ClockCallBackPhase: public ClockCallBackInterface
{

  public:
  
    typedef void (Class::*Method_A)(UINT64, CLK_EDGE);
    typedef void (Class::*Method_B)(PHASE);
    
  private:

    Class*  class_instance;
    Method_A  method_a;
    Method_B  method_b;
    
    bool type;
    
    CLK_EDGE edge;
    
  public:      
  
    ClockCallBackPhase(Class* _class_instance, Method_A _method,
                       CLK_EDGE _edge = HIGH)
    {
        class_instance = _class_instance;
        method_a = _method;
        edge = _edge;
        type = true;
    };
    
    ClockCallBackPhase(Class* _class_instance, Method_B _method,
                       CLK_EDGE _edge = HIGH)
    {
        class_instance = _class_instance;
        method_b = _method;
        edge = _edge;
        type = false;
    };
    
    virtual ~ClockCallBackPhase() { };

    bool withPhases()
    {
        return true;
    }
    
    CLK_EDGE getClkEdge()
    {
        return edge;
    }

    void setClkEdge(CLK_EDGE ed)
    {
        edge = ed;
    }        
    
    void Clock()
    {
        #ifdef ASIM_ENABLE_PROFILE
        #if defined(HOST_DUNIX) | defined(HOST_LINUX_X86)
        startCounter();
        #endif
        #endif
        
        if(type)
        {
            (class_instance->*method_a)(currentCycle, edge);
        }
        else
        {
            PHASE ph(currentCycle, edge);
            (class_instance->*method_b)(ph);
        }

        #ifdef ASIM_ENABLE_PROFILE
        #if defined(HOST_DUNIX) | defined(HOST_LINUX_X86)
        UINT64 result = 0;
        if(getCounter(result)) // Wrapped 
        {
            class_instance->IncWrapAround(result);
        }
        class_instance->IncCyclesSpent(result);
        #endif
        #endif
    };

};


typedef ClockCallBack<ASIM_CLOCKABLE_CLASS> ClockCallBackClockable;


/**
 * Clockserver thread class.
 *
 * WARNING: don't remove the volatile qualifiers, they are important
 * for thread synchronization !!
 **/
class ASIM_CLOCKSERVER_THREAD_CLASS
{
    
  private:
    ASIM_SMP_THREAD_HANDLE tHandle;
    
    // Current list of work
    list<CLOCK_CALLBACK_INTERFACE> tasks;
    
    // Pthread associated with this class
    pthread_t thread;
    pthread_attr_t thread_attr;
        
    // Code executed by the pthread 
    static void * ThreadWork(void* param);
  
    volatile bool threadActive;
    volatile bool threadForceExit;
    
  public:
    ASIM_CLOCKSERVER_THREAD_CLASS(ASIM_SMP_THREAD_HANDLE tHandle) :
        tHandle(tHandle),
        threadActive(false),
        threadForceExit(false),
        tasks_completed(true)
    {};
    
    ~ASIM_CLOCKSERVER_THREAD_CLASS()
    {
        VERIFY(!ThreadActive(), "Thread " << GetThreadId() << " not stopped!");
    }
    
    inline ASIM_SMP_THREAD_HANDLE GetAsimThreadHandle() const
    {
        return tHandle;
    }

    /** Returns the unique identifier of this thread */
    inline UINT32 GetThreadId()
    {
        return GetAsimThreadHandle()->GetThreadId();
    }
    
    /** Returns true if the pthread is created */
    inline bool ThreadActive()
    {
        return threadActive;
    }
    
    /** Method to add a new task to be executed by the pthread */
    inline void AssignTask(CLOCK_CALLBACK_INTERFACE task)
    {
        // WARNING! (only for multithreaded clocking)
        // It is not necessary to get the lock since we are using a
        // cycle-by-cycle barrier. The master thread waits for the workers to
        // clock the modules and tasks are assigned before signaling them.

        tasks.push_back(task);
    }
    
    /** Returns true if all the assigned tasks have been completed */
    inline bool CheckAllTasksCompleted()
    {
        ASSERTX(ThreadActive());

        // Wait for the thread to complete the tasks
        pthread_mutex_lock(&finish_cond_mutex);
        while(!tasks_completed)
        {
            pthread_cond_wait(&finish_condition, &finish_cond_mutex);
        }
        pthread_mutex_unlock(&finish_cond_mutex);
        
        return tasks_completed;
    }
    
    /** Tell the thread he can start to execute the current list of tasks */
    void PerformTasksSequential();
    
    inline void PerformTasksThreaded()
    {
        ASSERTX(ThreadActive());

        // Restart the thread waiting for the condition
        pthread_mutex_lock(&task_list_mutex);
        tasks_completed = false;
        pthread_cond_broadcast(&wait_condition);
        pthread_mutex_unlock(&task_list_mutex);
    }

    // Functions to create and destroy the pthread and synchronization devices
    void CreatePthread();
    bool DestroyPthread();

  public:
    
    /* WARNING!!!
    The following methods and structures are public because they may be accessed
    by the pthread. Anyway, they mustn't be accessed externally!! */
    
    // All the tasks in the execution list have been completed
    volatile bool tasks_completed;
  
    // Pthread synchronization devices
    pthread_cond_t wait_condition;
    pthread_cond_t finish_condition;
    pthread_mutex_t task_list_mutex;
    pthread_mutex_t finish_cond_mutex;

    /** Function that returns the next item to be executed */
    /** WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!
        The task_list_mutex must be locked before calling this function.
        The correct sequence to follow in the pthread code in order to
        avoid deadlocks is:
         - Lock task_list_mutex
         - Execute all the items in the list
         - Release the lock
    */
    inline CLOCK_CALLBACK_INTERFACE GetNextWorkItem()
    {
        CLOCK_CALLBACK_INTERFACE cb = NULL;
        if(tasks.size() > 0)
        {
            cb = tasks.front();
            tasks.pop_front();
        }
        
        return cb;
    }
};

typedef ASIM_CLOCKSERVER_THREAD_CLASS* ASIM_CLOCKSERVER_THREAD;


/**
 * Class that represents a clock domain
 * (set of modules that are clocked at the same frequency).
 **/
typedef class ClockDomain
{
    
  public:
    
    /** Unique name of the clock domain */
    const string name;
    
    /** List of possible frequencies */
    vector<UINT32> normalizedFrequencies;
    
    /** Clock registries registered to this domain (in fact,
        the different skews) */
    list<CLOCK_REGISTRY> lClock;
    
    /** Current working frequency */
    UINT32 currentFrequency;
    
    /** Default clocking thread for the modules registered to this domain */
    ASIM_CLOCKSERVER_THREAD defaultThread;
    
    ClockDomain(string _name, ASIM_CLOCKSERVER_THREAD _defaultThread)
        : name(_name),
          defaultThread(_defaultThread)
    { /* Nothing */ }
    
} CLOCK_DOMAIN_CLASS, *CLOCK_DOMAIN;


/**
 * Class that represents a clock event, that implies to clock all
 * contained modules
 **/
class ClockRegistry 
{
    
  public:
    
    /**
     * List that holds the modules registered at
     * this clock characteristics
     **/
    vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> > lModules;

    /**
     * List that holds the write rate matchers
     * registered to this clock characteristics
     **/
    vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> > lWriterRM;

    /** Multiple of base frequency */
    UINT32 nFrequency;

    /** Skew applied to */
    UINT32 nSkew;
    UINT32 dralSkew;

    /** Phase */
    CLK_EDGE edge;
    
    /**
     * Number that says the number of base cycles that must
     * pass before all the modules are clocked.
     **/
    UINT32 nStep;

    /**
     * Says which is the NEXT base cycle that the modules are going
     * to be clocked
     **/
    UINT64 nBaseCycle;

    /** Local cycle counter to this frequency */
    UINT64 nCycle;

    /** Clock domain to which the registry belongs to */
    CLOCK_DOMAIN clockDomain;        

    /**
     * List that holds the modules registered for
     * callback when the events are turned on
     **/
    vector<ASIM_CLOCKABLE> lEventsTurnOn;

    void DralEventsTurnedOn();

    /** Identifier for this clock registry. Used for the DRAL events */
    UINT16 clockId;

    ClockRegistry(CLOCK_DOMAIN _clockDomain, UINT32 skew, UINT32 _dralSkew,
                  CLK_EDGE _edge, bool create_Dral_clk, bool withPhases)
        : nSkew(skew),
          dralSkew(_dralSkew),
          edge(_edge),
          nStep(0),
          nBaseCycle(0),
          nCycle(0),
          clockDomain(_clockDomain)
    {
        nFrequency = clockDomain->currentFrequency;
        EVENT(
            if (runWithEventsOn && create_Dral_clk)
            {
                ostringstream sout;
                sout << clockDomain->name << "_skew" << dralSkew;

                if(withPhases)
                {
                    // TODO: Divisions set to max number of clk_edges by default.
                    // Change it!
                    clockId = DRALEVENT(NewClock(clockDomain->currentFrequency,
                                  dralSkew, NUM_CLK_EDGES, sout.str().c_str()));
                }
                else
                {
                    clockId = DRALEVENT(NewClock(clockDomain->currentFrequency,
                                        dralSkew, 1, sout.str().c_str()));
                }
            }
        );
    }
    
    inline void DralNewCycle()
    {
        EVENT
        (
            if (runWithEventsOn)
            {
                DRALEVENT
                (
                    Cycle(clockId, nCycle, edge)
                );
            }
        );
    }

};


/**
 * Class responsible to send the clock signal to all registered
 * modules. Every module that wants to be clocked register itself
 * to clock server. Each main loop, the clock server clocks every
 * module.
 **/
class ASIM_CLOCK_SERVER_CLASS : public TRACEABLE_CLASS
{
    
  private:
   
    /** Holds all the clock domains */
    list<CLOCK_DOMAIN> lDomain;
  
    /** Clocking thread assigned by default */
    ASIM_CLOCKSERVER_THREAD defaultThread;

    /** Clockserver threads array */
    DYNAMIC_ARRAY_CLASS<ASIM_CLOCKSERVER_THREAD> threads;
    
    /** List of created threads */
    list<ASIM_CLOCKSERVER_THREAD> lThreads;
    
    /** True if only one clock domain have been created */
    bool uniqueClockDomain;
  
    /** False if we want to disable the unique clock domain optimization */
    bool uniqueDomainOptimization;
        
    /** Threaded clocking? */
    bool threaded;
    UINT32 max_pthreads;
        
    /** Clock registry of the reference clock domain */
    CLOCK_REGISTRY referenceClockRegitry;
    CLOCK_DOMAIN referenceClockDomain;
    
    /** Clock registry of the first clock domain created */
    CLOCK_REGISTRY firstClockRegitry;
    bool firstClockRegitrySet;
    
    /** Clock server base frequency */
    UINT64 Bf;
    
    /** Internal base frequency cycle */
    UINT64 internalBaseCycle;

    /** List that holds the timing sequence. */
    deque<CLOCK_REGISTRY> lTimeEvents;

    /** Lists used at init time to connect the rate matchers */
    list<RATE_MATCHER> lrateWriter;
    list<RATE_MATCHER> lrateReader;
    
    /** List of the callbacks created by the clockserver,
        just to delete them at the end */
    list<ClockCallBackClockable*> lCreatedCallbacks;
    
    /** List of mutexs created by the clockserver,
        just to delete them at the end */
    list<pthread_mutex_t*> lCreatedMutexs;
    
    /** Add an Asim thread to the available threads */
    ASIM_CLOCKSERVER_THREAD MapThread(ASIM_SMP_THREAD_HANDLE tHandle);

    void AddTimeEvent(UINT64 time, CLOCK_REGISTRY freq);

    /** Internal method to register the write rate matchers
        at the reader frequency */
    void RegisterWriterRateMatcherClock(RATE_MATCHER wrm, CLOCK_DOMAIN domain,
                                    ASIM_CLOCKSERVER_THREAD thread, UINT32 skew);
    
    /** Method that produces a random clock order within all the modules that
        belongs to a ClockRegistry */
    UINT64 RandomClock();
    
    /** Method that uses pthreads to clock the modules in parallel */
    UINT64 ThreadedClock();
    
    /** Specialized clock method used when there is only one clock domain */
    UINT64 UniqueDomainClock();

    /** Random seed and state used in the RandomClock method */
    #define CLOCKSERVER_RANDOM_STATE_LENGTH 128
    UINT32 random_state[CLOCKSERVER_RANDOM_STATE_LENGTH / 4];
    UINT64 random_seed;

    /** Dump the profile info? */
    bool bDumpProfile;

    /**
     * @param a First operand
     * @param b Second operand
     * @return lcm(a, b)
     **/
    UINT32 lcm(int a,int b)
    {
        UINT32 result = b;

        while(result % a != 0) result += b;
        return result;
    }
   
  public:
  
    ASIM_CLOCK_SERVER_CLASS();
    ~ASIM_CLOCK_SERVER_CLASS();
    
    
    /** Accessors (used by the clockserver system) */
    inline UINT64 getBaseFrequency() { return Bf * 10; }  // In MHz
    
    inline UINT64& getReferenceCycle()
    {
        ASSERTX(referenceClockRegitry);
        return referenceClockRegitry->nCycle;
    }
    
    inline UINT64 getFirstDomainCycle()
    {
        if(firstClockRegitrySet)
            return firstClockRegitry->nCycle;
        else
            return 0;
    }
    
    inline UINT64 getNanosecond()
    {
        return (100*internalBaseCycle/Bf);
    }

    void DumpProfile(void);
    void DumpStats(STATE_OUT state_out, UINT64 total_base_cycles);

    void InitClockServer(void);


    /** Some methods to configure the clockserver */
    void SetDumpProfile(bool _bDumpProfile)
    {
        bDumpProfile = _bDumpProfile;
    }

    void SetRandomClockingSeed(UINT64 _random_seed)
    {
        random_seed = _random_seed;
    }

    void SetThreadedClocking(bool _threaded)
    {
        if (ASIM_SMP_CLASS::GetMaxThreads() > 1)
        {
            threaded = _threaded;
        }
    }

    void SetUniqueDomainOptimization(bool active)
    {
        uniqueDomainOptimization = active;
    }

    /** Returns the number of base frequency cycles forwarded */
    UINT64 Clock();   
    
    /** Clock domain related functions */
    void NewClockDomain(
        string domainName,
        list<float> freq,
        ASIM_SMP_THREAD_HANDLE tHandle = NULL);
    
    void SetReferenceClockDomain(string referenceDomain);
    
    CLOCK_DOMAIN RegisterClock(
        ASIM_CLOCKABLE m,
        string domainName,
        UINT32 skew,
        ASIM_SMP_THREAD_HANDLE tHandle = NULL);
    CLOCK_DOMAIN RegisterClock(
        ASIM_CLOCKABLE m,
        string domainName,
        CLOCK_CALLBACK_INTERFACE cb,
        UINT32 skew,
        ASIM_SMP_THREAD_HANDLE tHandle = NULL);
    
    void SetDomainFrequency(string domainName, float freq);
        
    
    /** Rate matchers related functions */
    void RegisterRateMatcherWriter(RATE_MATCHER w);
    void RegisterRateMatcherReader(RATE_MATCHER r);

    
    /** Invoques all the Clockables in order to dump their status
        just before the first cycle occurs */
    void DralTurnOn(void);

};

typedef class ASIM_CLOCK_SERVER_CLASS *ASIM_CLOCK_SERVER;

#endif /* _CLOCK_SERVER_ */

