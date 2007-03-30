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


/*****************************************************************************
 *
 * @Brief Atomic counter type.  Useful for threadsafe reference counting
 *
 * @author FSF, Frank Mayhar, Kenneth C. Barr.  See note.
 * @brief Wrappers around atomic x86 code giving the effect of atomic increments/decrements/assignments.
 * @warning this is (temporarily) GPL'd.  When our build includes
 * atomicity.h and that file gets 64b extensions, you shouldn't need
 * to duplicate that file's code here.
 * 
 * @note 
 * NMSTL (nmstl.sourceforge.net) -- LGPL -- introduced me to the 32-bit version
 * of this class, but I think they just use code from recent libstdc++
 * (which is makes this GPL, too, I think)
 *
 * Adapted by Ken Barr based on code from Frank Mayhar (http://www.exit.com/blog/technotes/)
 * for 64 bit counters; had to special-case subtraction and merge a copy of
 * read_val into the add/sub for exchanges.
 *
 * @todo 
 *  is ASIM_ENABLE_MM_DEBUG the best way to turn on asserts?
 *  get these 64b changes into the official atomicity.h so that we can #include one file and not be "infected" by GPL here.
 *
 *****************************************************************************/

#ifndef ASIM_ATOMIC_H
#define ASIM_ATOMIC_H

#include "asim/syntax.h"
#include "asim/mesg.h"

#define ll_low(x)       *(((unsigned int*)&(x))+0)
#define ll_high(x)      *(((unsigned int*)&(x))+1)


//
// MemBarrier() is a simple barrier, currently for x86 only.  It
// guarantees memory synchronization using a locked reference to the stack.
// No sempahore is updated.  The function can be used on its own without
// a surrounding class since there is no need to allocate a specific
// memory location.
//
static inline void
__attribute__ ((__unused__))
MemBarrier(void)
{
#if __WORDSIZE < 64
    __asm__ __volatile__ ("lock; addl $0,0(%%esp)": : :"memory");
#else
    __asm__ __volatile__ ("lock; addl $0,0(%%rsp)": : :"memory");
#endif
}


//
// Compare and exchange.  All compare and exchange functions return a boolean
// result, true if the exchange happens.
//

inline bool
__attribute__ ((__unused__))
CompareAndExchangeU32(
    volatile UINT32 *mem,
    UINT32 oldValue,
    UINT32 newValue)
{
    UINT8 didXchg;
    __asm__ __volatile__("lock; cmpxchgl %1,%2\n\t"
                         "sete %0"
                             : "=a"(didXchg)
                             : "q"(newValue), "m"(*mem), "0"(oldValue)
                             : "memory");
    return didXchg;
}

inline bool
__attribute__ ((__unused__))
CompareAndExchangeU64(
    volatile UINT64 *mem,
    UINT64 oldValue,
    UINT64 newValue)
{
    UINT8 didXchg;
#if __WORDSIZE >= 64
    __asm__ __volatile__("lock; cmpxchgq %1,%2\n\t"
                         "sete %0"
                             : "=a"(didXchg)
                             : "q"(newValue), "m"(*mem), "0"(oldValue)
                             : "memory");
#else
    UINT32 dummy;
    __asm__ __volatile__("lock; cmpxchg8b %4\n\t"
                         "sete %0"
                             : "=a"(didXchg), "=d"(dummy)
                             : "b"((UINT32)newValue),
                               "c"((UINT32)(newValue >> 32)),
                               "m"(*mem),
                               "0"((UINT32)oldValue),
                               "1"((UINT32)(oldValue >> 32))
                             : "memory", "cc");
#endif
    return didXchg;
}

#ifdef INT128_AVAIL

inline bool
__attribute__ ((__unused__))
CompareAndExchangeU128(
    volatile UINT128 *mem,
    UINT128 oldValue,
    UINT128 newValue)
{
    UINT8 didXchg;
    UINT64 dummy;

#if __WORDSIZE >= 64
    // GNU assembler doesn't know about cmpxchg16b but assembles the 8b
    // version as a cmpxchg16b when the rex64 prefix is set.  This can be
    // changed when gas is fixed.
    __asm__ __volatile__("lock; rex64 cmpxchg8b (%4)\n\t"
                         "sete %0"
                             : "=a"(didXchg), "=d"(dummy)
                             : "b"(UINT64(newValue)),
                               "c"(UINT64(newValue >> 64)),
                               "S"(mem),
                               "0"(UINT64(oldValue)),
                               "1"(UINT64(oldValue >> 64))
                             : "memory", "cc");
#else
    ASIMERROR("CompareAndExchangeU128 doesn't work in 32 bit mode");
#endif
    return didXchg;
}

#endif // INT128_AVAIL

template <class T>
static inline bool
CompareAndExchange(
    T *mem,
    T oldValue,
    T newValue)
{
    if (sizeof(T) == 8)
    {
        return CompareAndExchangeU64((UINT64 *)mem,
                                     (UINT64)oldValue,
                                     (UINT64)newValue);
        return true;
    }
    else if (sizeof(T) == 4)
    {
        return CompareAndExchangeU32((UINT32 *)mem,
                                     (UINT32)oldValue,
                                     (UINT32)newValue);
    }
#ifdef INT128_AVAIL
    else if (sizeof(T) == 16)
    {
        return CompareAndExchangeU128((UINT128 *)mem,
                                      (UINT128)oldValue,
                                      (UINT128)newValue);
    }
#endif
    else
    {
        ASIMERROR("Unexpected size");
        return false;
    }
}



#if defined(__GLIBCPP__) && __GLIBCPP__ >= 20020814
#  include <bits/atomicity.h>
#else

// Routine from STL: bits/atomicity.h.  Currently for
// i386 only
typedef int _Atomic_word;


static inline _Atomic_word 
__attribute__ ((__unused__))
__exchange_and_add (volatile _Atomic_word *__mem, int __val)
{
    register _Atomic_word __result;
    __asm__ __volatile__ ("lock; xaddl %0,%2"
                          : "=r" (__result) 
                          : "0" (__val), "m" (*__mem) 
                          : "memory");
    return __result;
}


static inline void
__attribute__ ((__unused__))
__atomic_add (volatile _Atomic_word* __mem, int __val)
{
    __asm__ __volatile__ ("lock; addl %0,%1"
                          : : "ir" (__val), "m" (*__mem) : "memory");
}

#endif


typedef INT64 _Atomic64;

static inline void
__attribute__ ((__unused__))
__atomic_add (volatile _Atomic64* __mem, _Atomic64 __val)
{
#if __WORDSIZE >= 64
    __asm__ __volatile__ ("lock; addq %0,%1"
                          : : "ir" (__val), "m" (*__mem) : "memory");
#else
    __asm__ __volatile__ ( "movl  %0, %%eax\n\t" 
                           "movl  %1, %%edx\n\t" 
                           "1:\t movl %2, %%ebx\n\t"
                           "xorl %%ecx, %%ecx\n" 
                           "addl %%eax, %%ebx\n\t" 
                           "adcl %%edx, %%ecx\n\t"
                           "lock;  cmpxchg8b       %0\n\t"
                           "jnz             1b"
                           : "+o" (ll_low(*__mem)), "+o" (ll_high(*__mem))
                           : "m" (__val) 
                           : "memory", "eax", "ebx", "ecx", "edx", "cc"); 
#endif
}

static inline _Atomic64 
__attribute__ ((__unused__))
__exchange_and_add (volatile _Atomic64 *__mem, _Atomic64 __val)
{
    _Atomic64 __result;

#if __WORDSIZE >= 64
    __asm__ __volatile__ ("lock; xaddq %0,%2"
                          : "=r" (__result) 
                          : "0" (__val), "m" (*__mem) 
                          : "memory");
#else
    __asm__ __volatile__("movl  %0, %%eax\n\t" 
                         "movl  %1, %%edx\n\t"
                         "1:\t movl %4, %%ebx\n\t"
                         "xorl %%ecx, %%ecx\n"
                         "addl %%eax, %%ebx\n\t"
                         "adcl %%edx, %%ecx\n\t" 
                         "lock;  cmpxchg8b       %0\n\t"
                         "jnz             1b\n\t"
                         "movl            %%eax, %2\n\t" 
                         "movl            %%edx, %3\n" 
                         : "+o" (ll_low(*__mem)), "+o" (ll_high(*__mem)), "=m" (ll_low(__result)),  "=m" (ll_high(__result))
                         : "m" (__val) 
                         : "memory", "eax", "ebx", "ecx", "edx", "cc"); 
#endif
    return __result;
}

 
static inline void
__attribute__ ((__unused__))
__atomic_decr (volatile _Atomic64* __mem, _Atomic64 __val)
{
#if __WORDSIZE >= 64
    return __atomic_add(__mem, -__val);
#else
    __asm__ __volatile__ ( "movl  %0, %%eax\n\t" 
                           "movl  %1, %%edx\n\t" 
                           "1:\t movl %2, %%ebx\n\t"
                           "movl $0xffffffff, %%ecx\n" 
                           "addl %%eax, %%ebx\n\t" 
                           "adcl %%edx, %%ecx\n\t" 
                           "lock;  cmpxchg8b       %0\n\t"
                           "jnz             1b"
                           : "+o" (ll_low(*__mem)), "+o" (ll_high(*__mem))
                           : "m" (__val) 
                           : "memory", "eax", "ebx", "ecx", "edx", "cc"); 
#endif
}
    

static inline _Atomic64 
__attribute__ ((__unused__))
__exchange_and_decr (volatile _Atomic64 *__mem, _Atomic64 __val)
{
#if __WORDSIZE >= 64
    return __exchange_and_add(__mem, -__val);
#else
    _Atomic64 __result;

    __asm__ __volatile__("movl  %0, %%eax\n\t" 
                         "movl  %1, %%edx\n\t" 
                         "1:\t movl %4, %%ebx\n\t"
                         "movl $0xffffffff, %%ecx\n" 
                         "addl %%eax, %%ebx\n\t"
                         "adcl %%edx, %%ecx\n\t" 
                         "lock;  cmpxchg8b       %0\n\t"
                         "jnz             1b\n\t"
                         "movl            %%eax, %2\n\t" 
                         "movl            %%edx, %3\n" 
                         : "+o" (ll_low(*__mem)), "+o" (ll_high(*__mem)), "=m" (ll_low(__result)),  "=m" (ll_high(__result))
                         : "m" (__val) 
                         : "memory", "eax", "ebx", "ecx", "edx", "cc"); 
    return __result;
#endif
}


static inline _Atomic64 
__attribute__ ((__unused__)) 
__read_val(const _Atomic64 * target) 
{
    _Atomic64 __out;

#if __WORDSIZE >= 64
    __out = *target;
#else
    __asm__ __volatile__("       xorl            %%eax, %%eax\n"
                         "       xorl            %%edx, %%edx\n"
                         "       xorl            %%ebx, %%ebx\n"
                         "       xorl            %%ecx, %%ecx\n"
                         "lock;  cmpxchg8b       %2\n"
                         "       movl            %%eax, %0\n"
                         "       movl            %%edx, %1"
                         : "=m" (ll_low(__out)), "=m" (ll_high(__out))
                         : "o" (*target)
                         : "memory", "eax", "ebx", "ecx", "edx", "cc");
#endif

    return __out;
}



class ATOMIC32_CLASS {
  public:
    typedef _Atomic_word word;

  private:
    word val;

  public:

    /// Constructs an atomic integer with a given initial value.
    ATOMIC32_CLASS(int val = 0) : val(val) {}

    /// Atomically increases the value of the integer and returns its
    /// old value.
    inline word ExchangeAndAdd(int addend) {
        return __exchange_and_add(&val, addend);
    }

    /// Atomically increases the value of the integer.
    inline void Add(int addend) {
        __atomic_add(&val, addend);
    }

    /// Atomically increases the value of the integer.
    inline void operator += (int addend) {
        Add(addend);
    }

    /// Atomically decreases the value of the integer.
    inline void operator -= (int addend) {
        Add(-addend);
    }

    /// Atomically increments the value of the integer.
    inline void operator ++ () {
        Add(1);
    }

    /// Atomically decrements the value of the integer.
    inline void operator -- () {
        Add(-1);
    }

    /// Atomically increments the value of the integer and returns its
    /// old value.
    inline int operator ++ (int) {
        return ExchangeAndAdd(1);
    }

    /// Atomically decrements the value of the integer and returns its
    /// old value.
    inline int operator -- (int) {
        return ExchangeAndAdd(-1);
    }

    /// Returns the value of the integer.
    operator int() const { return val; }

    friend std::ostream & operator<<(std::ostream &, ATOMIC32_CLASS);
};

typedef ATOMIC32_CLASS *ATOMIC32;


class ATOMIC64_CLASS {
  public:
    typedef _Atomic64 word;

  private:
    word val;

  private:
    //disable a few ops that would aren't yet defined
    word operator+(int);
    word operator+(long long);
    word operator-(int);
    word operator-(long long);

  public:

    /// Constructs an atomic integer with a given initial value.
    ATOMIC64_CLASS(word val = 0) : val(val) {}
    
    /// Atomically increases the value of the integer and returns its
    /// old value.
    inline word ExchangeAndAdd(word addend) {
#ifdef ASIM_ENABLE_MM_DEBUG
        _Atomic64 a = __read_val((word *)&val);
        ASSERT(a != 0x7fffffffffffffffULL, "overflowing 64b counter");
#endif
        return __exchange_and_add((_Atomic64 *)&val, addend);
    }
    
    /// Atomically increases the value of the integer.
    inline void Add(word addend) {
#ifdef ASIM_ENABLE_MM_DEBUG
        _Atomic64 a = __read_val((_Atomic64 *)&val);
        ASSERT(a != 0x7fffffffffffffffULL, "overflowing 64b counter");
#endif
        __atomic_add((_Atomic64 *)&val, addend);
    }
    
    
    /// Atomically decreases the value of the integer and returns its
    /// old value.
    inline word ExchangeAndSub(word addend) {
        return __exchange_and_decr((_Atomic64 *)&val, addend);
    }

    /// Atomically decreases the value of the integer.
    inline void Sub(word addend) {
        __atomic_decr((_Atomic64 *)&val, addend);
    }


    /// Atomically increases the value of the integer.
    inline void operator += (word addend) {
        Add(addend);
    }

    /// Atomically decreases the value of the integer.
    inline void operator -= (word addend) {
        Sub(-addend);
    }

    //NOTE: cannot use these in prefix ops in expressions!
    /// Atomically increments the value of the integer.
    inline void operator ++ () {        
        Add(1);
    }

    /// Atomically decrements the value of the integer.
    inline void operator -- () {
        Sub(-1);
    }

    //postfix ops: "int" is a dummy arg
    /// Atomically increments the value of the integer and returns its
    /// old value.
    inline word operator ++ (int) {
        return ExchangeAndAdd(1);
    }

    /// Atomically decrements the value of the integer and returns its
    /// old value.
    inline word operator -- (int) {
        return ExchangeAndSub(-1);
    }

    /// Returns the value of the integer.
    operator word() const { return __read_val((_Atomic64 *)&val); }

    friend std::ostream & operator<<(std::ostream &, ATOMIC64_CLASS);
};

typedef ATOMIC64_CLASS *ATOMIC64;


/*
std::ostream & operator<< (std::ostream & os, atomic a){
    os << __read_val((_Atomic64 *)(&(a.val)));
    return os;
}
*/

#if MAX_PTHREADS > 1
typedef ATOMIC32_CLASS ATOMIC_INT32;
typedef ATOMIC64_CLASS ATOMIC_INT64;
#else
typedef INT32 ATOMIC_INT32;
typedef INT64 ATOMIC_INT64;
#endif

// Definition of UID_GEN types (unique identifier generator)
typedef ATOMIC_INT32 UID_GEN32;
typedef ATOMIC_INT64 UID_GEN64;

#endif
