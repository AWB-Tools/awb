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
 * @author Shubu Mukherjee, Artur Klauser
 * @brief Usefull little gadgets.
 */

// generic
#include <cstring>

// ASIM core
#include "asim/utils.h"
#include "asim/syntax.h"
#include "asim/mesg.h"

char *
strjoin(char *s1, char *s2)
{
 INT32 len;
 char *ptr;

 ASSERTX(s1 != NULL);
 ASSERTX(s2 != NULL);
 len = strlen(s1) + strlen(s2) + 1;
 ptr = new char[len];

 strcpy(ptr,s1);
 strcat(ptr,s2);

 return ptr;
}


//
// DEC CC/CXX support fast intrinsic trailing/leading zeros implementations;
// other compilers have to use our own C code
//
#ifdef __DECC

// generic
#include <alpha/builtins.h>

//
// compute the number of leading zeros
//
UINT32 leadingZeros(UINT64 a) {
  // GEM CC intrinsic; really fast if compiled with "-tune ev67 -intrinsics"
  return _leadz(a);
}

//
// compute the number of trailing zeros
//
UINT32 trailingZeros(UINT64 a) {
  // GEM CC intrinsic; really fast if compiled with "-tune ev67 -intrinsics"
  return _trailz(a);
}

#else // __DECC

//
// compute the number of leading zeros
//
UINT32 leadingZeros(UINT64 a) {
  UINT32 ret;
  UINT32 topHalf;

  if (a == 0) {
    return 64;
  }

  //--------------------------------
  topHalf = (a >= (1LL << 32)) << 5;
  ret = topHalf;
  a >>= topHalf;
  //--------------------------------
  topHalf = (a >= (1L << 16)) << 4;
  ret += topHalf;
  a >>= topHalf;
  //--------------------------------
  topHalf = (a >= (1L << 8)) << 3;
  ret += topHalf;
  a >>= topHalf;
  //--------------------------------
  topHalf = (a >= (1L << 4)) << 2;
  ret += topHalf;
  a >>= topHalf;
  //--------------------------------
  topHalf = (a >= (1L << 2)) << 1;
  ret += topHalf;
  a >>= topHalf;
  //--------------------------------
  topHalf = (a >= (1L << 1)) << 0;
  ret += topHalf;
  //--------------------------------

  return (63 - ret);
}

//
// compute the number of trailing zeros
//
UINT32 trailingZeros(UINT64 a) {
  UINT32 ret;
  UINT32 notBottomHalf;

  if (a == 0) {
    return 64;
  }

  //--------------------------------
  notBottomHalf = ((a & ((1LL << 32) - 1)) == 0) << 5;
  ret = notBottomHalf;
  a >>= notBottomHalf;
  //--------------------------------
  notBottomHalf = ((a & ((1L << 16) - 1)) == 0) << 4;
  ret += notBottomHalf;
  a >>= notBottomHalf;
  //--------------------------------
  notBottomHalf = ((a & ((1L << 8) - 1)) == 0) << 3;
  ret += notBottomHalf;
  a >>= notBottomHalf;
  //--------------------------------
  notBottomHalf = ((a & ((1L << 4) - 1)) == 0) << 2;
  ret += notBottomHalf;
  a >>= notBottomHalf;
  //--------------------------------
  notBottomHalf = ((a & ((1L << 2) - 1)) == 0) << 1;
  ret += notBottomHalf;
  a >>= notBottomHalf;
  //--------------------------------
  notBottomHalf = ((a & ((1L << 1) - 1)) == 0) << 0;
  ret += notBottomHalf;
  //--------------------------------

  return (ret);
}

#endif // __DECC
