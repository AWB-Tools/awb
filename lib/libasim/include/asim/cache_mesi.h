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
 * @brief
 */

#ifndef _CACHE_MESI_H
#define _CACHE_MESI_H

// generic
#include <stdio.h>
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/utils.h"
#include "asim/ioformat.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
////
////
//// WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! :
////
////
//// This cache implementation DOES NOT KNOW/ASSUME anything about the size of the objects
//// that you are storing in the cache. In PARTICULAR, it DOES NOT ASSUME that the objects
//// being stored are QUADWORDS. This is necessary in order to support the directory cache, 
//// that holds 27-bit objects (directory entries). MOREOVER, this cache implementation does 
//// not assume any particular mapping from your addresses into cache locations. That is, 
//// this implementation DOES NOT assume the classical-typical-canonical mapping where low 
//// order bits indicate Byte-In-Quadword, then the next bits indicate Q-In-Line, then the 
//// next bits are the Index into the cache and whatever is left makes up the tag of the 
//// cache line.
////
//// HOWEVER, and before you decide NOT to use this data type :-), functions are provided that
//// will help you do all the above very easily (see below).
////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
////
////
////
//// PURPOSE AND GOALS OF cache_mesi.h
//// ---------------------------------
////
//// The purpose of this data type is to handle for you all the nasty details of managing a general
//// cache. It offers a generic implementation of a cache with the following major parameters:
//// 
////     NumWays:            1 for direct mapped, NumWays > 1 for a set associative cache
////     NumLinesPerWay:     Indicates the number of cache lines in each way. 
////     NumObjectsPerLine:  Indicates the number of ``objects'' (i.e., quadwords) in each cache line.
////
//// Although most likely you will be using this template for a fairly conventional cache that
//// holds lines of quadwords (64bit objects), no assumption is made on what
////
////
//// INDEXING INTO THE CACHE
//// -----------------------
//// 
//// One of the major design decisions behind this data type is that it must be generic enough to support 
//// reasonable indexing combinations into the cache such as:
//// 
////        Virtual-Index/Virtual-Tag
////        Virtual-Index/Physical-Tag
////        Physical-Index/Physical-Tag
//// 
//// Furthermore, this code must also make NO ASSUMPTIONS on how the bits of an address are used/mangled
//// in order to construct the <index,tag> pair.
//// 
//// The consequences of these two goals are that
//// 
////  1) Functions provided here always take as input parameters an <index,tag> pair and use them
////     directly (no masking/shifting) to access the data inside the cache. For example, the
////     function used to get the state of a cache line is:
//// 
////                line_state<NumObjectsPerLine>       *GetLineState(UINT64 index, UINT64 tag);
//// 
////     The user of this function (i.e., you) must compute the index and tag he wishes based
////     on the characteristics of his cache indexing and mapping functions.
//// 
////  2) Several groups of functions are provided to easily compute <index,tag> pairs based on 
////     the most common mapping algorithms. In particular, we offer two sets of functions,
////     called 'Classical' and 'Shifted' to save you from the pains of doing masking and
////     shifting to index the cache. Thus, if you are using a typical cache configuration,
////     you would access the information in the cache using something like:
////
////       mycache.GetLineStatus(mycache.Index(ea),mycache.Tag(ea));
////
////
//// MAPPING FUNCTIONS PROVIDED
//// --------------------------
////
//// As already mentioned above, the interface provided by 'GetLineState' is a RAW interface in
//// that it does not process at all the <index,tag> pair passed to it. The user is responsible
//// for computing the Index/Tag according to his cache characteristics.
////
//// To avoid having each of our ASIM users re-thinking the shifting and masking necessary to
//// compute the Index/Tag pairs (always a buggy process), we provide two sets of functions
//// for the begginner user of this data type.
////
//// The two sets are:
//// 
////   Classical: Assumes a liner mapping of addresses where the 3 low order bits indicate
////              the 'Byte-In-Quadword', the next Q bits select the 'Quadword-In-Line', 
////              the next I bits select the index within a way  and whatever is left is 
////              considered part of the tag:
//// 
////                                                     I        Q    3
////                          +----------------------+----------+----+----+
////                          |      Tag             |  Index   |QinL|BinQ|
////                          +----------------------+----------+----+----+
//// 
////               where I = log2(NumLinesPerWay)
////                     Q = log2(NumObjectsPerLine)
//// 
//// 
//// 
////   Shifted: Used when a cache is NOT holding quadwords (for example, in the directory cache).
////            Assumes that the user has already shifted the input address according to the size
////            of the objects stored in the cache and, therefore, the functions provided should
////            not be shifting the addresses by '3' -- to account for quadwords. As the figure
////            shows, there are no bits corresponding to 'ByteInQuadword'
////        
////                                                     I        O   
////                          +----------------------+----------+----+
////                          |      Tag             |  Index   |OinL|
////                          +----------------------+----------+----+
//// 
////               where I = log2(NumLinesPerWay)
////                     O = log2(NumObjectsPerLine)
//// 
//// 
////
//// Note that you ARE NOT FORCED TO USE THE MAPPING FUNCTIONS PROVIDED. In fact, you can use any 
//// other sophisticated mapping/indexing function you need, since the code provided here always 
//// takes as parameters an <idx,tag> pair, and uses them directly (no masking/shifting at all) to access 
//// the data within the cache.
////
//// VIRTUAL VS. PHYSICAL INDEXING AND TAGGING
//// -----------------------------------------
////
//// This cache code is generic enough to support reasonable combinations 
//// of indexing into the cache:
////
////   Virtual-Index/Virtual-Tag
////   Virtual-Index/Physical-Tag
////   Physical-Index/Physical-Tag
////
//// Thus, when you are asking whether a given address will hit or miss 
//// in the cache, you have to provide both the index and 
//// the tag that you expect to find (i.e., the tag that must be in the 
//// cache in order to declare HIT). If you are using the cache as a 
//// Virtually-Index/Virtually-Tagged cache, then you supply the same 
//// address to the first two parameters (of course, same thing for
//// Physically-Indexed/Physically-Tagged): 
////
////     p = mycache->GetLineState(mycache->Index(vea),mycache->Tag(vea));
////
//// On the other hand, if you have a Virtually-Indexed/Physically-Tagged 
//// cache, then this code assumes that you have translated your virtual 
//// address (vea) into a physical address (pea) using some sort of DTB 
//// (either in parallel with the cache lookup or in a previous cycle) and
//// then you invoke GetLineState with the following parameters:
////
////     p = mycache->GetLineState(mycache->Index(vea),mycache->Tag(pea));
////
//// Note that what you get in return is not hit/miss (that would be too simple
//// minded, wouldn't it? :-)... you get back a pointer to the cache line STATE. 
//// With that, you can compute yourself whether it is a hit or a miss and you can also
//// generate the appropriate coherency message request (given the current state
//// of the cache).
////
////
//// THE 'WithData' PARAMETER
//// ------------------------
////
////
////
//// USAGE DIRECTIONS FOR CACHE_MESI.H
//// ---------------------------------
////
//// The idea behind this data type is that you always follow a given sequence of
//// calls to do reads/writes into the data cache.
////
//// 1) First, you always invoke 'GetLineState' with the <index,tag> you are looking for.
////    If you 'hit' in the cache, you'll get a pointer to a 'line_state' data
////    structure. Once you have the 'line_state' pointer, you can truly determine whether
////    you have a hit or not based on the status of the line (Invalid, Exclusive, Shared, etc...)
////    and you can also find out in which way of the cache the line found was stored.
////
//// 2) After you get the cache STATE, you can update it directly by using the member
////    functions of the line_state class.
////
//// 3) If you need to, you update the LRU/MRU information by invoking UpdateLRU().
////
//// 4) Finally, in case you have true data stored in the cache, you can set the
////    data for every object in the cache line
////


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////
//// CACHE LINE STATE
////
////
////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// The STATE for a cache line consists of three main parts
//
// - the physical tag of the line: a bunch of bits that identify the cache line
//
// - the status of the line: the line can be in one of a number of different states.
//                          if the line is "Invalid" the other fields should be ignored.
//
// - The cache 'way' where the line is located (always 0 for a direct-map cache).
//
// - a bit vector indicating validty of each object in the line. Note that if the line is
//   in 'Invalid' state, the contents of the bit vector are considered 'UNPREDICTABLE'. Moreover,
//   if the state of the line is NOT Invalid, then at least 1 bit in the bit vector should be set.
//
// - a bit vector indicating 'dirtyness' of each object in the line. Note that if the line is
//   in 'Invalid' state, the contents of the bit vector are considered 'UNPREDICTABLE'. Moreover,
//   if the state of the line is NOT S_EXCLUSIVE_DIRTY, then all bits SHOULD BE ZERO. Conversly,
//   if the state of the line is S_EXCLUSIVE_DIRTY then at least 1 bit in the bit vector should be set.
//


//
// Major Cache Line Coherency States
//
typedef enum { S_INVALID, S_EXCLUSIVE_CLEAN, S_EXCLUSIVE_DIRTY, S_SHARED, S_FORWARD, S_MAX_LINE_STATUS } LINE_STATUS;

static char * LINE_STATUS2STR[S_MAX_LINE_STATUS] = { "Invalid", "Exclusive", "Dirty", "Shared", "Forward" };

template<UINT32 NumObjectsPerLine> class line_state
{

  private:
	UINT64		tag;
	UINT8		way;
	LINE_STATUS	status;
	bool		valid[NumObjectsPerLine];
	bool		dirty[NumObjectsPerLine];

  public:
	UINT64		GetTag() 		{ return tag; };
	LINE_STATUS	GetStatus()		{ return status; };
	UINT8		GetWay()		{ return way; };
	bool		GetValidBit(UINT32 i)   { ASSERTX(i < NumObjectsPerLine); return valid[i]; };
	bool		GetDirtyBit(UINT32 i)   { ASSERTX(i < NumObjectsPerLine); return dirty[i]; };

	void		SetTag(UINT64 t)	{ tag = t; };
	void		SetStatus(LINE_STATUS s){ status = s; }; 
  	void		SetWay(UINT32 i)	{ ASSERTX(i < 256); way = i; }
	void		SetValidBit(UINT32 i)	{ ASSERTX(i < NumObjectsPerLine); valid[i] = true; };
	void		SetDirtyBit(UINT32 i)	{ ASSERTX(i < NumObjectsPerLine); dirty[i] = true; };
	void		Clear() { tag = 0xdeadbeef; status = S_INVALID; for (UINT32 i = 0; i < NumObjectsPerLine; i++) valid[i] = dirty[i] = false;  }
	void		Dump()
	{
	  cout << "tag 0x" << fmt_x(tag)
               << " way " << (UINT32)way << " status " << status;
	  cout << " valid 0b";
          for (UINT32 i = 0; i < NumObjectsPerLine; i++ ) {
            cout << (valid[i] ? 1 : 0); 
          }
	  cout << " dirty 0b";
          for (UINT32 i = 0; i < NumObjectsPerLine; i++ ) {
            cout << (dirty[i] ? 1 : 0); 
          }
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
// The way we implement the lru_info for a particular group of cache lines is as follows
// We have an array used to implement a linked list. Each position in the array represents
// a particular way of the cache. I.e., position 4 in the array represents set 4. All sets
// are linked and, as the upper level (the gen_cache_class) touches data, it invokes the routines
// makeMRU() and makeLRU(). Most likely, in a normal cache, as the processor touches cache
// lines, you make those lines the most-recently-used of their set. At any point in time, the user
// can ask for both the first element of this list (the MRU line) and for the last element
// of the list (the LRU line).
//
template<UINT8 NumWays> class lru_info
{
  private: 
	struct {
	 INT8		next;
	 INT8		prev;
	} linklist[NumWays];

	UINT8		mru;
	UINT8		lru;

  public:

	// Constructor
	lru_info();

	// Make a particular way, the MRU (most-recently-used) way
	void	makeMRU(UINT8 way);

	// Make a particular way, the LRU (least-recently-used) way
	void	makeLRU(UINT8 way);

	// Get The MRU and LRU ways
	UINT8	getMRU() { return mru; };
	UINT8	getLRU() { return lru; };

	// Debugging
	void	Dump();
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

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T = UINT64, bool WithData = false> class gen_cache_class 
{
  public:
  	typedef lru_info<NumWays>		lruInfo;

  private:
  	
	//
	// As its name indicates, contains all the LRU data structures for each set in the cache
	//
	lruInfo				LruArray[NumLinesPerWay];

	//
	// Tag array holding the contents of the cache and its state.
	//
	line_state<NumObjectsPerLine>	TagArray[NumLinesPerWay][NumWays];

	//
	// This array reduces to almost nothing if the user sets 'WithData' to FALE. Otherwise, this array
	// is used to hold the REAL data that would be in the real cache. At this point, our feeders do not
	// still support this ability, but this class is ready to accept data as soon as the feeders become ready
	//
	T				DataArray[WithData ? NumLinesPerWay : 1][WithData ? NumWays : 1][NumObjectsPerLine];

	//
	// Shift Amounts useful for extracting index and tag information from an address
	//
	UINT64				IndexMask;
	UINT64				PosMask;
	UINT64				ClassicalTagMask;
	UINT32				ClassicalIndexShift;
	UINT64				ShiftedTagMask;
	UINT32				ShiftedIndexShift;

  private:

  	INT32		FindWay(UINT64 index, UINT64 tag);

  public:

	// Constructor
  	gen_cache_class();

	// Destructor
  	~gen_cache_class();

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
  	line_state<NumObjectsPerLine>	*GetLineState(UINT64 index, UINT64 tag);
  	
  	//
  	// Get a particular way of a particular index
  	//
  	line_state<NumObjectsPerLine>	*GetWayLineState(UINT64 index, UINT32 way);


	//
	// Functions used to find a Victim within a given index to be replaced
	//
	line_state<NumObjectsPerLine>	*GetLRUState(UINT64 index);
	line_state<NumObjectsPerLine>	*GetMRUState(UINT64 index);

	//
	// Functions to update the LRU information according to your favorite scheme. 
	//
	void				MakeMRU(UINT64 index, UINT32 way);
	void				MakeLRU(UINT64 index, UINT32 way);


	//
	// CLASSIC Mapping Functions used to compute an <index,tag> pair to be used with the previous 
	// functions. Index returns the index portion according to the definition at the beginning of this
	// file. Likewise for Tag. 'Pos' returns the 'Quadword-In-Line' bits so you can determine the
	// exact position of a quadword within a line.
	// 
      	UINT64		Index(UINT64 addr);  
      	UINT64		Tag(UINT64 addr);
      	UINT64		Pos(UINT64 addr);
      	UINT64		Original(UINT64 index, UINT64 tag); // rebuilds the original address from an <index,tag>

	//
	// SHIFTED Mapping Functions used to compute an <index,tag> pair to be used with the previous 
	// functions. Index returns the index portion according to the definition at the beginning of this
	// file. Likewise for Tag. 'Pos' returns the 'Quadword-In-Line' bits so you can determine the
	// exact position of a quadword within a line.
	// 
      	UINT64		IndexShifted(UINT64 addr);  
      	UINT64		TagShifted(UINT64 addr);
      	UINT64		PosShifted(UINT64 addr);
      	UINT64		OriginalShifted(UINT64 index, UINT64 tag); // rebuilds the original address from an <index,tag>

	//
	//
	// In case you are storing real data in your cache (i.e, WithData == true), you will need the
	// following functions to read and write the data information. Notice that
	// you HAVE TO PROVIDE A SPECIFIC way where the data has to be placed (you obtain the 'way' from
	// the GetLineState() call).  For your own protection, 'SetLineData' will not
	// allow overwriting a cache line that is NOT in S_EXCLUSIVE_DIRTY state. So, whenever writing
	// to a cache line, you first have to update its state and then perform the physical write.
	// 
	// The last 'SetLineData' will only activite the dirty bit for the particular object you are writing.
	//
	void				GetLineData(UINT64 index, UINT32 way, T data[NumObjectsPerLine]);
	void				GetLineData(UINT64 index, UINT32 way, UINT32 ObjectInLine, T *data);
	void				SetLineData(UINT64 index, UINT32 way, T data[NumObjectsPerLine]);
	void				SetLineData(UINT64 index, UINT32 way, UINT32 ObjectInLine, T data);

	
	//
	// Debugging routines
	//
	void				Dump(UINT64 index, UINT32 way);
	void				DumpLRU(UINT64 index);
	void				tester();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::gen_cache_class(void)
{
 UINT32 i,j;
 UINT32 KiloObjects = ( NumWays *  NumLinesPerWay * NumObjectsPerLine) / 1024;

 VERIFYX(isPowerOf2(NumObjectsPerLine));
 VERIFYX(isPowerOf2(NumLinesPerWay));
 IndexMask		= NumLinesPerWay - 1;
 PosMask		= NumObjectsPerLine -1;
 ClassicalIndexShift    = ilog2(NumObjectsPerLine) + 3;
 ClassicalTagMask	= ~(UINT64)((NumLinesPerWay * NumObjectsPerLine * 8) - 1);
 ShiftedIndexShift    	= ilog2(NumObjectsPerLine);
 ShiftedTagMask		= ~(UINT64)((NumLinesPerWay * NumObjectsPerLine) - 1);


 for ( i = 0; i < NumLinesPerWay; i++ ) {
  for (j = 0; j < NumWays; j++ ) {
   TagArray[i][j].Clear();
   TagArray[i][j].SetWay(j);
  }
 }
 TRACE(Trace_Sys, 
     cout << "You just created a cache with "
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::~gen_cache_class()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the basic function that looks for a cache line. The ones exported to the user
// are simply wrappers around this one that allow getting access to the Tag data structure
// or to the data itself
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
inline INT32
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::FindWay(UINT64 index, UINT64 tag)
{
  UINT32 i;

  //cout << "FindWay on index " << fmt_x(index)
  //     << " tag " << fmt_x(tag) << "..." << endl;
  for ( i = 0; i < NumWays; i++ ) {
   //cout << "\tTagArray[" << fmt_x(index)
   //     << "][" << fmt_x(i)
   //     << "].GetTag " << TagArray[index][i].GetTag() << endl;
   if ( TagArray[index][i].GetTag() == tag ) return i;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// GET a line_state POINTER
//
// Function returns a pointer to the STATE of a given line. All ways of the cache are searched to find a
// match against parameter 'tag'. 
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
line_state<NumObjectsPerLine> *
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::GetLineState(UINT64 index, UINT64 tag)
{
 INT32 way;

 ASSERTX(index < NumLinesPerWay);

 // Do an associative search on the several ways of the cache.
 way = FindWay(index,tag);

 return ( way == -1 ) ? NULL : (&(TagArray[index][way]));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Get a specific way of a specific cache index 
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
line_state<NumObjectsPerLine> *
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::GetWayLineState(UINT64 index, UINT32 way)
{
 ASSERTX(index < NumLinesPerWay);
 ASSERTX(way < NumWays);
 
 return (&(TagArray[index][way]));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Get the TAG POINTER of the LEAST RECENTLY USED WAY IN A GIVEN SET
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
line_state<NumObjectsPerLine> *
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::GetLRUState(UINT64 index)
{
 UINT32 way;
 // Get the LRU way
 ASSERTX(index < NumLinesPerWay);
 way = LruArray[index].getLRU();
 // Return pointer to it.
 return &(TagArray[index][way]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Get the TAG POINTER of the MOST RECENTLY USED WAY IN A GIVEN SET
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
line_state<NumObjectsPerLine> *
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::GetMRUState(UINT64 index)
{
 UINT32 way;

 // Get the MRU way
 ASSERTX(index < NumLinesPerWay);
 way = LruArray[index].getMRU();
 // Return pointer to it.
 return &(TagArray[index][way]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UPDATE LRU
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::MakeMRU(UINT64 index, UINT32 way)
{
 ASSERTX(index < NumLinesPerWay);
 ASSERTX(way < NumWays);
 LruArray[index].makeMRU(way);
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::MakeLRU(UINT64 index, UINT32 way)
{
 ASSERTX(index < NumLinesPerWay);
 ASSERTX(way < NumWays);
 LruArray[index].makeLRU(way);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CLASSICAL MAPPING FUNCTIONS
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
inline UINT64
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::Index(UINT64 addr)
{
 addr = (addr >> ClassicalIndexShift) & IndexMask;
 ASSERTX(addr < NumLinesPerWay);

 return addr;
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
UINT64
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::Tag(UINT64 addr)
{
 addr = addr & ClassicalTagMask;
 return addr;
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
UINT64
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::Pos(UINT64 addr)
{
 addr = (addr >> 3) & PosMask;
 return addr;
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
UINT64
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::Original(UINT64 index, UINT64 tag)
{
 return (tag | (index << ClassicalIndexShift));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SHIFTED MAPPING FUNCTIONS
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
inline UINT64
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::IndexShifted(UINT64 addr)
{
 addr = (addr >> ShiftedIndexShift) & IndexMask;
 ASSERTX(addr < NumLinesPerWay);
 return addr;
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
UINT64
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::TagShifted(UINT64 addr)
{
 addr = addr & ShiftedTagMask;
 return addr;
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
UINT64
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::PosShifted(UINT64 addr)
{
 addr = addr & PosMask;
 return addr;
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
UINT64
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::OriginalShifted(UINT64 index, UINT64 tag)
{
 return (tag | (index << ShiftedIndexShift));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// THE FOLLOWING SET OF FUNCTIONS ONLY MAKE SENSE IF YOU HAVE INSTANTIATED THE TEMPLATE WITH PAREMETER 'WithData'
// SET TO true.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copies all data stored in a cache line into the array 'data'
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::GetLineData(UINT64 index, UINT32 way, T data[NumObjectsPerLine])
{
 if ( WithData == false ) return;

 ASSERTX(index < NumLinesPerWay);
 ASSERTX(way < NumWays);

 for (UINT32 i = 0; i < NumObjectsPerLine; i++ ) {
  data[i] =  DataArray[index][way][i];
 }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copies a single data item stored in a cache line into the variable 'data'
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::GetLineData(UINT64 index, UINT32 way, UINT32 ObjectInLine, T *data)
{
 if ( WithData == false ) return;

 ASSERTX(index < NumLinesPerWay);
 ASSERTX(way < NumWays);
 ASSERTX(ObjectInLine < NumObjectsPerLine);

 *data = DataArray[index][way][ObjectInLine];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This function Updates a FULL cache line of the DATA ARRAY.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::SetLineData(UINT64 index, UINT32 way, T data[NumObjectsPerLine])
{
 if ( WithData == false ) return;

 //
 // Compute index
 //
 ASSERTX(index < NumLinesPerWay);
 ASSERTX(way < NumWays);

 //
 // Check we are in the appropriate state
 //
 ASSERT((TagArray[index][way].GetStatus() == S_EXCLUSIVE_DIRTY)||(TagArray[index][way].GetStatus() == S_EXCLUSIVE_CLEAN),"You can not use SetLineData on a line that is not S_EXCLUSIVE_*\n");

 //
 // copy data into the cache
 //
 for (UINT32 i = 0; i < NumObjectsPerLine; i++ ) {
   ASSERT(TagArray[index][way].GetValidBit(i) == true,"ValidBit(i) must be true before writing data into the cache!\n");
   TagArray[index][way].SetDirtyBit(i);
   DataArray[index][way][i] = data[i];
 }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This function Updates a single OBJECT within a cache line of the DATA ARRAY.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::SetLineData(UINT64 index, UINT32 way, UINT32 ObjectInLine, T data)
{
 if ( WithData == false ) return;

 //
 // Compute index
 //
 ASSERTX(index < NumLinesPerWay);
 ASSERTX(way < NumWays);
 ASSERTX(ObjectInLine < NumObjectsPerLine);

 //
 // Check we are in the appropriate state
 //
 ASSERT((TagArray[index][way].GetStatus() == S_EXCLUSIVE_DIRTY)||(TagArray[index][way].GetStatus() == S_EXCLUSIVE_CLEAN),"You can not use SetLineData on a line that is not S_EXCLUSIVE_*\n");

 //
 // copy data item into the cache
 //
 ASSERT(TagArray[index][way].GetValidBit(ObjectInLine) == true,"ValidBit(i) must be true before writing data into the cache!\n");
 TagArray[index][way].SetDirtyBit(ObjectInLine);
 DataArray[index][way][ObjectInLine] = data;
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::Dump(UINT64 index, UINT32 way)
{
 ASSERTX(index < NumLinesPerWay);
 ASSERTX(way < NumWays);

 //cout << "\tDump for cache line at <" << fmt_x(index)
 //     << "x,--> way " << way << ":";
 TagArray[index][way].Dump();
}

template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::DumpLRU(UINT64 index)
{
 ASSERTX(index < NumLinesPerWay);
 cout << "Dump for index 0x" << fmt_x(index) << " :";
 LruArray[index].Dump();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Tester routine
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<UINT8 NumWays, UINT32 NumLinesPerWay, UINT32 NumObjectsPerLine, class T, bool WithData>
void
gen_cache_class<NumWays,NumLinesPerWay,NumObjectsPerLine,T,WithData>::tester(void)
{
  LruArray[4].Dump();

  LruArray[4].makeMRU(5);
  ASSERTX(LruArray[4].GetLRU() == 4);
  ASSERTX(LruArray[4].GetMRU() == 5);
  LruArray[4].Dump();

  LruArray[4].makeLRU(2);
  ASSERTX(LruArray[4].GetLRU() == 2);
  ASSERTX(LruArray[4].GetMRU() == 5);
  LruArray[4].Dump();

  LruArray[4].makeLRU(5);
  ASSERTX(LruArray[4].GetLRU() == 5);
  ASSERTX(LruArray[4].GetMRU() == 0);
  LruArray[4].Dump();
  
  LruArray[4].makeMRU(3);
  ASSERTX(LruArray[4].GetLRU() == 5);
  ASSERTX(LruArray[4].GetMRU() == 3);
  LruArray[4].Dump();

  LruArray[4].makeMRU(2);
  ASSERTX(LruArray[4].GetLRU() == 5);
  ASSERTX(LruArray[4].GetMRU() == 2);
  LruArray[4].Dump();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////
////
////  IMPLEMENTATION for LRU manager class
////
////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<UINT8 NumWays>
lru_info<NumWays>::lru_info()
{
  UINT32 i;

  //
  // We set a doubly linked list of all elements in the chain array
  //
  for (i = 0; i < NumWays; i++ ) {
   linklist[i].prev = (i == 0)            ? 0xff : (i-1);
   linklist[i].next = (i == (NumWays -1)) ? 0xff : (i+1);
  }

  // Now, by pure convention, we declare the MRU set, the one in position 0
  // and we declare the LRU set the one in position NumWays -1.
  //
  mru = 0;
  lru = NumWays - 1;
}

template<UINT8 NumWays>
void
lru_info<NumWays>::makeMRU(UINT8 w)
{
  ASSERTX(w < NumWays);

  //
  // Fast check: if 'w' is already the MRU, nothing to be done
  //
  if ( mru == w ) return;

  //
  // Step number one: unlink element 'w' from its current position in the doubly
  // linked list (and connect its neighbours together)
  //
  if ( linklist[w].prev != -1 ) linklist[linklist[w].prev].next = linklist[w].next;
  if ( linklist[w].next != -1 ) linklist[linklist[w].next].prev = linklist[w].prev;

  //
  // Step two; if element 'w' happened to be the LRU element, then update LRU
  //
  if ( lru == w ) lru = linklist[w].prev;

  //
  // Step three: move element 'w' to the head of the list (he is MRU).
  //
  linklist[w].prev = -1;
  linklist[w].next = mru;
  linklist[mru].prev = w;

  // Make 'w' the MRU element.
  mru = w;
}

template<UINT8 NumWays>
void
lru_info<NumWays>::makeLRU(UINT8 w)
{
  ASSERTX(w < NumWays);

  //
  // Fast check: if 'w' is already the LRU, nothing to be done
  //
  if ( lru == w ) return;

  //
  // Step number one: unlink element 'w' from its current position in the doubly
  // linked list (and connect its neighbours together)
  //
  if ( linklist[w].prev != -1 ) linklist[linklist[w].prev].next = linklist[w].next;
  if ( linklist[w].next != -1 ) linklist[linklist[w].next].prev = linklist[w].prev;

  //
  // Step two; if element 'w' happened to be the MRU element, then update MRU
  //
  if ( mru == w ) mru = linklist[w].next;

  //
  // Step three: move element 'w' to the tail of the list (he is LRU).
  //
  linklist[w].next = -1;
  linklist[w].prev = lru;
  linklist[lru].next = w;

  // Make 'w' the LRU element.
  lru = w;
}

template<UINT8 NumWays>
void
lru_info<NumWays>::Dump()
{
 INT8 p,i;

 cout << "MRU ->";
 for (i = 0, p = mru; i < NumWays; i++, p = linklist[p].next ) {
  ASSERTX(p != -1);
  cout << " " << p << " ->";
 }
 cout << endl;
}

#endif // CACHE_MESI_H
