/*
 * Copyright (C) 2004-2006 Intel Corporation
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
 * @author Brian Morris
 * @brief
 */

#ifndef _CACHE_DYN_H
#define _CACHE_DYN_H

// generic
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

// ASIM core
#include "asim/cache.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
////
//// This is a re-work of cache.h to enable dynamic knobs for number of ways, lines,
//// objects per line, and replacement policy.  See cache.h for further documentation.
////
//// Caveats:  
////   - INFO class not implemented
////   - Data type T hard coded to UINT64
////


typedef enum 
{ 
    VP_LRUReplacement,
    VP_PseudoLRUReplacement, 
    VP_RandomReplacement, 
    VP_RandomNotMRUReplacement, 
    VP_MaxReplacement
} VICTIM_POLICY;

class line_state_dynamic
{
    
  private:
    UINT64		tag;
    UINT8		way;
    LINE_STATUS	        status;
    bool		*valid;
    bool		*dirty;
    UINT32              ownerId;
    UINT32              NumObjectsPerLine;
    
    // Stats
    PUBLIC_OBJ_INIT(UINT32, Accesses, 0);
    PUBLIC_OBJ_INIT(UINT64, AccumDistance, 0);
    PUBLIC_OBJ_INIT(UINT64, PreviousCycle, 0);    
    
  public:
    line_state_dynamic(UINT32 nObjectsPerLine) : status(S_INVALID) 
    {
        NumObjectsPerLine = nObjectsPerLine;
        valid = new bool[NumObjectsPerLine];
        dirty = new bool[NumObjectsPerLine];
    
    }

    line_state_dynamic(UINT32 nObjectsPerLine, const LINE_STATUS new_status, const bool new_dirty, const UINT32 owner_id = UINT32_MAX)
        : tag(0), way(0), status(new_status), ownerId(owner_id)
    {
        NumObjectsPerLine = nObjectsPerLine;
        valid = new bool[NumObjectsPerLine];
        dirty = new bool[NumObjectsPerLine];

        for (UINT i=0; i<NumObjectsPerLine; i++)
        {
            valid[i] = true;
            dirty[i] = new_dirty;
        }
    }

    line_state_dynamic(const line_state_dynamic* const copy)
        : status(S_INVALID) // need this because of the "if" in SetStatus()
    {
        ASSERTX(copy);
        NumObjectsPerLine = copy->NumObjectsPerLine;
        valid = new bool[NumObjectsPerLine];
        dirty = new bool[NumObjectsPerLine];

        SetTag(copy->tag);
        SetWay(copy->way);
        SetStatus(copy->status);
        for (UINT i=0; i<NumObjectsPerLine; i++)
        {
            valid[i] = copy->valid[i];
            dirty[i] = copy->dirty[i];
        } 
        SetOwnerId(copy->ownerId);
    }

    UINT64      GetTag() 		{ return tag; };
    LINE_STATUS	GetStatus()		{ return status; };
    UINT8	GetWay()		{ return way; };
    bool	GetValidBit(UINT32 i)   
    { 
        ASSERTX(i < NumObjectsPerLine); 
        return valid[i]; 
    };
    
    bool	GetDirtyBit(UINT32 i)   
    { 
        ASSERTX(i < NumObjectsPerLine); 
        return dirty[i]; 
    };
    
    UINT32      GetOwnerId()            { return ownerId; };
    
    void	SetTag(UINT64 t)	{ tag = t; };
    void	SetStatus(LINE_STATUS s){ if (status != S_PERFECT) status = s; }; 
    void	SetWay(UINT32 i)	{ ASSERTX(i < 256); way = i; }
    void	SetValidBit(UINT32 i)	
    { 
        ASSERTX(i < NumObjectsPerLine); 
        valid[i] = true; 
    };
    
    void	SetDirtyBit(UINT32 i)	
    { 
        ASSERTX(i < NumObjectsPerLine); 
        dirty[i] = true; 
    };

    void        SetOwnerId(UINT32 i)     
    { 
        ownerId = i; 
    };

    void	Clear() 
    { 
        tag = 0xdeadbeef; 
        status = S_INVALID; 
        for (UINT32 i = 0; i < NumObjectsPerLine; i++) 
        {
            valid[i] = false;
            dirty[i] = false; 
        }
        ownerId = UINT32(-1);  
    }

    void	Dump()
    {
        cout << "\ttag=0x" << fmt_x(tag)
             << ", way=" << (UINT32)way << ", status=" << LINE_STATUS_STRINGS[status];
        cout << ", valid=0b";
        for (UINT32 i = 0; i < NumObjectsPerLine; i++ ) 
        {
            cout << (valid[i] ? 1 : 0); 
        }
        cout << ", dirty=0b";
        for (UINT32 i = 0; i < NumObjectsPerLine; i++ ) 
        {
            cout << (dirty[i] ? 1 : 0); 
        }
        cout << ", ownerId=" << ownerId;
        cout << endl;
    }
    
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////
////
////     LRU INFO MANAGER CLASS
////
////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// The way we implement the lru_info_dynamic for a particular group of cache lines is as
// follows We have an array used to implement a linked list. Each position in
// the array represents a particular way of the cache. I.e., position 4 in the
// array represents set 4. All sets are linked and, as the upper level (the
// dyn_cache_class) touches data, it invokes the routines makeMRU() and
// makeLRU(). Most likely, in a normal cache, as the processor touches cache
// lines, you make those lines the most-recently-used of their set. At any point
// in time, the user can ask for both the first element of this list (the MRU
// line) and for the last element of the list (the LRU line).
//
class lru_info_dynamic
{
  private: 
    struct linklist_s {
        INT8		next;
        INT8		prev;
    } *linklist;

    UINT8		mru;
    UINT8		lru;
    
  public:

    // Constructor
    lru_info_dynamic(UINT8 nWays)
    {
        UINT8 i;
        NumWays = nWays;

        linklist = new linklist_s[NumWays];

        // We set a doubly linked list of all elements in the chain array
        for (i = 0; i < NumWays; i++ ) 
        {
            linklist[i].prev = (i == 0)            ? 0xff : (i-1);
            linklist[i].next = (i == (NumWays -1)) ? 0xff : (i+1);
        }

        // Now, by pure convention, we declare the MRU set, the one in position 0
        // and we declare the LRU set the one in position NumWays -1.
        mru = 0;
        lru = NumWays - 1;
    }

    UINT8               NumWays;
    
    // Make a particular way, the MRU (most-recently-used) way
    void makeMRU(UINT8 w)
    {
        ASSERTX(w < NumWays);

        // Fast check: if 'w' is already the MRU, nothing to be done
        if ( mru == w ) 
        {
            return;
        }

        // Step number one: unlink element 'w' from its current position in the doubly
        // linked list (and connect its neighbours together)
        if ( linklist[w].prev != -1 ) 
        {
            linklist[linklist[w].prev].next = linklist[w].next;
        }

        if ( linklist[w].next != -1 ) 
        {
            linklist[linklist[w].next].prev = linklist[w].prev;
        }

        // Step two; if element 'w' happened to be the LRU element, then update LRU
        if ( lru == w ) 
        {
            lru = linklist[w].prev;
        }

        // Step three: move element 'w' to the head of the list (he is MRU).
        linklist[w].prev = -1;
        linklist[w].next = mru;
        linklist[mru].prev = w;

        // Make 'w' the MRU element.
        mru = w;
    } 

    // Make a particular way, the LRU (least-recently-used) way
    void makeLRU(UINT8 w)
    {
        ASSERTX(w < NumWays);

        // Fast check: if 'w' is already the LRU, nothing to be done
        if ( lru == w ) 
        {
            return;
        }

        // Step number one: unlink element 'w' from its current position in the doubly
        // linked list (and connect its neighbours together)
        if ( linklist[w].prev != -1 ) 
        {
            linklist[linklist[w].prev].next = linklist[w].next;
        }

        if ( linklist[w].next != -1 ) 
        {
            linklist[linklist[w].next].prev = linklist[w].prev;
        }

        // Step two; if element 'w' happened to be the MRU element, then update MRU
        if ( mru == w ) 
        {
            mru = linklist[w].next;
        }

        // Step three: move element 'w' to the tail of the list (he is LRU).
        linklist[w].next = -1;
        linklist[w].prev = lru;
        linklist[lru].next = w;

        // Make 'w' the LRU element.
        lru = w;
    } 

    // Get The MRU and LRU ways
    UINT8	getMRU() { return mru; };
    UINT8	getLRU() { return lru; };
    UINT8	getPseudoLRU() 
    {
        cerr << "ERROR: pseudoLRU not implemented" <<  endl;
        ASSERTX(false);
        return(0);
    };
    UINT8       getRandom() { return random()%NumWays; };
    
    // Debugging
    void Dump()
    {
        INT8 p,i;

        cout << "MRU ->";
        for (i = 0, p = mru; i < NumWays; i++, p = linklist[p].next ) 
        {
            ASSERTX(p != -1);
            cout << " " << p << " ->";
        }
        cout << endl;
    }

};

class pseudo_lru_info_dynamic : public lru_info_dynamic
{
    typedef UINT16 PseudoLRUMaskType;
  private:
    PseudoLRUMaskType pseudoLRU;

    PseudoLRUMaskType *mask1s; 
    PseudoLRUMaskType *mask0s;

    PseudoLRUMaskType *mask1sFind;
    PseudoLRUMaskType *mask0sFind;

  public:
    pseudo_lru_info_dynamic(UINT8 NumWays) :
        lru_info_dynamic(NumWays),
        pseudoLRU(0)
    {
        mask1s = new PseudoLRUMaskType[NumWays]; 
        mask0s = new PseudoLRUMaskType[NumWays];

        mask1sFind = new PseudoLRUMaskType[NumWays];
        mask0sFind = new PseudoLRUMaskType[NumWays];
    }

    UINT8   getPseudoLRU() 
    {
        int i;

        for (i=0; i<NumWays ; ++i)
        {
            if (((pseudoLRU&mask1sFind[i])==0) && ((pseudoLRU|mask0sFind[i])==PseudoLRUMaskType(-1)))
            {
                return (i);
            }
        }
        cerr << "ERROR: correct the masks. pseudoLRU: " << UINT32(pseudoLRU) <<  endl;
        ASSERTX(false);
        return(NumWays);
    };

    // Make a particular way, the MRU (most-recently-used) way
    void	makeMRU(UINT8 way)
    {
        ASSERTX(way < NumWays);

        lru_info_dynamic::makeMRU(way);

        pseudoLRU = pseudoLRU | mask1s[way];
        pseudoLRU = pseudoLRU & mask0s[way];
    }
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////
////    REPLACEMENT POLICIES: replacement part of the generic cache class
////  
////
////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class VictimPolicy
{
    public:
        //
        // As its name indicates, contains all the LRU data structures for each set in the cache
        //
        typedef lru_info_dynamic		lruInfo;
        UINT8           NumWays; 
        UINT32          NumLinesPerWay;
        VICTIM_POLICY   Policy;

        VictimPolicy(UINT32 nLinesPerWay, UINT8 nWays, VICTIM_POLICY vPolicy) 
        {
            NumLinesPerWay = nLinesPerWay;
            NumWays = nWays;
            Policy = vPolicy;
            LruArray = new lruInfo[NumLinesPerWay](NumWays) ;
            switch(Policy) 
            {
                case VP_LRUReplacement:
                case VP_RandomReplacement:
                case VP_RandomNotMRUReplacement:
                    LruArray = new lru_info_dynamic[NumLinesPerWay](NumWays) ;
                    break;
                case VP_PseudoLRUReplacement:
                    LruArray = new pseudo_lru_info_dynamic[NumLinesPerWay](NumWays);
                    break;
                default:
                    cerr << "ERROR: Unknown Replacement Policy"<<  endl;
                    ASSERTX(false);
                    break;
            }  
        }


        UINT32 GetVictim(UINT64 index)
        {
            UINT32 way=0;
            UINT8  mruWay;
            UINT8  ran;

            ASSERTX(index<NumLinesPerWay);
            // Get the LRU way
            switch(Policy) {
                case VP_LRUReplacement:
                    ASSERTX(index<NumLinesPerWay);
                    way = LruArray[index].getLRU();
                    break;
                case VP_PseudoLRUReplacement:
                    ASSERTX(index<NumLinesPerWay);
                    way = LruArray[index].getPseudoLRU();
                    break;
                case VP_RandomReplacement:
                    ASSERTX(index<NumLinesPerWay);
                    way = LruArray[index].getRandom();
                    break;
                case VP_RandomNotMRUReplacement:
                    ASSERTX(index<NumLinesPerWay);
                    ASSERTX(NumWays>1);
                    // Get the MRU
                    mruWay = LruArray[index].getMRU();
                    // Get a random number between 0 and NumWays-1
                    ran = random()%(NumWays-1);
                    // Get random but not MRU
                    way = (1 + mruWay + ran)%NumWays;
                    break;
                default:
                    cerr << "ERROR: Unknown Replacement Policy"<<  endl;
                    ASSERTX(false);
                    break;
            }

            return way;
        }

                    protected:
        lruInfo *LruArray;
};




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////
////
////  GENERIC CACHE DATA TYPE
////
////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class dyn_cache_class:public VictimPolicy
{
  public:
    //typedef line_state_dynamic<NumObjectsPerLine, INFO> lineState;
    typedef line_state_dynamic lineState;

    UINT32 NumObjectsPerLine; 
    bool   WithData;

  private:

    // WARNING! Each cache instance has its own random state to guarantee
    // that the warm-up phase produces the same result (i.e. lockstepped processors).
    #define CACHE_RANDOM_STATE_LENGTH 128
    
    static UINT32 DEFAULT_CACHE_RANDOM_SEED;
    UINT32 random_state[CACHE_RANDOM_STATE_LENGTH / 4];  
  
    // What percent of this cache is warm (cold misses are turned into hits)? 0-100% are legal values.
    const UINT32 warmPercent;
    const UINT64 warmFactor;  ///< warmFactor = warmPercent*RAND_MAX
    const LINE_STATUS initialWarmedState;
    //
    // Tag array holding the contents of the cache and its state.
    //
    //lineState	TagArray[NumLinesPerWay][NumWays];
    lineState	**TagArray;

    //
    // This array reduces to almost nothing if the user sets 'WithData' to
    // FALSE. Otherwise, this array is used to hold the REAL data that would be in
    // the real cache. At this point, our feeders do not still support this
    // ability, but this class is ready to accept data as soon as the feeders
    // become ready
    //
    //T   DataArray[WithData ? NumLinesPerWay : 1][WithData ? NumWays : 1][NumObjectsPerLine];
    UINT32   ***DataArray;

    //
    // Shift Amounts useful for extracting index and tag information from an address
    //
    UINT64	IndexMask;
    UINT64	PosMask;
    UINT64	ClassicalTagMask;
    UINT32	ClassicalIndexShift;
    UINT64	ShiftedTagMask;
    UINT32	ShiftedIndexShift;

    inline INT32 FindWay(UINT64 index, UINT64 tag, UINT32 warm_owner = UINT32(-1))
    {
        UINT32 i;
        vector<UINT32> warm;
        bool inv=false;
        INT32 return_way = -1;
        
        warm.clear();

        //cout << "FindWay on index " << fmt_x(index)
        //     << " tag " << fmt_x(tag) << "..." << endl;

        for ( i = 0; i < NumWays; i++ ) 
        {
            //cout << "\tTagArray[" << fmt_x(index)
            //     << "][" << fmt_x(i)
            //     << "].GetTag " << TagArray[index][i].GetTag() << endl;
            if ( TagArray[index][i].GetTag() == tag ) 
            {
                if(TagArray[index][i].GetStatus() == S_INVALID)
                {
                    inv=true;
                }
                else
                {
                    ASSERTX(return_way == -1);
                    return_way = i;
                }
            }
            else if ( TagArray[index][i].GetStatus() == S_WARM )
            {
                warm.push_back(i);
            }
        }
        if (return_way != -1)
        {
            return return_way;
        }

        if(!inv)
        {
            if ( warm.size() > 0 )
            {
                
                // Set the current cache random state and save the existing one
                char *tmp_state = setstate((char*)random_state);
                
                UINT64 rand_factor = (warmPercent == 100) ? 0 : UINT64(random()) * 100;
                //          cout << "warmFactor=" << warmFactor << ", rand_factor=" << rand_factor;
                
                // Randomize the selected warmed way
                UINT64 warm_way = warm[UINT64(random()) % warm.size()];            
                ASSERTX(TagArray[index][warm_way].GetStatus() == S_WARM);                

                if ( warmFactor > rand_factor )
                {
                    //              cout << " -- WARM!" << endl;

                    TagArray[index][warm_way].SetTag(tag);
                    TagArray[index][warm_way].SetStatus(initialWarmedState);
                    TagArray[index][warm_way].SetOwnerId(warm_owner);
                    for (UINT32 j=0; j<NumObjectsPerLine; j++)
                    {
                        TagArray[index][warm_way].SetValidBit(j);
                    }
                    return warm_way;
                }
                else
                {
                    //              cout << " -- COLD!" << endl;

                    TagArray[index][warm_way].SetTag(tag);
                    TagArray[index][warm_way].SetStatus(S_INVALID);
                    return -1;
                }
                
                // Restore the previous random state
                setstate(tmp_state);                
                
            }
        }

        //    cout << "Miss at index=0x" << fmt_x(index) << ", tag=0x" << fmt_x(tag) << ". :-(" << endl;
        return -1;
    }

  public:

    // Constructor
    //dyn_cache_class(UINT8 nWays, UINT32 nLinesPerWay, UINT32 nObjectsPerLine, 
    //        const UINT32 warm_percent = 0, const LINE_STATUS initial_warmed_state = S_SHARED);
    dyn_cache_class(UINT8 nWays, UINT32 nLinesPerWay, UINT32 nObjectsPerLine, VICTIM_POLICY policy,
            const UINT32 warm_percent, const LINE_STATUS initial_warmed_state, const INT32 random_seed = -1)
        : VictimPolicy(nLinesPerWay, nWays, policy), warmPercent(warm_percent), warmFactor(UINT64(warm_percent) * RAND_MAX), initialWarmedState(initial_warmed_state) 
    {
        UINT32 i,j;

        //  NumWays = nWays;
        //  NumLinesPerWay = nLinesPerWay;
        NumObjectsPerLine = nObjectsPerLine;
        WithData = false;

        UINT32 KiloObjects = ( NumWays *  NumLinesPerWay * NumObjectsPerLine) / 1024;

        TagArray = new(lineState*[NumLinesPerWay]);
        for ( i = 0; i < NumLinesPerWay; i++ ) 
        {
            TagArray[i] = new lineState[NumWays](NumObjectsPerLine);
        }

        VERIFYX(isPowerOf2(NumObjectsPerLine));
        IndexMask = CEIL_POW2(NumLinesPerWay) - 1;
        PosMask = NumObjectsPerLine -1;
        ClassicalIndexShift = ilog2(NumObjectsPerLine) + 3;
        ClassicalTagMask = ~(UINT64)((CEIL_POW2(NumLinesPerWay) * NumObjectsPerLine * 8) - 1);
        ShiftedIndexShift = ilog2(NumObjectsPerLine);
        ShiftedTagMask = ~(UINT64)((CEIL_POW2(NumLinesPerWay) * NumObjectsPerLine) - 1);

        // Initialize the random state
        if(random_seed < 0)
        {
            initstate(DEFAULT_CACHE_RANDOM_SEED, (char*)random_state, CACHE_RANDOM_STATE_LENGTH);
            DEFAULT_CACHE_RANDOM_SEED++;
        }
        else
        {
            initstate(random_seed, (char*)random_state, CACHE_RANDOM_STATE_LENGTH);
        }
        
        for ( i = 0; i < NumLinesPerWay; i++ ) 
        {
            for (j = 0; j < NumWays; j++ ) 
            {
                TagArray[i][j].Clear();
                TagArray[i][j].SetWay(j);

                if (warmPercent > 0)
                {
                    TagArray[i][j].SetStatus(S_WARM);
                }
            }
        }
        TRACE(Trace_Sys, 
                cout << "You just created a "
                << warmPercent << "% warm cache with "
                << (UINT32)NumWays << " ways, "
                << NumLinesPerWay << " lines per way, "
                << NumObjectsPerLine << " objects per line = "
                << KiloObjects << " simulated KiloObjects (software variable = "
                << int(sizeof(*this)/1024) << " kilobytes)" << endl);

        TRACE(Trace_Sys,
                cout << "IndexMask           = " << fmt_x(IndexMask) << endl); 
        TRACE(Trace_Sys,
                cout << "PosMask             = " << fmt_x(PosMask) << endl); 
        TRACE(Trace_Sys,
                cout << "ClassicalIndexShift = " << ClassicalIndexShift << endl); 
        TRACE(Trace_Sys,
                cout << "ClassicalTagMask    = " << fmt_x(ClassicalTagMask) << endl); 
        TRACE(Trace_Sys,
                cout << "ShiftedIndexShift   = " << ShiftedIndexShift << endl); 
        TRACE(Trace_Sys,
                cout << "ShiftedTagMask      = " << fmt_x(ShiftedTagMask) << endl); 
    }

    // Destructor
    ~dyn_cache_class() 
    {
        for (unsigned int i = 0; i < NumLinesPerWay; i++ ) 
        {
            delete(TagArray[i]);
        }
        delete(TagArray);
    };

    // Method to set all the lines to S_INVALID
    void ClearAllLines()
    {
        
        for (UINT64 i = 0; i < NumLinesPerWay; i++ )
        {
            for (UINT64 j = 0; j < NumWays; j++ )
            {            
                TagArray[i][j].SetStatus(S_INVALID);            
            }
        }    
        
    }    
    
    //
    // Main function to check whether a given index is present or not in 
    // the cache. This routine returns a pointer to the STATE portion of the
    // data cache. Once you have a pointer to the STATE structure, you can do whatever
    // you feel like with it (change it, set dirty bits, etc...). Also, the
    // data structure returned contains the 'way' in which the cache line is stored.
    // You will need the 'way' for a lot of the remaining functions in this class.
    // If the <idx,tag> pair can not be found in the cache, the function returns 
    // a NULL pointer.
    //
    line_state_dynamic* GetLineState(UINT64 index, UINT64 tag, UINT32 warm_owner = UINT32(-1)) 
    {
        INT32 way;

        ASSERTX(index < NumLinesPerWay);

        // Do an associative search on the several ways of the cache.
        way = FindWay(index,tag,warm_owner);

        return ( way == -1 ) ? NULL : (&(TagArray[index][way]));
    };


    //
    // Get a particular way of a particular index
    //
    line_state_dynamic * GetWayLineState(UINT64 index, UINT32 way)
    {
        ASSERTX(index < NumLinesPerWay);
        ASSERTX(way < NumWays);

        return (&(TagArray[index][way]));
    } 

    //
    // Functions used to find a Victim within a given index to be replaced
    //
    line_state_dynamic * getLRUState(UINT64 index)
    {
        UINT32 way;
        // Get the LRU way
        ASSERTX(index < NumLinesPerWay);
        way = LruArray[index].getLRU();
        // Return pointer to it.
        return &(TagArray[index][way]);
    }

    line_state_dynamic *getMRUState(UINT64 index)
    {
        UINT32 way;

        // Get the MRU way
        ASSERTX(index < NumLinesPerWay);
        way = LruArray[index].getMRU();
        // Return pointer to it.
        return &(TagArray[index][way]);
    }

    line_state_dynamic *GetVictimState(UINT64 index)
    {
        UINT32 way;

        // Search for an invalid way that should be the first to be victimized
        for (UINT16 i = 0; i < NumWays; ++i)
        {
            if (TagArray[index][i].GetStatus()==S_INVALID)
            {
                return &(TagArray[index][i]);
            }
        }
        // No invalid line -> get the victim according to the selected replacement algorithm
        way = GetVictim(index);
        // Return pointer to it.
        return &(TagArray[index][way]);
    }


    //
    // Functions to update the LRU information according to your favorite scheme. 
    //
    void MakeMRU(UINT64 index, UINT32 way)
    {
        ASSERTX(index < NumLinesPerWay);
        ASSERTX(way < NumWays);
        LruArray[index].makeMRU(way);
    }

    void MakeLRU(UINT64 index, UINT32 way)
    {
        ASSERTX(index < NumLinesPerWay);
        ASSERTX(way < NumWays);
        LruArray[index].makeLRU(way);
    }



    //
    // CLASSIC Mapping Functions used to compute an <index,tag> pair to be used
    // with the previous functions. Index returns the index portion according to
    // the definition at the beginning of this file. Likewise for Tag. 'Pos'
    // returns the 'Quadword-In-Line' bits so you can determine the exact position
    // of a quadword within a line.
    // 
    inline UINT64 Index(UINT64 addr) const
    {
        addr = (addr >> ClassicalIndexShift) & IndexMask;
        ASSERTX(addr < NumLinesPerWay);

        return addr;
    }

    UINT64 Tag(UINT64 addr) const
    {
        addr = addr & ClassicalTagMask;
        return addr;
    }  

    UINT64 Pos(UINT64 addr) const
    {
        addr = (addr >> 3) & PosMask;
        return addr;
    }  

    UINT64 Original(UINT64 index, UINT64 tag) const // rebuilds the original address from an <index,tag>
    {
        ASSERTX((tag == 0xdeadbeef) || (tag & (index << ClassicalIndexShift)) == 0);
        return (tag | (index << ClassicalIndexShift));
    }


    //
    // SHIFTED Mapping Functions used to compute an <index,tag> pair to be used
    // with the previous functions. Index returns the index portion according to
    // the definition at the beginning of this file. Likewise for Tag. 'Pos'
    // returns the 'Quadword-In-Line' bits so you can determine the exact position
    // of a quadword within a line.
    // 
    inline UINT64 IndexShifted(UINT64 addr)
    {
        addr = (addr >> ShiftedIndexShift) & IndexMask;
        ASSERTX(addr < NumLinesPerWay);
        return addr;
    }

    UINT64 TagShifted(UINT64 addr)
    {
        addr = addr & ShiftedTagMask;
        return addr;
    }

    UINT64 PosShifted(UINT64 addr)
    {
        addr = addr & PosMask;
        return addr;
    }

    UINT64 OriginalShifted(UINT64 index, UINT64 tag) // rebuilds the original address from an <index,tag>
    {
        ASSERTX ((tag == 0xdeadbeef) || (tag & (index << ShiftedIndexShift)) == 0);
        return (tag | (index << ShiftedIndexShift));
    }


    //
    //
    // In case you are storing real data in your cache (i.e, WithData ==
    // true), you will need the following functions to read and write the
    // data information. Notice that you HAVE TO PROVIDE A SPECIFIC way
    // where the data has to be placed (you obtain the 'way' from the
    // GetLineState() call).  For your own protection, 'SetLineData' will
    // not allow overwriting a cache line that is NOT in S_EXCLUSIVE_DIRTY
    // state. So, whenever writing to a cache line, you first have to update
    // its state and then perform the physical write.
    // 
    // The last 'SetLineData' will only activite the dirty bit for the particular object you are writing.
    //

    // Copies all data stored in a cache line into the array 'data'
    void GetLineData(UINT64 index, UINT32 way, UINT32 *data, UINT32 NumObjectsPerLine)
    {
        if ( WithData == false ) return;

        ASSERTX(index < NumLinesPerWay);
        ASSERTX(way < NumWays);

        for (UINT32 i = 0; i < NumObjectsPerLine; i++ ) {
            data[i] =  DataArray[index][way][i];
        }
    }

    // Copies a single data item stored in a cache line into the variable 'data'
    void GetLineData(UINT64 index, UINT32 way, UINT32 ObjectInLine, UINT32 *data)
    {
        if ( WithData == false ) return;

        ASSERTX(index < NumLinesPerWay);
        ASSERTX(way < NumWays);
        ASSERTX(ObjectInLine < NumObjectsPerLine);

        *data = DataArray[index][way][ObjectInLine];
    }

    void SetLineData(UINT64 index, UINT32 way, UINT32 *data, UINT32 NumObjectsPerLine)
    {
        if ( WithData == false ) return;

        // Compute index
        ASSERTX(index < NumLinesPerWay);
        ASSERTX(way < NumWays);

        // Check we are in the appropriate state
        // ASSERT((TagArray[index][way].GetStatus() == S_EXCLUSIVE_DIRTY)
        //      ||(TagArray[index][way].GetStatus() == S_EXCLUSIVE_CLEAN),
        //      "You can not use SetLineData on a line that is not S_EXCLUSIVE_*\n");

        // copy data into the cache
        for (UINT32 i = 0; i < NumObjectsPerLine; i++ ) {
            ASSERT(TagArray[index][way].GetValidBit(i) == true,"ValidBit(i) must be true before writing data into the cache!\n");
            TagArray[index][way].SetDirtyBit(i);
            DataArray[index][way][i] = data[i];
        }
    }

    void SetLineData(UINT64 index, UINT32 way, UINT32 ObjectInLine, UINT32 data)
    {
        if ( WithData == false ) return;

        // Compute index
        ASSERTX(index < NumLinesPerWay);
        ASSERTX(way < NumWays);
        ASSERTX(ObjectInLine < NumObjectsPerLine);

        // Check we are in the appropriate state
        // ASSERT((TagArray[index][way].GetStatus() == S_EXCLUSIVE_DIRTY)
        //      ||(TagArray[index][way].GetStatus() == S_EXCLUSIVE_CLEAN),
        //      "You can not use SetLineData on a line that is not S_EXCLUSIVE_*\n");

        // copy data item into the cache
        ASSERT(TagArray[index][way].GetValidBit(ObjectInLine) == true,"ValidBit(i) must be true before writing data into the cache!\n");
        TagArray[index][way].SetDirtyBit(ObjectInLine);
        DataArray[index][way][ObjectInLine] = data;
    } 

    //
    // Debugging routines
    //
    void Dump(UINT64 index, UINT32 way)
    {
        ASSERTX(index < NumLinesPerWay);
        ASSERTX(way < NumWays);

        //cout << "\tDump for cache line at <" << fmt_x(index)
        //     << "x,--> way " << way << ":";
        TagArray[index][way].Dump();
    }

    void DumpLRU(UINT64 index)
    {
        ASSERTX(index < NumLinesPerWay);
        cout << "Dump for index 0x" << fmt_x(index) << " :";
        LruArray[index].Dump();
    }  

    void tester(void)
    {
        LruArray[4].Dump();

        LruArray[4].makeMRU(5);
        ASSERTX(LruArray[4].getLRU() == 4);
        ASSERTX(LruArray[4].getMRU() == 5);
        LruArray[4].Dump();

        LruArray[4].makeLRU(2);
        ASSERTX(LruArray[4].getLRU() == 2);
        ASSERTX(LruArray[4].getMRU() == 5);
        LruArray[4].Dump();

        LruArray[4].makeLRU(5);
        ASSERTX(LruArray[4].getLRU() == 5);
        ASSERTX(LruArray[4].getMRU() == 0);
        LruArray[4].Dump();

        LruArray[4].makeMRU(3);
        ASSERTX(LruArray[4].getLRU() == 5);
        ASSERTX(LruArray[4].getMRU() == 3);
        LruArray[4].Dump();

        LruArray[4].makeMRU(2);
        ASSERTX(LruArray[4].getLRU() == 5);
        ASSERTX(LruArray[4].getMRU() == 2);
        LruArray[4].Dump();
    }
};

UINT32 dyn_cache_class::DEFAULT_CACHE_RANDOM_SEED = 0;

#endif // CACHE_DYN_H
