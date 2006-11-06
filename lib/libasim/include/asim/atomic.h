/*
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

//
// Homegrown implementation of ATOMIC_CLASS for threaded Asim.
// This file provides classes ATOMIC and ATOMIC64 that are 32- and 64-bit integers
// that can be operated on atomically in a multithreaded program.
// This is a naive (but portable) implementation based on pthreads.
//
// THIS CODE IS TOTALLY UNTESTED.
//
// author: Carl Beckmann
// date: 10/9/2006
//
#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include <stdint.h>
#include <pthread.h>

//
// data types for 32- and 64-bit atomic classes
//
#define ATOMIC32_TYPE int32_t
#define ATOMIC64_TYPE int64_t

//
// generic atomic class, parameterized by data type
//
template <class ATOMIC_N_TYPE> class ATOMIC_N_CLASS {
private:
  pthread_mutex_t  lock;     // the lock guarding access to the data
  ATOMIC_N_TYPE    value;    // the data variable being operated on atomically

  // helper routines to acquire and release the lock
  inline void _lock  () { pthread_mutex_lock(    &lock ); };
  inline void _unlock() { pthread_mutex_unlock(  &lock ); };

public:
  // constructor: create and initialize the lock
  ATOMIC_N_CLASS<ATOMIC_N_TYPE>( ATOMIC_N_TYPE initval = 0 ) {
    value = initval;
    pthread_mutex_init( &lock, NULL );
  };

  // destructor: destroy the lock
  ~ATOMIC_N_CLASS<ATOMIC_N_TYPE> () {
    pthread_mutex_destroy( &lock );
  };

  // atomic read into the native type.
  // We need the lcck in case all bytes not written at once.
  inline operator ATOMIC_N_TYPE () {
    ATOMIC_N_TYPE retval;
    _lock();
    retval = value;
    _unlock();
    return retval;
  };

  // atomic assignment.  Must wait for lock before assigning
  inline ATOMIC_N_TYPE operator = ( ATOMIC_N_TYPE newval ) {
    _lock();
    value = newval;
    _unlock();
    return newval;
  };

  // return the old value, and then add the given number to it
  inline ATOMIC_N_TYPE ExchangeAndAdd( ATOMIC_N_TYPE addend ) {
    ATOMIC_N_TYPE retval;
    _lock();
    retval = value;
    value += addend;
    _unlock();
    return retval;
  };

  // add the given number and return the resulting sum
  inline ATOMIC_N_TYPE Add( ATOMIC_N_TYPE addend ) {
    ATOMIC_N_TYPE retval;
    _lock();
    value += addend;
    retval = value;
    _unlock();
    return retval;
  };

  inline ATOMIC_N_TYPE operator += (int addend) {
    return Add( addend );
  };

  inline ATOMIC_N_TYPE operator -= (int subtractor) {
    return Add( -subtractor );
  };

  // postfix operator.  Returns the old value
  inline ATOMIC_N_TYPE operator ++ () {
    return ExchangeAndAdd( 1 );
  };

  // postfix operator.  Returns the old value
  inline ATOMIC_N_TYPE operator -- () {
    return ExchangeAndAdd( -1 );
  };

  // prefix operator.  Returns the new value
  inline ATOMIC_N_TYPE operator ++ (int) {
    return Add( 1 );
  };

  // prefix operator.  Returns the new value
  inline ATOMIC_N_TYPE operator -- (int) {
    return Add( -1 );
  };
};


//
// 32-bit atomic class
//
typedef ATOMIC_N_CLASS<ATOMIC32_TYPE> ATOMIC_CLASS;
typedef ATOMIC_CLASS *ATOMIC;

//
// 64-bit atomic class
//
typedef ATOMIC_N_CLASS<ATOMIC64_TYPE> ATOMIC64_CLASS;
typedef ATOMIC64_CLASS *ATOMIC64;


//
// Definition of identifier generator type
//
#if NUM_PTHREADS > 1

#define UID_GEN32   ATOMIC_CLASS
#define UID_GEN64   ATOMIC64_CLASS

#else

#define UID_GEN32   ATOMIC32_TYPE
#define UID_GEN64   ATOMIC64_TYPE

#endif // NUM_PTHREADS > 1

#endif // __ATOMIC_H__
