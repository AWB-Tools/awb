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
 * @author David Goodwin, Shubu Mukherjee
 * @brief
 */

#ifndef _BUFFER_
#define _BUFFER_

// generic
#include <stdio.h>
#include <ioformat.h>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/module.h"
#include "asim/ioformat.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

typedef class ASIM_BUFFER_CLASS *ASIM_BUFFER;
typedef class ASIM_MODULE_CLASS *ASIM_MODULE; 

extern bool registerBufferStats; 
extern bool debugOn; 

#define BUFPTR_INCR_1(N) (UP_1_WRAP((N), bufEntries))

enum BufferRecordStatus {NO_RECORD, READ_RECORD, WRITE_RECORD};

/*
 * Class ASIM_BUFFER
 *
 * Base class for buffers. Specific instances of buffers
 * should be created using ASIM_BUFTEMPLATE template.
 */
class ASIM_BUFFER_CLASS
{
    private:
        /*
         * User assigned name for the buffer.
         */
        char *name;

	/*
	 * Name of peer buffer where we are connected 
	 */
	char *peerName;

	ASIM_MODULE module; 

    protected:
        /*
         * The size and delay specified for the buffer.
         * Note that the specified 'size' is not necessarily
         * the same as the number of entries that the buffer
         * can hold (i.e. all buffers can hold at least one
         * entry, but the user can specify a size of 0).
         */
	UINT32 size;
        const UINT32 delay;

	/*
	 * Roger -
	 * Introduced new field 'bw' so that the user can ask
	 * for the bandwidth of a given connection between buffers
	 * Note the difference between 'bw' and 'size'. 
	 * Typically size = bw * delay + bw
	 */
	UINT32 bw;

	/*
	 * BOBBIE -
	 * Introduced this flag so that we can specify which buffers 
	 * we want tracked for per inst. profiles.  We also need to
	 * assign an unique number to each recording buffer. 
	 */
	enum BufferRecordStatus origStatus;
	INT32 origNumber;
	enum BufferRecordStatus destStatus;
	INT32 destNumber;
        
        /*
         * True if we want to check to make sure all ready entries
         * are read from the buffer each cycle (i.e. true if
         * the buffer is not allowed to "hold" objects for multiple
         * cycles until the reader is ready for them).
         */
        const bool mustRead;

        /*
         * Number of entries that the buffer can hold.
         */
        UINT32 numEntries;
        UINT32 bufEntries;

        /*
         *
         * Circular array holding buffer entries,
         * and a parallel array holding the cycle
         * when each entry is read-ready.
         */
        // <template_type> *buffer; // this will be handled in template
        UINT64 *ready;

        /*
         * Pointers to the next entry to read and write.
         * 'rPtr' points to the entry to read next. If
         * 'rPtr' == 'wPtr', the buffer is empty. 'wPtr'
         * points to the entry to write. If 'wPtr'+1
         * points to 'rPtr', then the buffer is full.
         */
        UINT32 rPtr, wPtr;
        UINT32 peekPtr;

        /*
         * Number of buffer entries currently free, i.e.
         * number of writes we can do to the buffer (without
         * intervening reads) before it is full.
         */
        UINT32 free;

	// Stats
	UINT64 totRead; 	// number of times buffer is read
	UINT64 successRead;	// number of times buffer returns an entry

	void RegisterStats(ASIM_MODULE m, char *n)
	{
	    if (registerBufferStats == false)
	    {
		return;
	    }

            string name;
	    char *buf1, *buf2;

	    name = string(n) + "_TOTREAD";
            buf1 = strdup(name.c_str());

	    name = string(n) + "_SUCCESSREAD";
            buf2 = strdup(name.c_str());
	    
	    m->RegisterState(&totRead, buf1,
                "Total number of times buffer is read"); 
	    m->RegisterState(&successRead, buf2,
                "Total number of times buffer returns value");
	}


    public:
        ASIM_BUFFER_CLASS (char *n, ASIM_MODULE m, UINT32 s, const UINT32 d, const bool mr)
            : name(n), module(m), size(s), delay(d), mustRead(mr)
        {
            numEntries = ((size == 0) ? 1 : size);
            bufEntries = numEntries + 1;
            ready = new UINT64[bufEntries];
            rPtr = wPtr = 0;
            peekPtr = 0;
            free = numEntries;	    

            RegisterStats(m, n); 
        }
        
        /*
         * Accessors...
         */
        char *Name (void) { return(name); }
        char *PeerName(void) const { return peerName; }
	ASIM_MODULE Module (void) { return module; }
        
	/*
	 * Modifiers
	 */
	void SetPeerName(char *n) { peerName = n; }


	/*
	 * Return Latency of the buffer
	 */
	UINT32 GetLat (void) const { return delay; }

	/*
	 * Return BW of the buffer
	 */
	UINT32 GetBw (void) const 
        { 
#ifdef ASIM_ENABLE_ASSERTION
            if ( bw == 0 )
            {
                ASIMERROR("Cycle " << cycle << ": Buffer " << Name()
                    << " has BW=0" << endl);
            }
#endif
            return bw;
        }

	void SetBw (UINT32 b) 
	{ 
	 ASSERTX(b > 0);
	 bw = b;
	}



	void PrintInfo(void) 
	{
	    if (debugOn)
	    {
		cout << fmt("  ", "30", Name(), " :")
                     << " <size " << size << ", bw " << bw
                     << ", lat " << delay << "> --> peer " << PeerName()
                     << endl;
	    }
	}

        /*
         * Return the number of free entries in the buffer,
         * i.e. the number of entries we can write before the
         * buffer becomes full.
         */
        UINT32 Free (void) const {
#ifdef ASIM_ENABLE_ASSERTIONS
            UINT32 used = ((wPtr >= rPtr) ? (wPtr-rPtr) :
                                            (numEntries-(rPtr-wPtr-1)));
            ASSERTX(used <= numEntries);
            ASSERT(free == (numEntries - used), "free count incorrect.\n");
#endif
            return(free);
        }
	
        /*
	 * Check to see if there is anything in the buffer that
	 * could be read out this cycle.
         */
        bool SomethingToRead (UINT64 cycle) {

            if ((rPtr == wPtr) || (ready[rPtr] > cycle))
	    {
                return(false);
	    }

            ASSERTX(free < numEntries);
	    
            return(true);
        }

        /*
         * Check buffer to ensure invariants hold. Typically this
         * is called by the reader of the buffer to ensure that
         * the buffer is in a valid state after the reader has
         * finished using the buffer. Invarients described above.
         */
        void CheckBuf (UINT64 cycle) {
#ifdef ASIM_ENABLE_ASSERTIONS
            if (mustRead) {
                for (UINT32 i = rPtr; i != wPtr; i = BUFPTR_INCR_1(i))
                    if (ready[i] <= cycle)
                    {
                        ASIMERROR("Cycle " << cycle << ": Buffer " << Name()
                            << " has unread entry ready in cycle " << ready[i]
                            << endl);
                    }
            }
#endif
        }


        /*
         * defining required interfaces with pure virtual functions
         */
        virtual ASIM_BUFFER Combine (ASIM_BUFFER buf) =0;
        virtual ASIM_BUFFER Combine (ASIM_BUFFER buf, const UINT32 bw, const UINT32 lat) =0;

};


/*
 * Template ASIM_BUFTEMPLATE
 *
 * Template to create buffers. When creating a buffer the
 * user specifies a size and a delay for the buffer. The
 * size is the number of entries the buffer can hold, and
 * the delay is the number of cycles from when an entry
 * is written into the buffer until the entry can be
 * read from the buffer.
 * When buffers are combined, the new buffer has size
 * equal to the sum of the combined buffers sizes, and
 * delay equal to the sum of the combined buffers delays.
 * You can specify size=0... if the buffer is not combined
 * then it will have size 1.
 *
 * B - (class) type of object held by the buffer
 */
template<class B> class ASIM_BUFTEMPLATE :
                  public ASIM_BUFFER_CLASS {
    private:
        /*
         * Circular array holding buffer entries;
         * This parallels the "ready" array holding the cycle
         * when each entry is read-ready.
         */
        B *buffer;

    public:
        ASIM_BUFTEMPLATE(ASIM_MODULE m, char *n, UINT32 s = 0, const UINT32 d = 0, 
			 const bool mr = false)
	    : ASIM_BUFFER_CLASS(n, m, s, d, mr)
	    {
		buffer = new B[bufEntries];
	    }

        /*
         * Attempt to read this buffer. If successful, return true
         * and the value read. Remove the read value from the buffer.
         * A read fails if the buffer is empty or if the head
         * entry is not ready to be read.
         */
        bool Read (B *v, UINT64 cycle) {
	    totRead++;

            if ((rPtr == wPtr) || (ready[rPtr] > cycle))
	    {
                return(false);
	    }

            ASSERTX(free < numEntries);

	    //	    B dummy;
            *v = buffer[rPtr];
	    //	    buffer[rPtr] = dummy;
	    // TO DO: NULL out buffer[rPtr] when read an element out
	    // of an mm ptr
            rPtr = BUFPTR_INCR_1(rPtr);
            free++;	
	    successRead++; 

            return(true);
        }

	/*
	 * Is the Buffer full ?
	 */
        bool Full (B v, UINT64 cycle) {
         UINT32 nw = BUFPTR_INCR_1(wPtr);

         if (nw == rPtr) return true;
	 else return false;
	}
	
        /*
         * Attempt to write to this buffer. If successful, return true;
         * otherwise return false, indicating the buffer is full.
         */
        bool Write (B v, UINT64 cycle) {
            UINT32 nw = BUFPTR_INCR_1(wPtr);
            if (nw == rPtr) {
	      cout << "ERROR: FAILURE: Cycle " << cycle
                   << ": Write to Buffer " << Name()
                   << " failed: buffer is full!" << endl;
	      exit(-1);
	      return (false); // not reached
	    }
            ASSERTX(free > 0);

            buffer[wPtr] = v;
            ready[wPtr] = cycle + delay;

            wPtr = nw;
            free--;
            return(true);
        }

        /*
         * Peek into this buffer. Iterate over the elements that can be read
         * out of the buffer by time "cycle". This does not change the state
         * of the buffer.
         * The iteration is initialized by a call to PeekReset().
         * The iteration is performed by successive calls to PeekNext().
         * If no more entries are available for being read by the specified
         * cycle, PeekNext() returns false.
         */
        void PeekReset (void) {
            peekPtr = rPtr;
        }
            
        bool PeekNext (B *v, UINT64 cycle) {
            if ((peekPtr == wPtr) || (ready[peekPtr] > cycle))
	    {
                return(false);
	    }

            *v = buffer[peekPtr];
            peekPtr = BUFPTR_INCR_1(peekPtr);

            return(true);
        }

        /*
         * Combine 'this' buffer with 'buf' and return
         * a new buffer to represent both 'this' and 'buf'.
         */
        ASIM_BUFFER Combine (ASIM_BUFFER buf) {
            //
            // Create a new buffer whose size is the sum of the
            // sizes of the combining buffers.  The recording status
	    // of the buffer is a function of the recording status of
	    // the two merged buffers.  If one of the merged buffers
	    // has record turned on, then the combined buffer does, also. 
            ASIM_BUFTEMPLATE<B> *tbuf = (ASIM_BUFTEMPLATE<B> *)buf;
	    ASIM_BUFFER pb = new ASIM_BUFTEMPLATE<B>(Module(), Name(), 
						     size > tbuf->size ? size : tbuf->size,
						     delay+tbuf->delay, false /* read */);

	    pb->SetPeerName(tbuf->Name());

	    return pb;
        }

        /*
         * Combine 'this' buffer with 'buf' and return
         * a new buffer to represent both 'this' and 'buf' that has 
	 * bandwidth 'bw' and latency 'lat'.
         */
        ASIM_BUFFER Combine (ASIM_BUFFER buf, const UINT32 bw, const UINT32 lat) {
            ASIM_BUFTEMPLATE<B> *tbuf = (ASIM_BUFTEMPLATE<B> *)buf;
	    ASIM_BUFFER pb = new ASIM_BUFTEMPLATE<B>(Module(), Name(), 
						     (bw * lat) + bw, lat, false /* read */);
	    

	    pb->SetPeerName(tbuf->Name());
	    pb->SetBw(bw);
	    
	    return pb;
        }
};

//
// This very simple class is designed to be used in conjunction with the
// ASIM_BUFTEMPLATE class. In general, when two boxes are interchanging
// objects of type <OBJ> through a buffer, it is very common that you will
// need some sort of ACK mechanism between the boxes. If this is your case,
// you can use ASIM_ACKMSG to quickly create a class that contains a
// <boolan, OBJ *> pair; i.e., a class that indicates which object you are
// ACKing/NACKing, and a boolean to indicate whether this is an ACK or a
// NACK. 
// 
template<class T> class ASIM_ACKMSG 
{
 private:
 	bool			ack;
 	T			obj;

 public:

	// constructors
 	ASIM_ACKMSG() 			{ ack = false; obj = NULL; }
	ASIM_ACKMSG(bool a, T o)	{ ASSERTX(o != NULL); ack = a; obj = o; }

	void	SetAck(bool a)		{ ack = a; }
	void	SetObject(T o)		{ obj = o; }

	bool	GetAckValue()		{ ASSERTX(obj != NULL); return ack; }
	T	GetObject()		{ ASSERTX(obj != NULL); return obj; }

};

#endif /* _BUFFER_ */
