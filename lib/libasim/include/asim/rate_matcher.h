/*****************************************************************************
 *
 * @brief   Port wrapper to allow different frequency communication 
 *
 * @author  Santi Galan, Ramon Matas Navarro
 *
 *Copyright (C) 2004-2006 Intel Corporation
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

#ifndef _RATE_MATCHER_
#define _RATE_MATCHER_

// C++/STL
#include <string>
#include <sstream>
#include <pthread.h>

// ASIM core
#include "asim/clockable.h"
#include "asim/port.h"

using namespace std;

#define DEFAULT_MAX_BANDWIDTH 8
#define DEFAULT_MAX_LATENCY 8


#if MAX_PTHREADS > 1
  #define cs_lock(p)   pthread_mutex_lock(p);
  #define cs_unlock(p) pthread_mutex_unlock(p);  
#else
  #define cs_lock(p)
  #define cs_unlock(p)
#endif

//************************************************************************************************
// Class RateMatcher
//************************************************************************************************

class RateMatcher: public ASIM_CLOCKABLE_CLASS
{

    protected:
    
      string id;            //> Rate matcher id
      bool listCreated;     //> Has the connectedRateMatchers list been created?
      bool initialized;     //> Has the RateMatcher been initialized?
      
      // Rate matchers connected to the current one
      list<RateMatcher*> connectedRateMatchers;

      ASIM_CLOCKABLE clockable;  // Module where the rate matcher is attached to
      
      pthread_mutex_t* port_mutex;  // Mutex to access the port 
      
    public:
    
      RateMatcher(ASIM_CLOCKABLE _clockable)
        : ASIM_CLOCKABLE_CLASS(NULL),
          listCreated(false),
          initialized(false),
          clockable(_clockable)
      { /* Nothing */ }
      
      virtual ~RateMatcher() { } 
    
      const char* GetId()
      {
          VERIFYX(initialized);
          return id.c_str();
      }
      
      // Returns the ClockRegistry of the module that has created the rate matcher
      virtual CLOCK_REGISTRY getClockInfo() = 0;

      // Returns the Clockserver_thread of the module that has created the rate matcher
      virtual ASIM_CLOCKSERVER_THREAD GetClockingThread() = 0;

      // Returns the list of the rate matchers connected to the current one
      virtual list<RateMatcher*> getConnectedRateMatchers() = 0;

      virtual void Clock(UINT64 cycle) = 0;

      void setPortMutex(pthread_mutex_t* _mutex)
      {
          port_mutex = _mutex;
      }
      
      // Virtual clockable methods that have to be implemented...
      const char *ProfileId(void) const
      {
          VERIFYX(initialized);
          string tmp = id + "_Rate_Matcher";
          return tmp.c_str();
      }
      
};



//************************************************************************************************
// Class ReadRateMatcher
//************************************************************************************************

template<class T, int W = DEFAULT_MAX_BANDWIDTH, int L = DEFAULT_MAX_LATENCY>
class ReadRateMatcher: public ReadPort<T>, public RateMatcher
{
    
  public:
  
    ReadRateMatcher(ASIM_CLOCKABLE _clockable):
        ReadPort<T>(),
        RateMatcher(_clockable)
    {
        // Nothing.
    }
    
    ~ReadRateMatcher() { }
    
    // From RateMatcher
    CLOCK_REGISTRY getClockInfo()
    {
        return clockable->GetClockInfo();
    }
    
    ASIM_CLOCKSERVER_THREAD GetClockingThread()
    {
        VERIFY(false, "This write matcher function should not be called!");
        return NULL;
    }
        
    list<RateMatcher*> getConnectedRateMatchers();
    
    void Clock(UINT64 cycle)
    {
        VERIFY(false, "The read rate matcher should not be clocked!");
    }

    
    // Redefined from BasePort
    bool Init(const char *name, int nodeId = 0, int instance = 0, const char *scope = NULL);
    bool InitConfig(const char *name, int bw, int lat, int nodeId = 0 );

    // Read is redefined to lock the port before accessing in a multithreaded simulation
    bool Read(T& data, UINT64 cycle);
      
};

template<class T, int W, int L>
inline bool
ReadRateMatcher<T,W,L>::Read(T& data, UINT64 cycle)
{

    cs_lock(port_mutex);
    bool ret = ReadPort<T>::Read(data, cycle);
    cs_unlock(port_mutex);
    
    return ret;
}

template<class T, int W, int L>
bool
ReadRateMatcher<T,W,L>::Init(const char *name, int nodeId, int instance, const char *scope)
{

    bool ret = BasePort::Init(name, nodeId, instance, scope);
    
    if(ret)
    {
        
        // IMPORTANT: the id should be initialized before calling the register method!!
        // (it may call GetId())
        
        id = "";
        stringstream ss;
        ss.clear();
        // ss << this->GetName() << "_" << this->GetInstance();
        ss << this->GetName();
        ss >> id;

        initialized = true;
        
        // The clockserver should know about us.
        RegisterRateMatcherReader((RateMatcher*)this);
        
        TTMSG(Trace_Ports, "Read rate matcher " << id << " initialized." << endl);
    }
    
    return ret;

}


template<class T, int W, int L>
bool
ReadRateMatcher<T,W,L>::InitConfig(const char *name, int bw, int lat, int nodeId)
{
    
    bool ret = BasePort::InitConfig(name, bw, lat, nodeId);
    
    if(ret)
    {
        
        // IMPORTANT: the id should be initialized before calling the register method!!
        // (it may call GetId())

        id = "";
        stringstream ss;
        ss.clear();
        // ss << this->GetName() << "_" << GetInstance();
        ss << this->GetName();
        ss >> id;

        initialized = true;
        
        // The clockserver should know about us.
        RegisterRateMatcherReader((RateMatcher*)this);

        TTMSG(Trace_Ports, "Read rate matcher " << id << " initialized." << endl);
    }
    
    return ret;
    
}

template<class T, int W, int L>
list<RateMatcher*>
ReadRateMatcher<T,W,L>::getConnectedRateMatchers()
{

    if(listCreated) return connectedRateMatchers;

    // Access the BasePort list of connected ports    
    list<BasePort*> connectedPorts = ReadPort<T>::BasePort::connectedPorts;
    VERIFY(connectedPorts.size() > 0, "No WriteRateMatcher connected to ReadRateMatcher " << id);
    
    for(list<BasePort*>::iterator it = connectedPorts.begin(); it != connectedPorts.end(); it++)
    {
        RateMatcher* rm = dynamic_cast<RateMatcher*>(*it);
        VERIFY(rm, "Read rate matcher " << id << " connected to a normal write port.");
        connectedRateMatchers.push_back(rm);
    }
    
    listCreated = true;
    
    return connectedRateMatchers;

}


//************************************************************************************************
// Class WriteRateMatcher
//************************************************************************************************

template<class T, int F = 1, int W = DEFAULT_MAX_BANDWIDTH, int L = DEFAULT_MAX_LATENCY>
class WriteRateMatcher: public WritePort<T,F>, public RateMatcher
{
    
  private:
  
    T internalBuffer[W];    // Internal buffer storage
    INT32 currentPosition;  // Current buffer index
    
    const T Dummy;

  public:
  
    WriteRateMatcher(ASIM_CLOCKABLE _clockable):
        WritePort<T,F>(),
        RateMatcher(_clockable),
        currentPosition(0),
        Dummy(T())
    {   
        for(UINT32 i = 0; i < W; i++)
        {
            internalBuffer[i] = Dummy;
        }
    }
    
    ~WriteRateMatcher()
    {
        // Clear the buffer: releases smart pointers
        for(UINT32 i = 0; i < W; i++)
        {
            internalBuffer[i] = Dummy;
        }
    }
    
    // From RateMatcher
    CLOCK_REGISTRY getClockInfo()
    {
        VERIFY(false, "This write matcher function should not be called!");
        return NULL;
    }

    ASIM_CLOCKSERVER_THREAD GetClockingThread()
    {
        return clockable->GetClockingThread();
    }
    
    list<RateMatcher*> getConnectedRateMatchers();
        
    // Clock function moves data from the internal buffer to the port buffer.
    // IMPORTANT!! The rate matcher must be clocked after the writer module and
    // at the reader frequency. The clockserver is responsible of doing this.
    void Clock(UINT64 cycle);

    
    // Write is redefined: data is stored in an internal buffer and moved
    // into the real port when the rate matcher is clocked.
    bool Write(const T data, UINT64 cycle);
        
    // Redefined from BasePort
    bool Init(const char *name, int nodeId = 0, int instance = 0, const char *scope = NULL);
    bool InitConfig(const char *name, int bw, int lat, int nodeId = 0 );

    
    // Returns true if there is free space in the buffer.
    bool CanWrite();

};


template<class T, int F, int W, int L>
bool
WriteRateMatcher<T,F,W,L>::Init(const char *name, int nodeId, int instance, const char *scope)
{

    bool ret = BasePort::Init(name, nodeId, instance, scope);
    
    if(ret)
    {
        
        // IMPORTANT: the id should be initialized before calling the register method!!
        // (it may call GetId())
        
        id = "";
        stringstream ss;
        ss.clear();
        ss << this->GetName();
        ss >> id;

        initialized = true; 
                
        // The clockserver should know about us.
        RegisterRateMatcherWriter((RateMatcher*)this);
        
        TTMSG(Trace_Ports, "Write rate matcher " << id << " initialized." << endl);
    }
    
    return ret;

}


template<class T, int F, int W, int L>
bool
WriteRateMatcher<T,F,W,L>::InitConfig(const char *name, int bw, int lat, int nodeId)
{
    
    bool ret = BasePort::InitConfig(name, bw, lat, nodeId);
    
    if(ret)
    {

        // IMPORTANT: the id should be initialized before calling the register method!!
        // (it may call GetId())

        id = "";
        stringstream ss;
        ss.clear();
        ss << this->GetName();
        ss >> id;

        initialized = true;
        
        // The clockserver should know about us.
        RegisterRateMatcherWriter((RateMatcher*)this);
  
        TTMSG(Trace_Ports, "Write rate matcher " << id << " initialized." << endl);
    }
    
    return ret;
    
}


template<class T, int F, int W, int L>
inline bool
WriteRateMatcher<T,F,W,L>::CanWrite()
{
    // Is there any free bucket in the internal buffer?
    return (currentPosition < this->Bandwidth);
}


template<class T, int F, int W, int L>
inline bool
WriteRateMatcher<T,F,W,L>::Write(T data, UINT64 cycle)
{
    
    ASSERTX(this->IsConnected());
       
    // By now, abort in case the buffer is full (just in case...)
    VERIFY((currentPosition < this->Bandwidth),
           "Internal rate matcher buffer full! Bandwidth exceeded" <<
           " this reader cycle. Rate matcher name: " << id);
    
    bool write = (currentPosition < this->Bandwidth);
    
    if(write)
    {
        internalBuffer[currentPosition] = data;
        currentPosition++;
        
        TTMSG(Trace_Ports, "Rate matcher " << id << " write. Current position: "
              << currentPosition << ".");
    }
    
    return write;
    
}


template<class T, int F, int W, int L>
void
WriteRateMatcher<T,F,W,L>::Clock(UINT64 cycle)
{
    
    TTMSG(Trace_Ports, "Clocking rate matcher " << id << ". Reader cycle: " << cycle);
    
    ASSERTX(this->IsConnected());
    
    // Make sure to have the lock when accessing the port
    cs_lock(port_mutex);
    for(INT32 i = 0; i < currentPosition; i++)
    {                
        // Move the data to the internal WritePort buffer.
        WritePort<T,F>::Write(internalBuffer[i], cycle);
        
        // Release smart pointer.
        internalBuffer[i] = Dummy;
               
        TTMSG(Trace_Ports, "Internal buffer position " << i << " sent.");
    }
    cs_unlock(port_mutex);
    
    currentPosition = 0;
    
}


template<class T, int F, int W, int L>
list<RateMatcher*>
WriteRateMatcher<T,F,W,L>::getConnectedRateMatchers()
{

    if(listCreated) return connectedRateMatchers;
    
    list<BasePort*> connectedPorts = WritePort<T,F>::BasePort::connectedPorts;
    VERIFY(connectedPorts.size() > 0, "No ReadRateMatcher connected to WriteRateMatcher " << id);
    
    for(list<BasePort*>::iterator it = connectedPorts.begin(); it != connectedPorts.end(); it++)
    {
        RateMatcher* rm = dynamic_cast<RateMatcher*>(*it);
        VERIFY(rm, "Write rate matcher " << id << " connected to a normal read port.");
        connectedRateMatchers.push_back(rm);
    }
    
    listCreated = true;
    
    return connectedRateMatchers;

}

#endif /* _RATE_MATCHER_ */

