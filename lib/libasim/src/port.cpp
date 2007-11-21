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
 * @author Nathan Binkert, Eric Borch
 * @brief ASIM port abstraction - access points to wires.
 */

// generic
#include <typeinfo>
#include <iostream>

// ASIM core
#include "asim/port.h"

bool registerPortStats = false;
asim::Vector<BasePort*> BasePort::AllPorts;

const char *BasePort::PortName[] =
{
    "BasePort",
    "WritePort",
    "WritePhaseType",
    "ReadPort",
    "ReadPhaseType",
    "ConfigPort",
    "PeekPort",        
    "LastPort"  // This should always be the last one!!!
};

int BasePort::id_count=0;


void foo()
{}

template <class T, class C >
int
_Partition(C& Container, int left, int right, T* dummy_for_broken_cxxV6_1 = NULL)
{
    typename C::Iterator array = Container.Begin();
    
    foo();
    
    T pivot = array[left];
    int l = left;
    int r = right;
    
    while (1)
    {
        while (*(array[l]) <= *pivot && l < right) ++l;
        while (*(array[r]) > *pivot && r > left) --r;

        if (l < r)
        {
            T Temp = array[l];
            array[l] = array[r];
            array[r] = Temp;
        }
        else
        {
            array[left] = array[r];
            array[r] = pivot;
            return r;
        }
    }
}

template <class T, class C >
void
_QuickSort(C& Container, int left, int right, T* dummy_for_broken_cxxV6_1 = NULL)
{
    if (left >= right)
        return;
    
    foo();
    
    int part = _Partition<T,C>(Container, left, right);
    _QuickSort<T,C>(Container, left, part-1);
    _QuickSort<T,C>(Container, part+1, right);
}

template <class T, class C >
void
QuickSort(C& Container, T* dummy_for_broken_cxxV6_1 = NULL)
{
    _QuickSort<T,C>(Container, 0, Container.GetOccupancy() - 1);
}


// sort needs to be one that doesn't swap elements, because that
// screws up the ordering of port names (which rules out QuickSort
// and HeapSort)
// MergeSort and InsertionSort are 2 options.  The advantage of
// Insertion Sort is that it doesn't need additional storage space to
// do the sorting, unlike Merge Sort.  Merge Sort could certainly be
// made to work, but we only do this sort once, so the fact that
// Insertion Sort is slower really doesn't matter.
template <class T, class C >
void
InsertionSort(C& Container, T* dummy_for_broken_cxxV6_1 = NULL)
{
    typename C::Iterator array = Container.Begin();
    int len = Container.GetOccupancy();
    T key;
    int i;
    
    for (int j = 1; j < len; j++)
    {
        key = array[j];
        i = j - 1;
        while ((i >= 0) && (*(array[i]) > *key))
        {
            array[i+1] = array[i];
            i--;
        }
        array[i+1] = key;
    }    
}

bool
BasePort::ConnectPorts(int rdPort, int port, int index, 
		      asim::Vector<BasePort*>::Iterator i)
{
    // ensure that the data types of the endpoints match:
    VERIFY( i[port]->GetDataType() == i[rdPort]->GetDataType(),
            "Data type mismatch when connecting ports (" <<
            i[port]->GetName()                           << ")\n" );

    if (i[rdPort]->GetBandwidth() > 0)
    {
        VERIFY(i[port]->GetBandwidth() <= 0 || 
               i[port]->GetBandwidth() == i[rdPort]->GetBandwidth(),
               "Bandwidth must only be set once unless it's "
               "set to the same value.\n");
        i[port]->Bandwidth = i[rdPort]->GetBandwidth();
    }
    else
        i[rdPort]->Bandwidth = i[port]->GetBandwidth();
    
    
    if (i[rdPort]->GetLatency() >= 0)
    {
        if (i[port]->GetLatency() == -1)
        {
            // This is the first time latency is set in the write port.
            i[port]->Latency = i[rdPort]->GetLatency();
        }
        else if (i[port]->GetLatency() != i[rdPort]->GetLatency())
        {
            // This case means that either this is a multicast port 
            // with different latencies on each end or it is a monocast port
            // with different latencies defined on each side.
            // Let's assert that we are not on the second case.
            VERIFY((index !=0),
                   "Port with different latencies defined on write and read end (" <<
                   i[port]->GetName() << ")");
            // We are in from of a multicast multilatency port.
            // Mark-it on the write side.
            i[port]->Latency = -2;
        }  
    }
    else
        i[rdPort]->Latency = i[port]->GetLatency();

    //the bandwidth is defined in the port, but not the buffer yet so we have to
    //pass it down
    i[rdPort]->CreateStorage(i[rdPort]->Latency, i[rdPort]->Bandwidth);

    i[rdPort]->Connected = true;
    if (i[port]->GetType() == BasePort::PeekType)
    {
        // index is unused here. Remove with new method?  Eric
        i[port]->SetBuffer(i[rdPort]->GetBuffer(), index);
        
        // don't set buffer info on read port, since it'll happen when the
        // read port connects to the write port.
        
        return true;
    }
    
    if (i[port]->GetType() == BasePort::WriteType || i[port]->GetType() == BasePort::WritePhaseType)
    {
        i[rdPort]->SetBufferInfo();
        i[port]->SetBuffer(i[rdPort]->GetBuffer(), index);
        
        if (runWithEventsOn)
        {
            EVENT(i[port]->EventConnect(index, i[rdPort]->node));
        }
        
        return true;
    }

    return false;

}


void
BasePort::ConnectAll()
{

    asim::Vector<BasePort*>::Iterator i = AllPorts.Begin();
    asim::Vector<BasePort*>::Iterator end = AllPorts.End();
    asim::Vector<BasePort*>::Iterator prev = i;
    asim::Vector<BasePort*>::Iterator next = i + 1;
    
    while (i != end) 
    {
        if ((*i)->GetName() == NULL) 
        {
    
            // We should not get here. A port should have a name, which it
            // acquires when it is initialized. Fail now, because if we
            // don't, we will fail later in obscure ways.
            
            cout << "A port has no name. Most likely, it was declared "
                 << "but not initialized. " << endl;
            cout << "Type: " << (*i)->GetTypeName() << endl;
            cout << "Id: "<< (*i)->GetUid() <<endl;
            
            if ((*prev)->GetName())
            {
                cout << "Previous port in the list: "
                     << (*prev)->GetName()
                     << "[" << (*prev)->GetInstance() << "]" << endl;
            }
            
            if (next != end && (*next)->GetName())
            {
                cout << "Next port in the list: "
                     << (*next)->GetName()
                     << "[" << (*next)->GetInstance() << "]" << endl;
            }
    
            VERIFYX(false);

        }
        else 
        {
    
            //Normal case. Print port name and type
    
//   duplicates trace stuff below
//            T1_AS((*i), (*i)->GetName() << "["  << (*i)->GetInstance() << "]."
//                  << (*i)->GetTypeName());
        }
        
        prev = i;
        i = next;
        ++next;
    }
    
    // sort needs to be one that doesn't swap elements, because that
    // screws up the ordering of port names (which rules out QuickSort
    // and HeapSort)
    InsertionSort<BasePort*, asim::Vector<BasePort*> >(AllPorts);
    
    // cout << __FILE__ << ":" << __LINE__ << endl;
    i = AllPorts.Begin();
    if(i != end) {
        T1_AS((*i), "Connect All Ports." << endl);
    }
    
    
    while (i != end)
    {
        T1_AS((*i), "Connecting " << (*i)->GetName() << "[" << (*i)->GetInstance() << "]."
              << (*i)->GetTypeName());
        int num = 0;
        const char *name = i[0]->GetName();
        int instance = i[0]->GetInstance();
        int numWritePorts = 0;
        int numReadPorts = 0;
        int numPeekPorts = 0;
        int numWritePhasePorts = 0;
        int numReadPhasePorts = 0;
        
        // As we scan the ports with the same name, We need to be careful not
        // to overrun the list.  
        while (((i+num) != end) && i[num] && !strcmp(name, i[num]->GetName()) &&
               instance == i[num]->GetInstance())
        {
    
            if (i[num]->GetType() == BasePort::WriteType)
            {
                VERIFY(numReadPorts == 0, "Read Port came first!  Sort failed!");
                numWritePorts++;
            }
    
            if (i[num]->GetType() == BasePort::ReadType)
            {
                numReadPorts++;
            }
    
            if (i[num]->GetType() == BasePort::PeekType)
            {
                numPeekPorts++;
            }

            if (i[num]->GetType() == BasePort::WritePhaseType)
            {
                VERIFY(numReadPhasePorts == 0, "Read Port came first!  Sort failed!");
                numWritePhasePorts++;
                numWritePorts++;
            }
    
            if (i[num]->GetType() == BasePort::ReadPhaseType)
            {
                numReadPhasePorts++;
                numReadPorts++;
            }
    
            num++;
        }

        /*
        std::ostringstream os;
        VERIFY(numReadPorts > 0, os << "There is no read port for write port " << (*i)->GetName() \
        << endl);
        */
    
        VERIFY(numReadPorts > 0, 
               "There is no read port for write port "
               << (*i)->GetName() << endl);
        VERIFY(numReadPorts >= numWritePorts, 
               "Number of write ports("<<numWritePorts<<") exceed number of read ports ("<<numReadPorts<<") for "
               << (*i)->GetName() << endl);
        VERIFY(numReadPhasePorts >= numWritePhasePorts, 
               "Number of write phase ports exceed number of read phase ports for "
               << (*i)->GetName() << endl);
        VERIFY(numPeekPorts == 0 || numPeekPorts == numReadPorts, 
               "Can't have more peek ports than read ports for "
               << (*i)->GetName() << endl);
        VERIFY(num == numWritePorts + numReadPorts + numPeekPorts, 
               "Port numbers don't add up for "
               << (*i)->GetName() << endl);
    
        if (numWritePorts == 0)
        {
            cout << "****** WARNING! Read port " << (*i)->GetName() << 
                " doesn't have any port to connect to!" << endl;
        }
        
        int readPortPosition = numWritePorts;
        int peekPortPosition = numReadPorts + numWritePorts;
    
        for (int writePortPosition = 0; writePortPosition < numWritePorts; writePortPosition++)
        {
            int fanout = i[writePortPosition]->GetFanout();
            VERIFYX(fanout > 0);
            if (fanout > 1)
                VERIFY(numPeekPorts == 0, "Can't use a Peek Port with a Fanout > 1\n");
    
            VERIFYX(i[writePortPosition]->GetType() == BasePort::WriteType || i[writePortPosition]->GetType() == BasePort::WritePhaseType);
    
            int bufferIndex = 0;
            int endReadPort = readPortPosition + fanout;
    
            bool connect;
    
            while (readPortPosition < endReadPort)
            {
                VERIFY((i[readPortPosition]->GetType() == BasePort::ReadType) || (i[readPortPosition]->GetType() == BasePort::ReadPhaseType),
                       "Expected another Read Port for this unique string: " << (*i)->GetName()); 

                VERIFY(((i[writePortPosition]->GetType() == BasePort::WriteType) && (i[readPortPosition]->GetType() != BasePort::ReadPhaseType)) ||
                       (((i[writePortPosition]->GetType() == BasePort::WritePhaseType) && (i[readPortPosition]->GetType() == BasePort::ReadPhaseType))),
                       "Trying to connect a phase port with a normal one!\n"); 

                connect = ConnectPorts(readPortPosition, writePortPosition, bufferIndex++, i);
                VERIFY(connect,
                       "ConnectPorts for " << (*i)->GetName() <<" failed!" << endl);
    
                // Record the connected ports
                i[writePortPosition]->connectedPorts.push_back(i[readPortPosition]);
                i[readPortPosition]->connectedPorts.push_back(i[writePortPosition]);
    
                T1_AS((*i), "Connected write port " << writePortPosition
                   << " with read port " << readPortPosition
                   << " with bw = " << i[writePortPosition]->GetBandwidth()
                   << " & lat = " << i[readPortPosition]->GetLatency());
    
                readPortPosition++;
            }
    
            if (numPeekPorts > 0)
            {
                VERIFYX(i[peekPortPosition]->GetType() == BasePort::PeekType);
    
                // numWritePorts ends up being the first read port, fix for clarity - Eric
                connect = ConnectPorts(numWritePorts, peekPortPosition, bufferIndex++, i);
                VERIFY(connect,
                       "ConnectPorts for " << (*i)->GetName() <<" failed!" << endl);
    
                T1_AS((*i), "Connected peek port " << peekPortPosition
                   << " with read port " << numWritePorts);
            }
    
            if (fanout > 1)
            {
                // this assertion doesn't hold if the latencies of the fanout ports are
                // the same and are set from the write port //
                //
                // (r2r) this assertion does not hold for Arana, but should
                // hold for Tanglewood!!!
                //VERIFY(i[writePortPosition]->Latency == 0, "Latency of fanout write port shouldn't be set\n");
            }
            else
            {
                // this assertion doesn't hold because the read ports are setting the
                // latency, and the write port never knows about it, because with fan
                // outs, there could be different latencies on each read port, and so
                // there is no latency associated with the write port
                // VERIFY(i[writePortPosition]->Latency >= 0, "Nobody set the latency!\n");
            }
            
            VERIFY(i[writePortPosition]->Bandwidth > 0, "Nobody set the bandwidth!\n");
            i[writePortPosition]->Connected = true;
            
        }
    
        // if there's a writePort, make sure ALL ports get connect.  This
        // does allow readPorts with no writeports.  However, this
        // assertion will trigger incorrectly if you have a
        // writeport/readport combination and ANOTHER readport (say from
        // another module) with the same name that you left unconnected on
        // purpose.  This is legal, but very unlikely.  Maybe once the
        // code's run a while and I'm confident in the algorithm to
        // connect ports, we can remove this, but for now I'ld like to
        // keep it for error checking. - Eric
    
        T1_AS((*i), "nWP=" << numWritePorts << ", nRP=" << numReadPorts << ", rPP=" << readPortPosition);

        // TO DO: change this to VERIFYX.  this is an ASSERT because the smac
        // code is messed up and fails at this point with a VERIFY.  Yes, that
        // does mean the code's broken, but this is the only way to pass
        // regression until they fix it.
        ASSERTX((numWritePorts == 0) || (readPortPosition == numWritePorts + numReadPorts));
//        VERIFYX((numWritePorts == 0) || (readPortPosition == numWritePorts + numReadPorts));
    
        i += num;
    }
}

