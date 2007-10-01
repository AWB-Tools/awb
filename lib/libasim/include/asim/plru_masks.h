/*
 *Copyright (C) 2004-2007 Intel Corporation
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


#ifndef __PLRU_H__
#define __PLRU_H__

#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/utils.h"
#include "asim/ioformat.h"
#include "asim/trace.h"
#include "asim/cache_manager.h"

template<int Ways>
class PLRU_MaskInner{
  /* This class is a placeholder: it should only be used for one of the 
     template specification'ed instances below */
};




template<>
class PLRU_MaskInner<2> {
public:
  static const UINT64  mask[2];     
  static const UINT64 compare[2];  
};


template<>
class PLRU_MaskInner<3> {
public:
  static const UINT64 mask[3]   ;
  static const UINT64 compare[3] ;
};



template<>
class PLRU_MaskInner<4> {
public:
  static const UINT64 mask[4];    
  static const UINT64 compare[4]; 
};




template<>
class PLRU_MaskInner<6> {
public:
  static const UINT64 mask[6];   
  static const UINT64 compare[6];
};


template<>
class PLRU_MaskInner<8> {
public:
  static const UINT64 mask[8];    
  static const UINT64 compare[8];
};


template<>
class PLRU_MaskInner<12> {
public:
  static const UINT64 mask[12];
  static const UINT64 compare[12];
};


template<>
class PLRU_MaskInner<16> {
public:
  static const UINT64 mask[16]; 
  static const UINT64 compare[16];
};

template<>
class PLRU_MaskInner<24> {
public:
  static const UINT64 mask [24];

  static const UINT64 compare [24];
};

template<int Ways> 
class PLRU_Mask  {

public:
  static inline UINT64 GetMask(int wayNum) {
    return PLRU_MaskInner<Ways>::mask[wayNum];
  }
  static inline UINT64 GetCompare(int wayNum) {
    return PLRU_MaskInner<Ways>::compare[wayNum];
  }
  static UINT64 makeMRU(int way, UINT64 state) {
    return (GetMask(way) & ~GetCompare(way)) | (~GetMask(way) & state);
  }
  static UINT64 makeLRU(int way, UINT64 state) {
    return (GetMask(way) & GetCompare(way)) | (~GetMask(way) & state);
  }
  static int getLRUWayFor(UINT64 state){
    int i;
    for(i=0;i<Ways;i++) {
      if((state & GetMask(i)) == GetCompare(i))
	return i;
    }
    ASSERT(0, "No possible way in LRU selection?");
    return 0;
  }
  static int getLRUWayWithReservations(UINT64 state, UINT64 rsvd) {
    int way;
    int counter = 0;
    do {
      way = getLRUWayFor(state);
      /* if this is reserved, treat it as MRU and try again */
      state = makeMRU(way,state);
      counter++;
      ASSERT(counter<=Ways || ((rsvd & (1<<way) ==0)), "Can't find any ways!");
    }
    while(rsvd & (1 << way));
    return way;
  }
};

#endif 
