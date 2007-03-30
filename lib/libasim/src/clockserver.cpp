/*
 * **********************************************************************
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
 * @author Santi Galan, Ken Barr, Ramon Matas Navarro
 **/

#include <stdlib.h>
#include <cstdlib>
#include <ctime>

#include "asim/clockserver.h"
#include "asim/clockable.h"
#include "asim/module.h"
#include "asim/smp.h"
#include "asim/rate_matcher.h"



/******************************************************************************
*   ASIM_CLOCKSERVER_THREAD_CLASS
*******************************************************************************/

void
ASIM_CLOCKSERVER_THREAD_CLASS::CreatePthread()
{
    VERIFYX(!ThreadActive());

    // Initialize synchronization devices
    VERIFYX(!pthread_cond_init(&wait_condition, NULL));
    VERIFYX(!pthread_mutex_init(&task_list_mutex, NULL));
    VERIFYX(!pthread_cond_init(&finish_condition, NULL));
    VERIFYX(!pthread_mutex_init(&finish_cond_mutex, NULL));

    // Initialize thread attributes
    pthread_attr_init(&thread_attr);
    pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);
    
    threadForceExit = false;

    ASIM_SMP_CLASS::CreateThread(
        &thread,
        &thread_attr,
        ASIM_CLOCKSERVER_THREAD_CLASS::ThreadWork,
        this,
        GetAsimThreadHandle());

    // Wait the pthread to be initialized
    // We are serializing the pthread creation this way
    while(!ThreadActive());
}

bool ASIM_CLOCKSERVER_THREAD_CLASS::DestroyPthread()
{
    VERIFYX(ThreadActive());

    bool return_value = true;
    UINT32* status;
    
    // Force exit
    threadForceExit = true;

    // Restart the thread (it will destroy itself)
    pthread_mutex_lock(&task_list_mutex);
    tasks_completed = false;
    pthread_cond_broadcast(&wait_condition);
    pthread_mutex_unlock(&task_list_mutex);
    
    // Wait until it finishes (return value is meaningless)
    return_value &= pthread_join(thread,(void**) &status);
      
    // Mark as terminated
    threadActive = false;

    // Destroy synchronization devices
    pthread_cond_destroy(&wait_condition);
    pthread_mutex_destroy(&task_list_mutex);
    pthread_cond_destroy(&finish_condition);
    pthread_mutex_destroy(&finish_cond_mutex);

    return return_value;
}

void * ASIM_CLOCKSERVER_THREAD_CLASS::ThreadWork(void* param)
{
    
    ASIM_CLOCKSERVER_THREAD parent = (ASIM_CLOCKSERVER_THREAD)param;
    VERIFYX(parent != NULL);

    parent->threadActive = true;
    
    // Before enter the loop lock the task_list_mutex to avoid any race condition
    pthread_mutex_lock(&(parent->task_list_mutex));
   
    while(1)
    {      
        // IMPORTANT!! We don't need to get/release the lock in this loop
        // since pthread_cond_wait does it for us.

        // Wait until we get some work to perform
        while(parent->tasks_completed)
        {
            pthread_cond_wait(&(parent->wait_condition), &(parent->task_list_mutex));
        }

        // Check if we have to exit...
        if(parent->threadForceExit)
        {
            // Not needed, for sanity...
            parent->tasks_completed = true;

            // Make sure to release the lock before exiting!
            pthread_mutex_unlock(&(parent->task_list_mutex));
            
            pthread_exit(0);
        }
        
        CLOCK_CALLBACK_INTERFACE cb = parent->GetNextWorkItem();
        while(cb != NULL)
        {              
            cb->Clock();
            cb = parent->GetNextWorkItem();
        }

        // Tell the master thread that all the work is done
        pthread_mutex_lock(&(parent->finish_cond_mutex));
        parent->tasks_completed = true;
        pthread_cond_broadcast(&(parent->finish_condition));
        pthread_mutex_unlock(&(parent->finish_cond_mutex));
                
    }    
    
    return 0;
}

/** Execute sequentially the current list of tasks */
void ASIM_CLOCKSERVER_THREAD_CLASS::PerformTasksSequential()
{
    ASSERTX(!ThreadActive());

    // Not needed
    //tasks_completed  = false;

    CLOCK_CALLBACK_INTERFACE cb = GetNextWorkItem();
    while(cb != NULL)
    {

        EVENT
        (
            cb->cReg->DralNewCycle();
        );
        
        cb->Clock();
        cb = GetNextWorkItem();
    }

    //tasks_completed  = true;
}

/******************************************************************************
*   ASIM_CLOCK_SERVER_CLASS
*******************************************************************************/

/** 
 * Constructor.
 **/
ASIM_CLOCK_SERVER_CLASS::ASIM_CLOCK_SERVER_CLASS()
    : defaultThread(NULL),
      uniqueClockDomain(false),
      uniqueDomainOptimization(true),
      threaded(false),
      referenceClockRegitry(NULL),
      firstClockRegitry(NULL),
      firstClockRegitrySet(false),
      random_seed(0),
      bDumpProfile(false)
{    
    SetTraceableName("ASIM_CLOCK_SERVER_CLASS");
}


/** 
 * Destructor. Erases the ClockRegistry entries, the domains, the threads
 * and the callbacks internally created.
 **/
ASIM_CLOCK_SERVER_CLASS::~ASIM_CLOCK_SERVER_CLASS()
{

    list<CLOCK_DOMAIN>::iterator iter_dom;
    for(iter_dom = lDomain.begin(); iter_dom != lDomain.end(); ++iter_dom)
    {

        list<CLOCK_REGISTRY>::iterator iter_reg;
        for(iter_reg = (*iter_dom)->lClock.begin();
            iter_reg != (*iter_dom)->lClock.end(); ++iter_reg)
        {
            delete *iter_reg;
        }
        
    	delete *iter_dom;
    }
    
    list<ClockCallBackClockable*>::iterator iter_callbacks;
    for(iter_callbacks = lCreatedCallbacks.begin();
        iter_callbacks != lCreatedCallbacks.end(); ++iter_callbacks)
    {
        delete *iter_callbacks;
    }

    list<ASIM_CLOCKSERVER_THREAD>::iterator iter_threads;
    for(iter_threads = lThreads.begin(); iter_threads != lThreads.end();
        ++iter_threads)
    {
        if((*iter_threads)->ThreadActive())
        {
            (*iter_threads)->DestroyPthread();
        }
        delete *iter_threads;
    }
    
    list<pthread_mutex_t*>::iterator iter_mutexs;
    for(iter_mutexs = lCreatedMutexs.begin(); iter_mutexs != lCreatedMutexs.end();
        ++iter_mutexs)
    {
        pthread_mutex_destroy(*iter_mutexs);
        delete *iter_mutexs;
    }
}


/** 
 * Dumps some clockserver stats
 *
 **/
void
ASIM_CLOCK_SERVER_CLASS::DumpStats(STATE_OUT state_out, UINT64 total_base_cycles)
{
    ostringstream os;

    os.str("");
    os << "Reference_clock_domain";
    state_out->AddScalar("string", os.str().c_str(),
        "reference clock domain name",
        referenceClockDomain->name);        
        
    list<CLOCK_DOMAIN>::iterator iter_dom;
    for(iter_dom = lDomain.begin(); iter_dom != lDomain.end(); ++iter_dom)
    {
        if((*iter_dom)->lClock.size() > 0)
        {
            os.str("");
            os << "Domain_" << (*iter_dom)->name << "_cycles_simulated";
            state_out->AddScalar("uint", os.str().c_str(),
                "number of cycles simulated for this domain",
                (*iter_dom)->lClock.front()->nCycle);

            os.str("");
            os << "Domain_" << (*iter_dom)->name << "_current_frequency_in_MHz";
            state_out->AddScalar("uint", os.str().c_str(),
                "current working frequency for this domain in MHz",
                (*iter_dom)->currentFrequency * 10);
        }
    }
}


/** 
 * Dumps all the profiling info for each ClockRegistry 
 * entry.
 **/
void ASIM_CLOCK_SERVER_CLASS::DumpProfile()
{
    
#if defined(HOST_DUNIX) | defined(HOST_LINUX_X86)

    if(bDumpProfile)
    {
        ofstream ofsProfiling("clockserver.profile");

        list<CLOCK_DOMAIN>::iterator iter_dom;
        for(iter_dom = lDomain.begin(); iter_dom != lDomain.end(); ++iter_dom)
        {        
            list<CLOCK_REGISTRY>::iterator iterFreq;
            for(iterFreq = (*iter_dom)->lClock.begin();
                iterFreq != (*iter_dom)->lClock.end(); ++iterFreq)
            {
                vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
                    endModule = (*iterFreq)->lModules.end();
                vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
                    iterModule = (*iterFreq)->lModules.begin();
                for( ; iterModule != endModule; ++iterModule)
                {
                    ASIM_CLOCKABLE mCurrent = (*iterModule).first;
        
                    TTMSG(Trace_Sys, "~ASIM_CLOCK_SERVER_CLASS: Logging " <<
                        mCurrent->ProfileId());
        
                    if(mCurrent->GetCycles() != 0)
                    {
                        ofsProfiling << mCurrent->ProfileId() << endl;
                        
                        ofsProfiling << "\tCycles spent : " <<
                            mCurrent->GetCycles() << endl;
                        ofsProfiling << "\tMax Cycles  : " <<
                            mCurrent->GetCyclesMax() << endl;
                        ofsProfiling << "\tMin Cycles  : " <<
                            mCurrent->GetCyclesMin() << endl;
                        ofsProfiling << "\tInvocations : " <<
                            mCurrent->GetNumInvocations() << endl;
                        ofsProfiling << "\tWrap around cycles spent : " <<
                            mCurrent->GetWrapAroundCycles() << endl;
                        ofsProfiling << "\tWrap around : " <<
                            mCurrent->GetWrapAround() << endl;
                        ofsProfiling << "\tWrap around cycles/invocation: " << 
                            ( mCurrent->GetWrapAround() > 0 ?
                              mCurrent->GetWrapAroundCycles() / mCurrent->GetWrapAround()
                              : 0) << endl;
    
                        ofsProfiling << "\t----------------------------" << endl;
                        ofsProfiling << "\tCycles/Invocation : " <<
                            (mCurrent->GetCycles() / mCurrent->GetNumInvocations()) <<
                            endl;
                        ofsProfiling << "\t----------------------------" << endl;
                    }
                }
            }
        }

        ofsProfiling.close();
    }

#endif

}


ASIM_CLOCKSERVER_THREAD
ASIM_CLOCK_SERVER_CLASS::MapThread(ASIM_SMP_THREAD_HANDLE tHandle)
{
    if (! threads.IsInitialized())
    {
        // Initialize the local thread ID to clockserver thread mapping array
        // the first time any thread reference is seen.
        threads.Init(ASIM_SMP_CLASS::GetMaxThreads());
        for (UINT32 i = 0; i < ASIM_SMP_CLASS::GetMaxThreads(); i++)
        {
            threads[i] = NULL;
        }
    }

    // Map Asim thread handle to a clockserver thread
    ASIM_CLOCKSERVER_THREAD thread = threads[tHandle->GetThreadId()];
    if (thread == NULL)
    {
        // Create a new thread (will be the default one for this domain)
        thread = new ASIM_CLOCKSERVER_THREAD_CLASS(tHandle);
        threads[tHandle->GetThreadId()] = thread;
        lThreads.push_back(thread);
    }

    return thread;
}


/**
 * Create a new clock domain.
 *
 **/
void
ASIM_CLOCK_SERVER_CLASS::NewClockDomain(
    string domainName,
    list<float> freq,
    ASIM_SMP_THREAD_HANDLE tHandle)
{
    // Check that the given clock domain doesn't exist
    list<CLOCK_DOMAIN>::iterator iter_dom = lDomain.begin();
    for( ; (iter_dom != lDomain.end()) && ((*iter_dom)->name!=domainName);
        ++iter_dom);
    VERIFY(iter_dom == lDomain.end(), "Clocking domain " << domainName <<
           " already exists!");

    ASIM_CLOCKSERVER_THREAD thread;
    if (tHandle != NULL)
    {
        thread = MapThread(tHandle);
    }
    else
    {
        if (!defaultThread)
        {
            ASIM_SMP_THREAD_HANDLE defaultHandle;
            if (threaded)
            {
                defaultHandle = new ASIM_SMP_THREAD_HANDLE_CLASS();
            }
            else
            {
                // Not threaded.  Just use the primary thread.
                defaultHandle = ASIM_SMP_CLASS::GetMainThreadHandle();
            }
            defaultThread = MapThread(defaultHandle);
        }

        thread = defaultThread;
    }
    
    // Create the clock domain (with its default thread)
    CLOCK_DOMAIN newDomain = new ClockDomain(domainName, thread);
    
    list<float>::iterator iter_freq = freq.begin();
    VERIFYX(iter_freq != freq.end());
    
    // The initial working frequency is by default the head of the list
    newDomain->currentFrequency = static_cast<UINT32>((*iter_freq) * 100.0 + 0.5 /* round to the closest int */);
    
    // Store all the possible frequencies
    for(; iter_freq != freq.end(); ++iter_freq)
        newDomain->normalizedFrequencies.push_back(static_cast<UINT32>((*iter_freq)
                                * 100.0 + 0.5 /* round to the closest int */));
    
    // Push the new domain in the global list
    lDomain.push_back(newDomain);
}


/**
 * Set the reference clock domain
 *
 * @param referenceDomain Clock domain name to be set.
 * If the string is "", the default clock domain will be used
 * as reference (the first in creation order)
 *
 **/
void ASIM_CLOCK_SERVER_CLASS::SetReferenceClockDomain(string referenceDomain)
{
    
    VERIFY(lDomain.size() > 0, "No clock domain has been created!");
    
    if(referenceDomain == "")
    {
        referenceClockDomain = lDomain.front();
    }
    else
    {
        list<CLOCK_DOMAIN>::iterator iter_dom = lDomain.begin();
        for(; (iter_dom != lDomain.end()) && ((*iter_dom)->name!=referenceDomain);
            ++iter_dom);

        VERIFY(iter_dom != lDomain.end(), "Clocking domain " <<
               referenceDomain << " doesn't exist!");

        referenceClockDomain = (*iter_dom);
    }
    
    VERIFY(referenceClockDomain->lClock.size() > 0,
           "No module has been registered to the reference clock domain " <<
           referenceClockDomain->name);
    
    referenceClockRegitry = referenceClockDomain->lClock.front();
    
    if(lDomain.size() > 1)
    {
        // Print the reference clock domain
        cout << "Reference clock domain is " << referenceClockDomain->name << endl;
    }
    
}


/**
 * Register a rate matcher to the given clock domain. Private method.
 *
 **/
void
ASIM_CLOCK_SERVER_CLASS::RegisterWriterRateMatcherClock( RATE_MATCHER wrm,
                CLOCK_DOMAIN domain, ASIM_CLOCKSERVER_THREAD thread, UINT32 skew)
{
      
    VERIFYX(wrm != NULL && domain != 0 && skew < 100);
    
    bool bInserted = false;
    CLOCK_REGISTRY pcrCurrent;
    list<CLOCK_REGISTRY>::iterator end = domain->lClock.end();
    list<CLOCK_REGISTRY>::iterator iter = domain->lClock.begin();
    for( ; iter != end && !bInserted; ++iter)
    {
    	pcrCurrent = *iter;
    	if(pcrCurrent->nSkew == skew) 
        {
            ClockCallBackClockable* cb =
                new ClockCallBackClockable(wrm, &ASIM_CLOCKABLE_CLASS::Clock);
            lCreatedCallbacks.push_back(cb);
            pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> p(wrm, cb);
            
    		pcrCurrent->lWriterRM.push_back(p);
            
    		bInserted = true;
            
            wrm->SetClockInfo(pcrCurrent);
            cb->setClockRegistry(pcrCurrent);
    	}
    }

    // Don't forget setting the clocking thread
    wrm->SetClockingThread(thread);
    
    // We know for sure that there is a clockRegistry with
    // the current frequency and skew. Checking it!
    VERIFYX(bInserted);

}


/**
 * Adds a new module to be clocked.
 * It will be clocked at clock domain frequency & skew
 * given.
 *
 * @param m ASIM_CLOCKABLE to be clocked.
 * @param name Clock domain name of the module.
 *
 * @param skew % of the reference cycle that must be skewed the clock
 *             of the clocked element. It's value must be between 0 and 99
 **/
CLOCK_DOMAIN
ASIM_CLOCK_SERVER_CLASS::RegisterClock(
    ASIM_CLOCKABLE m,
    string domainName,
    UINT32 skew,
    ASIM_SMP_THREAD_HANDLE tHandle)
{

    // Create a callback using the default asim_clockable virtual Clock method
    ClockCallBackClockable* cb =
        new ClockCallBackClockable(m, &ASIM_CLOCKABLE_CLASS::Clock);
    lCreatedCallbacks.push_back(cb);
    
    return RegisterClock(m, domainName, cb, skew, tHandle);
}


/**
 * Adds a new module to be clocked. It will be clocked at clock domain frequency & skew
 * given. The method func passed as parameter will be called instead of Clock.
 *
 * @param m ASIM_CLOCKABLE to be clocked.
 * @param name Clock domain name of the module.
 * @param func Name of the method that will be called.
 * @param skew % of the reference cycle that must be skewed the clock
 *             of the clocked element. It's value must be between 0 and 99
 **/
CLOCK_DOMAIN
ASIM_CLOCK_SERVER_CLASS::RegisterClock(
    ASIM_CLOCKABLE m,
    string domainName,
    CLOCK_CALLBACK_INTERFACE cb,
    UINT32 skew,
    ASIM_SMP_THREAD_HANDLE tHandle)
{

    VERIFYX(m!=NULL && skew < 100);
    
    // Find the clock domain
    list<CLOCK_DOMAIN>::iterator iter_dom = lDomain.begin();
    for( ; (iter_dom != lDomain.end()) && ((*iter_dom)->name!=domainName);
        ++iter_dom);
    VERIFY(iter_dom != lDomain.end(), "Clocking domain " << domainName <<
           " not created!");
    CLOCK_DOMAIN domain = (*iter_dom);
    
    // A negative threadId results in the use of the default domain thread
    ASIM_CLOCKSERVER_THREAD thread = domain->defaultThread;
    if(tHandle != NULL)
    {
        thread = MapThread(tHandle);
    }
    
    // IMPORTANT: Assign the thread to the module only if the module was not
    // registered!! By now, a module should be clocked only by one thread
    if(!m->IsRegistered())
    {
        m->SetClockingThread(thread);
    }
    
    // Search for a ClockRegistry with the same skew in the domain list.
    bool bInserted = false;
    INT32 dralClkId = -1;
    CLOCK_REGISTRY pcrCurrent;
    CLK_EDGE edge = cb->getClkEdge();
    
    // Dral skew must be <= 50
    UINT32 dralSkew = skew - clkEdges2skew[cb->getClkEdge()];
    
    bool create_dral_clk = true;
   
    list<CLOCK_REGISTRY>::iterator end = domain->lClock.end();
    list<CLOCK_REGISTRY>::iterator iter = domain->lClock.begin();
    for( ; iter != end && !bInserted; ++iter)
    {
    	pcrCurrent = *iter;
    	if(pcrCurrent->nSkew == skew)
        {
            
            pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> p(m, cb);
            
    		pcrCurrent->lModules.push_back(p);
            
    		bInserted = true;
            m->SetClockInfo(pcrCurrent);
    	}
        
        // If already exists a clock_registry with the same Dral skew,
        // we don't have to create a new DRAL Clock.
        if(pcrCurrent->dralSkew == dralSkew)
        {
            create_dral_clk = false;
            dralClkId = pcrCurrent->clockId;
        }
    }

    if(!bInserted)
    {
        // Create a new ClockRegistry always at the current default
        // frequency of the domain.
        pcrCurrent = new ClockRegistry(domain, skew, dralSkew, edge,
                                       create_dral_clk, cb->withPhases());
        
        pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> p(m, cb);
        
        if(!firstClockRegitrySet)
        {
            firstClockRegitrySet = true;
            firstClockRegitry = pcrCurrent;
        }

        pcrCurrent->lModules.push_back(p);
        
        domain->lClock.push_back(pcrCurrent);
        m->SetClockInfo(pcrCurrent);
        
        if(create_dral_clk) dralClkId = pcrCurrent->clockId;
        else pcrCurrent->clockId = dralClkId;
    }

    cb->setClockRegistry(pcrCurrent);

    EVENT
    (
        // IMPORTANT! By now, a node should be registered only to one clock
        if (runWithEventsOn && !(m->IsRegisteredDral()))
        {
            VERIFYX(dynamic_cast<ASIM_DRAL_NODE_CLASS *>(m) != NULL);
            VERIFYX(dralClkId >= 0);
            DRALEVENT(
            SetNodeClock(dynamic_cast<ASIM_DRAL_NODE_CLASS *>(m)->GetUniqueId(),
                         dralClkId)
            );
            m->SetRegisteredDral();
        }
    );
    
    return domain;
    
}


/**
 * Change the working frequency of the given clock domain.
 *
 **/
void ASIM_CLOCK_SERVER_CLASS::SetDomainFrequency(string domainName, float freq)
{

    CLOCK_DOMAIN domain = 0;
    
    // Find the clock domain
    bool found = false;
    list<CLOCK_DOMAIN>::iterator iter_dom = lDomain.begin();
    for( ; !found && (iter_dom != lDomain.end()); ++iter_dom)
        if(found = ((*iter_dom)->name == domainName)) domain = (*iter_dom);
    VERIFY(found, "Clocking domain doesn't exist!");    
    
    UINT32 normFreq = static_cast<UINT32>(freq * 100.0);
    
    if(normFreq == domain->currentFrequency) return;
    
    // Check if freq is a valid frequency
    found = false;
    for(UINT32 i = 0; !found && (i < domain->normalizedFrequencies.size()); ++i)
        found = (domain->normalizedFrequencies[i] == normFreq);  
    VERIFY(found, "Trying to set an invalid frequency!");
    
    cout << "Changing working frequency of domain " << domainName << " from "
         << domain->currentFrequency << " to " << normFreq << endl;
    
    // Change the current working frequency
    domain->currentFrequency = normFreq;
    
    list<CLOCK_REGISTRY>::const_iterator end = domain->lClock.end();
    list<CLOCK_REGISTRY>::const_iterator iter = domain->lClock.begin();
    for( ; iter != end; ++iter)
    {

        CLOCK_REGISTRY pcrCurrent = *iter;
        pcrCurrent->nFrequency = normFreq;
              
        // We multiply per 100 to compute the skew relative to current frequency
        pcrCurrent->nStep = Bf / pcrCurrent->nFrequency * 100;
        
        //pcrCurrent->nBaseCycle +=
        //      pcrCurrent->nSkew * Bf / pcrCurrent->nFrequency;

        // IMPORTANT!! We don't need to do this here!
        // It will be done after clocking all the modules.
        // AddTimeEvent(pcrCurrent->nBaseCycle, pcrCurrent);

    }
        
}

/**
 * Register a Rate matcher writer for connect it to it's reader
 *
 * @param w Rate matcher writer
 **/
void ASIM_CLOCK_SERVER_CLASS::RegisterRateMatcherWriter(RATE_MATCHER w)
{
    lrateWriter.push_back(w);
}

/**
 * Register a Rate matcher reader for connect it to it's writer
 *
 * @param w Rate matcher reader
 **/
void ASIM_CLOCK_SERVER_CLASS::RegisterRateMatcherReader(RATE_MATCHER r)
{   
    lrateReader.push_back(r);
}

/**
 * Adds a ClockRegistry the time event list in a ordered way
 *
 * @param time Time event to be added
 * @param freq Frequency to be added to vector
 **/
void ASIM_CLOCK_SERVER_CLASS::AddTimeEvent(UINT64 time, CLOCK_REGISTRY freq)
{

    T1("ASIM_CLOCK_SERVER::AddTimeEvent: Freq = " << freq->nFrequency << 
       " skew = " << freq->nSkew << 
       " time = " << time);

    bool bAdded = false;
    
    // TODO: Start by the end of the list for efficiency: it is more likely that the recently
    // clocked elements should be clocked after the waiting ones...
    deque<CLOCK_REGISTRY>::iterator end = lTimeEvents.end();
    deque<CLOCK_REGISTRY>::iterator iter = lTimeEvents.begin();
    for( ; iter != end && !bAdded; ++iter)
    {
        if((*iter)->nBaseCycle > time) 
        {
            T1("\tinserting at Freq = " << (*iter)->nFrequency << 
               " skew = " << (*iter)->nSkew << 
               " time = " << (*iter)->nBaseCycle);

            lTimeEvents.insert(iter, freq);

    		bAdded = true;
    	}
    }

    if(!bAdded) lTimeEvents.push_back(freq);
    
}

/**
 * Compute the step & the base cycle for each registered frequency &
 * generate the initial state of the event list.
 *
 * The event list genertation has the following phases:
 *
 * a) Connect all the registered Rate matchers
 * b) Obtain the base frequency (Bf)
 * c) Compute the Step
 * d) Add the Group to the EventTime list
 **/
void ASIM_CLOCK_SERVER_CLASS::InitClockServer(void) 
{
       
    
    // a) Connect the Rate matchers and register them to be clocked.
    //    Basically this process is composed by assigning the ClockRegistry 
    //    to the RateMatcher
    list<RATE_MATCHER>::iterator endRateWriter = lrateWriter.end();
    list<RATE_MATCHER>::iterator iter = lrateWriter.begin();
    for( ; iter != endRateWriter; ++iter)
    {
        
        // Create a mutex to protect the rate matcher port.
        // This mutex is shared between the reader and writer rate matchers
        pthread_mutex_t* mut = new pthread_mutex_t;
        UINT32 rs = pthread_mutex_init(mut, NULL);
        VERIFY(rs == 0, "Pthread mutex init failed");
        lCreatedMutexs.push_back(mut);
        
        // Assign the shared mutex to the writer
        (*iter)->setPortMutex(mut);
        
        // Obtain the connected read rate matchers
        list<RATE_MATCHER> readRM = (*iter)->getConnectedRateMatchers();
        VERIFYX(readRM.size() > 0);
        
        // Check that all them are in the same clock domain and have
        // the same clock skew!!
        list<RATE_MATCHER>::iterator rIter = readRM.begin();
        CLOCK_REGISTRY clockInfo = (*rIter)->getClockInfo();
        CLOCK_DOMAIN rmCDomain = clockInfo->clockDomain;
        UINT64 rmSkew = clockInfo->nSkew;
        (*rIter)->setPortMutex(mut);
        for(++rIter; rIter != readRM.end(); ++rIter)
        {
            clockInfo = (*rIter)->getClockInfo();
            VERIFY((rmCDomain->name == clockInfo->clockDomain->name) &&
                   (rmSkew == clockInfo->nSkew), "Write rate matcher "
                    << (*iter)->GetId() << " has readers in different clock" <<
                    " domains or with different skews!");
            
            // Assign the shared mutex to the readers
            (*rIter)->setPortMutex(mut);
        }
        
        // Finally, register the rate matcher to be clocked
        // at reader clock domain frequency
        RegisterWriterRateMatcherClock(*iter, rmCDomain, (*iter)->GetClockingThread(),
                                       rmSkew);
        
        TTMSG(Trace_Ports, "Rate matcher " << (*iter)->GetId() <<
              " connected. Domain: " << rmCDomain->name << endl);
    }

    
    // b) Compute the base frequency (Bf) for all the clockables elements
    //    that are not WriteRateMatchers. They don't need to participate
    //    in the Base Frequency computation because they MUST always match
    //    with a ASIM_CLOCKABLE located at the reader side.
    Bf = 1;

    list<CLOCK_DOMAIN>::const_iterator end_dom = lDomain.end();
    list<CLOCK_DOMAIN>::const_iterator iter_dom = lDomain.begin();
    for( ; iter_dom != end_dom; ++iter_dom)
    {
        for(UINT32 i = 0; i < (*iter_dom)->normalizedFrequencies.size(); ++i)
        {
            Bf = lcm(Bf, (*iter_dom)->normalizedFrequencies[i]);
        }
    }    
   
    // Init the clock server internal base cycle
    internalBaseCycle = 0;

    T1("ASIM_CLOCK_SERVER::InitClockServer: Bf = " << Bf);


    // c) Compute the step
    // d) Add the Group to the EventTime list
    uniqueClockDomain = (lDomain.size() <= 1);
    if(!uniqueDomainOptimization) uniqueClockDomain = false;
        
    iter_dom = lDomain.begin();
    for( ; iter_dom != end_dom; ++iter_dom)
    {   
        list<CLOCK_REGISTRY>::const_iterator end = (*iter_dom)->lClock.end();
        list<CLOCK_REGISTRY>::const_iterator iter = (*iter_dom)->lClock.begin();
        for( ; iter != end; ++iter)
        {
            CLOCK_REGISTRY pcrCurrent = *iter;
                  
            // We multiply per 100 to compute the skew relative
            // to current frequency
            pcrCurrent->nStep = Bf / pcrCurrent->nFrequency * 100;
            pcrCurrent->nBaseCycle = pcrCurrent->nSkew * Bf / pcrCurrent->nFrequency;
            
            T1("ASIM_CLOCK_SERVER::InitClockServer: Freq = " <<
                          pcrCurrent->nFrequency <<
                          " skew = " << pcrCurrent->nSkew << 
                          " step = " << pcrCurrent->nStep
                          );
                          
            AddTimeEvent(pcrCurrent->nBaseCycle, pcrCurrent);
        }
    }
    
    // Init the random state
    initstate(random_seed, (char*)random_state, CLOCKSERVER_RANDOM_STATE_LENGTH);

    if(threaded)
    {
        UINT32 max_pthreads = ASIM_SMP_CLASS::GetMaxThreads();
        VERIFY(lThreads.size() <= max_pthreads, "Max pthreads set to "
                << max_pthreads << " and the model is trying to create "
                << lThreads.size() << " pthreads");
        
        // Create the pthreads if requested
        list<ASIM_CLOCKSERVER_THREAD>::iterator iter_threads = lThreads.begin();
        for( ; iter_threads != lThreads.end(); ++iter_threads)
        {
            (*iter_threads)->CreatePthread();
        }
    }
    
}


/**
 * Calls the DralEventsTurnedOn method of all the ASIM_CLOCKABLE instances.
 */
void ASIM_CLOCK_SERVER_CLASS::DralTurnOn() 
{
    deque<CLOCK_REGISTRY>::iterator iter = lTimeEvents.begin();
    deque<CLOCK_REGISTRY>::iterator end = lTimeEvents.end();
    while(iter != end)
    {
        
        CLOCK_REGISTRY currentEvent = NULL;
        UINT32 currentFreq = 0;

        currentEvent = *iter;

        // DRALEVENT(Cycle(currentEvent->clockId, currentEvent->nCycle, 0));

        do
        {
            currentFreq = currentEvent->nFrequency;
            currentEvent->DralEventsTurnedOn();
            ++iter;
            currentEvent = *iter;
        }
        while((iter != end) && (currentFreq == (*iter)->nFrequency));
    }
}

UINT64 ASIM_CLOCK_SERVER_CLASS::Clock() 
{
        
    ASSERTX(!lTimeEvents.empty());

    // Check some basic conditions to execute specialized clock methods
    // and avoid unnecessary work
    
    if(random_seed > 0)
    {
        return RandomClock();        
    }
    
    if(threaded)
    {
        return ThreadedClock();
    }
    
    if(uniqueClockDomain)
    {
        return UniqueDomainClock();
    }
    

    // Common case with more than one clock domain and without threaded clocking
    
    deque<CLOCK_REGISTRY> lClockedEvents;
    
    CLOCK_REGISTRY currentEvent = lTimeEvents.front();
    UINT64 currentBaseCycle = currentEvent->nBaseCycle;
    UINT64 currentBaseCycleMod = currentBaseCycle/100;
    
    while(lTimeEvents.size() > 0)
    {
        
        currentEvent = lTimeEvents.front();
        if(currentBaseCycle != currentEvent->nBaseCycle) break;
            
        lTimeEvents.pop_front();
        lClockedEvents.push_back(currentEvent);
   
        // Generate dral new cycle event if necessary
        EVENT( currentEvent->DralNewCycle(); );
        
        // We clock all the modules that must be clocked at current time
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            endM = currentEvent->lModules.end();
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            iter = currentEvent->lModules.begin();            
        for( ; iter != endM; ++iter)
        {
            (*iter).second->currentCycle = currentEvent->nCycle;
            (*iter).second->Clock();
        }
        
    }
    
    // Re-add the events to the list
    // IMPORTANT: This has to be done after all the modules have
    // clocked, because they may change the frequency of some events.
    deque<CLOCK_REGISTRY>::iterator it_event = lClockedEvents.begin();
    for( ; it_event != lClockedEvents.end(); ++it_event)
    {     
        
        // We clock all the WriterRateMatcher that must be clocked at current time
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            endRM = (*it_event)->lWriterRM.end();
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            iter = (*it_event)->lWriterRM.begin();
        
        if(iter != endRM)
        {
            // Generate dral new cycle event if necessary
            EVENT( (*it_event)->DralNewCycle(); );   
        }
        
        for( ; iter != endRM; ++iter)
        {
            (*iter).second->currentCycle = (*it_event)->nCycle;
            (*iter).second->Clock();
        }
        
        (*it_event)->nCycle++;
        
        // WARNING! The step may have been modified at
        // setDomainFrequency during the clocking
        (*it_event)->nBaseCycle += (*it_event)->nStep;
        AddTimeEvent((*it_event)->nBaseCycle, (*it_event));
    }

    // Return the number of base cycles forwarded                       
    UINT64 inc = currentBaseCycleMod - internalBaseCycle;  
    internalBaseCycle = currentBaseCycleMod; 
    return inc; 
    
}

UINT64 ASIM_CLOCK_SERVER_CLASS::UniqueDomainClock() 
{
   
    // In this case we can maintain always the same event clock order.
    // Then, we don't have to pop and push the elements of the lTimeEvents list.
    
    deque<CLOCK_REGISTRY>::iterator cRegIter = lTimeEvents.begin();
    CLOCK_REGISTRY currentEvent = NULL;
    
    while(cRegIter != lTimeEvents.end())
    {
        
        currentEvent = *cRegIter;

        // Generate dral new cycle event if necessary
        EVENT( currentEvent->DralNewCycle(); );
        
        // We clock all the modules that must be clocked at current time
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            endM = currentEvent->lModules.end();
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            iter = currentEvent->lModules.begin();            
        for( ; iter != endM; ++iter)
        {
            (*iter).second->currentCycle = currentEvent->nCycle;
            (*iter).second->Clock();
        }

        // We clock all the WriterRateMatcher that must be clocked at current time
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            endRM = currentEvent->lWriterRM.end();
        iter = currentEvent->lWriterRM.begin();
            
        for( ; iter != endRM; ++iter)
        {
            (*iter).second->currentCycle = currentEvent->nCycle;
            (*iter).second->Clock();
        }
        
        currentEvent->nCycle++;
        currentEvent->nBaseCycle += currentEvent->nStep;
        
        ++cRegIter;
        
    }
    
    // Return the number of base cycles forwarded
    ASSERTX(currentEvent != NULL);
    UINT64 currentBaseCycle = (currentEvent->nBaseCycle - currentEvent->nStep)/100;
    UINT64 inc = currentBaseCycle - internalBaseCycle;
    internalBaseCycle = currentBaseCycle; 
    return inc;
    
}

UINT64 ASIM_CLOCK_SERVER_CLASS::ThreadedClock() 
{
    deque<CLOCK_REGISTRY> lClockedEvents;
    
    CLOCK_REGISTRY currentEvent = lTimeEvents.front();
    UINT64 currentBaseCycle = currentEvent->nBaseCycle;
    UINT64 currentBaseCycleMod = currentBaseCycle/100;
    
    while(true)
    {
        
        if(lTimeEvents.size() > 0) currentEvent = lTimeEvents.front();
        
        if((lTimeEvents.size() <= 0) ||
           (currentBaseCycle != currentEvent->nBaseCycle))
        {
            
            if(!eventsOn && !traceOn)
            {
                // Perform task threaded
                
                // Wake up threads
                list<ASIM_CLOCKSERVER_THREAD>::iterator
                    iter_threads = lThreads.begin();
                for( ; iter_threads != lThreads.end(); ++iter_threads)
                {
                    (*iter_threads)->PerformTasksThreaded();
                }

                // Cycle-by-cycle barrier: wait for all threads to finish
                iter_threads = lThreads.begin();
                for( ; iter_threads != lThreads.end(); )
                {
                    if((*iter_threads)->CheckAllTasksCompleted())
                        ++iter_threads;
                }
            }
            else
            {

                // Perform tasks sequentially
                list<ASIM_CLOCKSERVER_THREAD>::iterator
                    iter_threads = lThreads.begin();
                for( ; iter_threads != lThreads.end(); ++iter_threads)
                {
                    (*iter_threads)->PerformTasksSequential();
                }

            }
            
            // Re-add the events to the list
            // IMPORTANT: This has to be done after all the threads have
            // finished, because they may change the frequency of some events.
            deque<CLOCK_REGISTRY>::iterator it_event = lClockedEvents.begin();
            for( ; it_event != lClockedEvents.end(); ++it_event)
            {
                (*it_event)->nCycle++;
                
                // WARNING! The step may have been modified at
                // setDomainFrequency during the clocking
                (*it_event)->nBaseCycle += (*it_event)->nStep;
                AddTimeEvent((*it_event)->nBaseCycle, (*it_event));
            }

            // Return the number of base cycles forwarded                       
            UINT64 inc = currentBaseCycleMod - internalBaseCycle;  
            internalBaseCycle = currentBaseCycleMod; 
            return inc;
        }
        
        lTimeEvents.pop_front();
        lClockedEvents.push_back(currentEvent);
   
        // We insert all the modules that must be clocked at current
        // time in the threads task list
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            endM = currentEvent->lModules.end();
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            iter = currentEvent->lModules.begin();
            
        for( ; iter != endM; ++iter)
        {
            (*iter).second->currentCycle = currentEvent->nCycle;
            (*iter).first->GetClockingThread()->AssignTask((*iter).second);
        }

        // We insert all the WriterRateMatcher that must be clocked at
        // current time in the threads task list
        vector< pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE> >::iterator
            endRM = currentEvent->lWriterRM.end();
        iter = currentEvent->lWriterRM.begin();
            
        for( ; iter != endRM; ++iter)
        {
            (*iter).second->currentCycle = currentEvent->nCycle;
            
            // The clocking thread must be the same than the writer
            // in order to be clocked after it. It is guaranteed just by
            // redefining GetClockingThread in the rate matcher
            VERIFYX(dynamic_cast<RATE_MATCHER>((*iter).first)!=NULL);
            dynamic_cast<RATE_MATCHER>((*iter).first)->GetClockingThread()->
                AssignTask((*iter).second);
        }
        
    }
    
    VERIFYX(false);
    return 0;
    
}

/**
 * Method that clocks the registered modules in function of their
 * frequencies. With this method we produce a random order of all the
 * modules that belongs to a ClockRegistry
 *
 **/ 
UINT64 ASIM_CLOCK_SERVER_CLASS::RandomClock() 
{

    ASSERT(!threaded, "Threaded random clocking not implemented by now!");

    CLOCK_REGISTRY currentEvent = lTimeEvents.front();
    UINT64 currentBaseCycle = currentEvent->nBaseCycle;
    
    while(true)
    {
  
        currentEvent = lTimeEvents.front();                
        if(currentBaseCycle != currentEvent->nBaseCycle)
        {
            UINT64 inc = currentBaseCycle / 100 - internalBaseCycle;
            internalBaseCycle = currentBaseCycle / 100;
            return inc;
        }
        lTimeEvents.pop_front();

        // We clock all the frequencies that must be clocked at current time
        // Generate a random order between all the elements inside the frequency
        size_t vSize = currentEvent->lModules.size();
        while(vSize)
        {
            
            // We have to set the random_state before calling the random generator
            char *tmp_state = setstate((char *)random_state);            
            UINT32 i = random() % vSize;
            setstate(tmp_state);

            EVENT ( currentEvent->lModules[i].second->cReg->DralNewCycle(); )
            
            currentEvent->lModules[i].second->currentCycle = currentEvent->nCycle;
            currentEvent->lModules[i].second->Clock();

            // Swap the selected element with the last one
            pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE>
                tmp = currentEvent->lModules[vSize - 1];
            currentEvent->lModules[vSize - 1] = currentEvent->lModules[i];
            currentEvent->lModules[i] = tmp;
            --vSize;
        }

        // We clock all the WriterRateMatcher that
        // must be clocked at current time
        vSize = currentEvent->lWriterRM.size();
        while(vSize)
        {
            
            // We have to set the random_state before calling
            // the random generator
            char *tmp_state = setstate((char *)random_state);            
            UINT32 i = random() % vSize;
            setstate(tmp_state);

            EVENT ( currentEvent->lWriterRM[i].second->cReg->DralNewCycle(); )
            
            currentEvent->lWriterRM[i].second->currentCycle = currentEvent->nCycle;
            currentEvent->lWriterRM[i].second->Clock();

            // Swap the selected element with the last one
            pair<ASIM_CLOCKABLE, CLOCK_CALLBACK_INTERFACE>
                tmp = currentEvent->lWriterRM[vSize - 1];
            currentEvent->lWriterRM[vSize - 1] = currentEvent->lWriterRM[i];
            currentEvent->lWriterRM[i] = tmp;
            --vSize;
            
        }

        currentEvent->nCycle++;
        
        // WARNING! The step may have been modified at setDomainFrequency
        currentEvent->nBaseCycle += currentEvent->nStep;
        
        AddTimeEvent(currentEvent->nBaseCycle, currentEvent);
    
    }
    
    VERIFYX(false);
    return 0;    
    
}


/**
 * Calls the DralEventsTurnedOn method of all the ASIM_CLOCKABLE instances
 * within the same clock registry
 */
void ClockRegistry::DralEventsTurnedOn ()
{
    vector<ASIM_CLOCKABLE>::iterator iter = lEventsTurnOn.begin();
    vector<ASIM_CLOCKABLE>::iterator end = lEventsTurnOn.end();
    for (;iter != end; ++iter)
    {
        (*iter)->DralEventsTurnedOn();
    }
}

