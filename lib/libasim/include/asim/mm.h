/**************************************************************************
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
 * @author David Goodwin, Nathan Binkert, Artur Klauser, Ken Barr
 * @brief Memory management base class (ref. counting, allocation, debugging)
 */

#ifndef _MM_
#define _MM_

// Enable These To Debug MM_OBJ bugs
//#define ASIM_ENABLE_MM_DEBUG 1
//#define MM_OBJ_DUMP 1

// generic
#include <iostream>
#include <string>
#include <vector>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/mmptr.h"
#include "asim/ioformat.h"
#include "asim/atomic.h"
#include "asim/smp.h"

namespace iof = IoFormat;
using namespace iof;

typedef UINT64 MM_UID_TYPE;

/*
Original mmptr code would break when running in multiple threads [1]: it's too
easy to assign one object to n different threads and not track it appropriately.
To fix this, we can split shared structures into NUM_PTHREADS pools and use an
ATOMIC_CLASS reference count. This causes each thread to be allocated objects from
distinct pools (thus no locking is required).

The freelist, however, must be locked to handle object deletion.  For recycling,
they must be placed on the original owner's freelist, but they may be deleted by
a different thread.  TODO: It's probably that it -will- be the owner who is the last
reference; can we exploit this?  (if (get_asim_thread_id == obj->ownerThread) {
no lock needed? } )

Handling destruction of statics is a mess.  Write a good destructor and see
comments in LastRefDropped()

1. in this context, "threads" means -real- threads (pthreads) on the machine hosting
the simulator, NOT software contexts.
*/

#if NUM_PTHREADS > 1

  #include <pthread.h>
  #define POOL get_asim_thread_id()
  #define lockFreeList(p)   pthread_mutex_lock(p);
  #define unlockFreeList(p) pthread_mutex_unlock(p);

#else

  #define POOL 0
  #define lockFreeList(p)
  #define unlockFreeList(p)

#endif


/**
 * Macro used to define static members in ASIM_MM_CLASS.
 */
#define ASIM_MM_DEFINE_L(M,L,MAX) \
template<> \
ASIM_MM_CLASS<M,L>::DATA ASIM_MM_CLASS<M,L>::data(MAX, #M);

#define ASIM_MM_DEFINE(M, MAX) \
template<> \
ASIM_MM_CLASS<M>::DATA ASIM_MM_CLASS<M>::data(MAX, #M);

// even more per-object debugging can be turned on with MM_OBJ_DUMP
//#define MM_OBJ_DUMP

// allocate all memory of the pool at once (rather than each object on demand)
#define MM_PREALLOC_MEMORY

/**
 * Turn on valgrind annotations - an x86 memory access checker.
 * with these annotations we can declare accesses to objects illegal while
 * they are on our free list, and have valgrind check for violations;
 * by itself, valgrind can not track memory violations of user-level
 * memory management (like this MM code) and would deem every access legal
 * if the memory has been new/malloc'ed but not yet delete/free'd;
 *
 * How to use this:
 *  - uncomment the define below
 *  - in Makefile.config, add -I/proj/vssad/local/i386_linux24/include
 *    to CFLAGS (or whereever else you might have valgrind.h installed)
 *  - run benchmark under valgrind, e.g. with
 *    # valgrind --gdb-attach=yes --num-callers=12 <benchmark_and_flags>
 *  - turn back off again (ie. restore this file) before you check in
 */
//#define MM_VALGRIND

#ifdef MM_VALGRIND
#include <valgrind.h>
#endif


#ifdef ASIM_ENABLE_MM_DEBUG
/**
 * This macro should be used by derived objects to "protect" all
 * methods that access the object. When debugging, this macro will
 * make sure that the object being accessed is not freed.
 */
#define MMCHK MmCheckRefCnt(__LINE__, __FILE__)
#else
#define MMCHK
#endif

extern bool debugOn;

/**
 * @brief simple reference counting, memory debugging class
 *
 * This class provides support for reference counting, memory allocation,
 * and memory debugging. We'll call a class M that is derived from
 * ASIM_MM_CLASS<M,L> also a "MM class", and its objects are called "MM
 * objects" for short.
 *
 * - Reference Counting:
 *   ASIM_MM_CLASS is a templated base class for objects that require
 *   reference counting for correct operation. For an MM class, it is
 *   not necessary to call the delete operator on its objects.
 *   Instead, the object will automatically be deleted when there are
 *   no more references to it (this is done by reference counting,
 *   not liveness analysis and garbage collection like in Java).
 *
 * - Memory Allocation:
 *   Deleted objects are kept on a free list in order to avoid the
 *   malloc/free overhead for the storage associated with the object.
 *   When an object is newed, the storage for the object is taken off
 *   the free list. However, newed objects do execute their associated
 *   constructor in the normal way. @note The object's destructor is
 *   called only immediately before the storage is reassigned to a new
 *   object from the free list, NOT as one might assume, when the
 *   object gets put onto the freelist. This has only implications if
 *   you are trying to do something interesting in the destructor,
 *   like stats keeping.
 *
 * - Memory Debugging:
 *   MM classes must declare the maximum number of objects of their type
 *   that can be in use an any time.  The ASIM_MM_CLASS will make sure
 *   that this limit can not be exceeded, providing a relatively fast
 *   detection of memory leaks.  When accessing MM objects in code
 *   compiled for debug, the validity of the access is checked by
 *   making sure the object has a reference count > 0. While compiled
 *   for debug the MM objects also carry a unique ID "mmUid" and the
 *   particular MM class carries a class name string. These fields can
 *   assist in debugging memory problems, by allowing each MM object
 *   to be easily uniquely identified. In addition, in debug mode MM object
 *   references check that they are references to the correct type by
 *   making sure the object carries a key (mmMagicKey) that
 *   corresponds to the object type they are supposed to represent.
 *   For performance reasons, many of the debugging features are not
 *   compiled in for non-debug compilation.
 */
template <class MM_TYPE, bool LAZY_DEST = false>
class ASIM_MM_CLASS
{
  private:
    // types
    // objects on the free list (mmCnt must always be < 0)
    /// object is freed but not yet destructed
    static const INT32 MMCNT_ON_FREELIST_NOT_DELETED = -1;
    /// object is both freed and destructed
    static const INT32 MMCNT_ON_FREELIST_AND_DELETED = -2;

    /**
     * @brief Per MM class data
     */
  public:

    class DATA
    {
      public:

        // types
        typedef vector<MM_TYPE *> ObjectVector;

        // members
        string className;      ///< Name of the class that we are managing

#ifdef ASIM_ENABLE_MM_DEBUG
        /// magic key for this MM object type (only in debug mode)
        UINT32 mmMagicKey;
#endif

        UINT32 mmMaxObjs;       ///< Maximum number of objects
        UINT32 mmTotalObjs[NUM_PTHREADS];     ///< Number of Objects
        /// List of free MM_TYPE objects; its used as a stack, but we also
        /// need to have an iterator, so <stack> does not work;
        ObjectVector mmFreeList[NUM_PTHREADS];

#if NUM_PTHREADS > 1
        pthread_mutex_t mmFreeListLock[NUM_PTHREADS];
#endif

#ifdef MM_OBJ_DUMP
        /// List of all objects (only in extended debug mode MM_OBJ_DUMP)
        ObjectVector mmObjList[NUM_PTHREADS];
#endif // MM_OBJ_DUMP

        bool destructed;        ///< has destructor for this already run

        // constructors/destructors
                                         //   magic = 0xf.d.b.9.  -->
                                         //             .0.2.4.6  <--
        DATA (UINT32 max, string name, UINT32 magic = 0xf0d2b496);
        ~DATA();

        /// Destructor call for undestructed objects on free list
        void FreeListObjectDestructor(void);
        /// Dispose of objects properly at end of run
        void FinalObjectCleanup (MM_TYPE * obj);

        // debug
        /// Dump all objects of this MM type.
        void ObjDump(void);
    };

  private:
    /// To aid debugging make mmptr friend of MM
    friend class mmptr<MM_TYPE>;

  public:
    // static members
    static DATA data; ///< per MM class data

    // members
    /// current reference count of this object; @note we use mmCnt < 0
    /// for error checking; we keep mmCnt < 0 while the object is on
    /// the free list, so we can detect illegal operations on the
    /// object during this time;

#if NUM_PTHREADS > 1
    ATOMIC_CLASS mmCnt;  //ATOMIC_CLASS to support references across multiple threads.
#else
    INT32 mmCnt;
#endif

    int mmOwnerThread;   //note which thread 1st grabbed this object.  It will be returned to the same freelist

  private:

#ifdef ASIM_ENABLE_MM_DEBUG
    /// magic key for this MM object; has to match the key for this
    /// MM object type (only in debug mode)
    UINT32 mmMagicKey;
#endif

    MM_UID_TYPE mmUid; ///< unique ID of this MM object

  public:

    // constructors/destructors
    /// Create a new MM object
    /// \param uid A unique object identifier.
    /// \param initCount Initial value for the reference counter.
    ASIM_MM_CLASS (MM_UID_TYPE uid = 0, UINT32 initCount = 0);
    /// Delete this MM object
    virtual ~ASIM_MM_CLASS();

    // methods
    /// Increment reference count for this object
    inline void IncrRef (void);
    /// Decrement reference count for this object
    inline void DecrRef (void);

    // accessors / modifiers
    /// Get reference count
    INT32 GetMMRefCount(void) const { return mmCnt; }

    /// Get Uid
    MM_UID_TYPE GetMMUid(void) const { return mmUid; }

#ifdef ASIM_ENABLE_MM_DEBUG
    /// Check if object can legally be accessed in current state
    void MmCheckRefCnt (UINT32 line, char *file) const;
#endif

    /// Get a free object from the free list
    void * operator new (size_t size);
    /// Stub for delete: check if object could legally be deleted
    void operator delete (void * ptr, size_t size);

    // debugging
    /// Dump MM info about this object
    virtual void Dump(int count) const;

    // static methods
    /// Reset maximum number of objects in allocation pool to new value
    static void SetMaxObjs(UINT32 max);

  private:

    /// Put object back on free list if last reference is dropped
    void LastRefDropped (void);

    /// Pre-allocate memory for the whole pool of objects
    static void PreAllocateMemory (void);
};

//----------------------------------------------------------------------------
// implementation
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ASIM_MM_CLASS<>::DATA
//----------------------------------------------------------------------------

/**
 * Create a new 'factory' object of this object type
 */
template <class MM_TYPE, bool LAZY_DEST>
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::DATA::DATA (
    UINT32 max,   ///< max number of objects of this type
    string name,  ///< name of this MM object type
    UINT32 magic) ///< magic key for this MM object type (only in debug mode)
  : className(name),
#ifdef ASIM_ENABLE_MM_DEBUG
    mmMagicKey(magic),
#endif
    mmMaxObjs(max),
    destructed(false)
{
    for(int i=0; i<NUM_PTHREADS; i++)
    {
        mmTotalObjs[i]=0;

#if NUM_PTHREADS > 1
        pthread_mutex_init(&mmFreeListLock[i], NULL);
#endif

    }
}

/**
 * Delete this object type
 */
template <class MM_TYPE, bool LAZY_DEST>
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::DATA::~DATA()
{
    if (debugOn)
    {
        cout << className << ": Peak Number of Objects = "
             << mmTotalObjs[POOL] << endl;
    }

#ifdef MM_OBJ_DUMP
    // dump a list of all objects of this type
    ObjDump();
#endif // MM_OBJ_DUMP

    // all objects of this MM type should be on the free list by the
    // time we call the destructor for this type ...
    if (debugOn)
    {
        if (mmFreeList[POOL].size() != mmTotalObjs[POOL])
        {
            cerr << "WARNING: ASIM_MM_CLASS<" << className << ">" << endl
                 << "  has allocated " << mmTotalObjs[POOL] << " objects, but only "
                 << mmFreeList[POOL].size() << " can be found at destructor time."
                 << endl;
        }
    }
    for(int i=0; i<NUM_PTHREADS; i++)
    {
        // free all objects that are on the free list
        while ( ! mmFreeList[i].empty())
        {
            MM_TYPE * obj = mmFreeList[i].back();
            mmFreeList[i].pop_back();
            FinalObjectCleanup(obj);
        }

    }

    destructed = true;

#if NUM_PTHREADS > 1
    for(int i=0; i<NUM_PTHREADS; i++)
    {
        pthread_mutex_destroy(&mmFreeListLock[i]);
    }
#endif

}

/**
 * When the life of this MM class comes to an end, make sure we dispose
 * properly of all objects that are under our supervison.
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::DATA::FinalObjectCleanup(MM_TYPE * obj)
{ 
    if (obj->mmCnt == MMCNT_ON_FREELIST_NOT_DELETED)
    {

#ifdef MM_VALGRIND
        // make it legal to call the destructor on the object now
        VALGRIND_DISCARD (
            VALGRIND_MAKE_READABLE (obj, sizeof(MM_TYPE)));
#endif

        // call the objects destructor
        obj->mmCnt = MMCNT_ON_FREELIST_AND_DELETED;
        delete obj;
    }

    // free the actual memory we allocated in MM for this object
    delete [] (char*) obj;
}

#ifdef MM_OBJ_DUMP
/**
 * Dump all objects of this MM type.
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::DATA::ObjDump(void)
{
#ifdef MM_OBJ_DUMP
    cout << "Object Dump:" << endl;

    if (mmObjList[POOL].empty())
    {
        cout << "  No " << className << " objects in use." << endl;
    }
    else
    {
        int count = 0;

        for (typename ObjectVector::const_iterator it = mmObjList[POOL].begin();
             it != mmObjList[POOL].end();
             it++)
        {
            if (*it && (*it)->mmCnt > 0)
            {
                (*it)->Dump(count++);
            }
        }
    }
#endif // MM_OBJ_DUMP
}
#endif // MM_OBJ_DUMP

//----------------------------------------------------------------------------
// ASIM_MM_CLASS<>
//----------------------------------------------------------------------------

/**
 * Increment the reference count for this object. If this object has
 * been put back on the free list (by a previous DecrRef(), this call
 * is trying to illegaly revive it - flag this error.
 */
template <class MM_TYPE, bool LAZY_DEST>
inline void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::IncrRef (void)
{
    MMCHK;

#ifdef ASIM_ENABLE_MM_DEBUG
    ASSERT (mmCnt >= 0, "MM Object type " << data.className
        << " referenced while on free list.");
#endif

//     if( data.className == "MICRO_INST_CLASS" )
//     {
//         if( mmUid >= 1 && mmUid <=10 ){
//             cout <<" Incrementing Ref for MACRO type mmuid: "<<mmUid<<endl;
//         }
//     }

    mmCnt++;
}



/**
 * Decrement the reference count of this object. If the reference
 * count drops down to 0, we continue this work in LastRefDropped().
 * Even though this split seems useless, it helps keeping DecrRef()
 * small enough to be inlinable, and the most frequent path through it
 * does not call LastRefDropped() at all.
 */
template <class MM_TYPE, bool LAZY_DEST>
inline void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::DecrRef (void)
{
    MMCHK;

    // FOR THREAD-SAFE MM_OBJECTS ONLY:
    // mmCnt is atomically decremented, but its value may
    // be changed by another thread concurrently. We must make a copy
    // to compare it with 1. Do not compare the mmCnt itself!!
    INT32 mmCntCopy = mmCnt --;


//    if( data.className == "MICRO_INST_CLASS" )
//    {
//        if(mmCntCopy < 3)
//        {
//            cout <<" Decrementing Ref for MICRO type mmuid: "<<mmUid;
//            cout << "  Count = " << mmCntCopy << endl;
//        }
//    }

    // mmCntCopy is the value before being decremented, so we have to
    // compare it with 1 instead of 0!
    if (mmCntCopy <= 1)
    {
        // continue the work there ...
        LastRefDropped();
    }
}

/**
 * When the reference count drops down to 0, put the object back on
 * the free list.
 *
 * @note The reason why this is separated out from DecrRef(), which is
 * its only caller, is that this way DecrRef() can be inlined and in
 * most cases this continutation of the work of DecrRef() does not
 * need to be called anyway.
 *
 * @note An object is not 'deleted' when it is put on the free list,
 * ie. its destructor is not called. The destructor for the object is
 * called lazyly before the memory associated with the object is taken off
 * the free list again and reassigned to another object (of this same
 * type). This is to avoid cascading effects of recursive deletes that
 * have been shown to run us out of stack space and thus crash the
 * application. @see also @ref lazy_delete "Lazy Delete".
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::LastRefDropped (void)
{
    ASSERT (mmCnt == 0, "MM Object type " << data.className
        << " ref count decremented while on free list.");

    // indicate to object that it is on free list, pending deletion
    mmCnt = MMCNT_ON_FREELIST_NOT_DELETED;

    MM_TYPE *obj = (MM_TYPE *)this;

    if (!data.destructed)
    {
        // Note: due to the undeterminate ordering of global
        // destructors, we might execute this code for objects
        // AFTER the static data object has already been destroyed;
        // in that case we refrain from writing to it;

        lockFreeList(&data.mmFreeListLock[mmOwnerThread]);

        data.mmFreeList[mmOwnerThread].push_back(obj);

        if(!LAZY_DEST)
        {
            obj->mmCnt = MMCNT_ON_FREELIST_AND_DELETED;
            delete obj;
        }
        
        unlockFreeList(&data.mmFreeListLock[mmOwnerThread]);
    }
    else
    {
        //TODO: only if(data.destructed)
        // we are past the destuctor call of our static DATA object which
        // has already abandoned the free list, so we have to cleanup
        // directly any remaining objects which would get onto the free
        // list in the normal life

        data.FinalObjectCleanup(obj);
    }

#ifdef MM_VALGRIND
    // object is on the free list and should not be accessed anymore!
    // revoke access permission from object's memory range
    VALGRIND_DISCARD ( VALGRIND_MAKE_NOACCESS (obj, sizeof(MM_TYPE)));
    // allow read+write on MM meta information though
    VALGRIND_DISCARD (
        VALGRIND_MAKE_READABLE (obj, sizeof(ASIM_MM_CLASS<MM_TYPE>)));
#endif

}

/**
 * Pre-allocate all memory for this MM pool of objects
 *
 * Pre-requisite: The mmFreeListLock[POOL] must be caught when calling this method
 *
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::PreAllocateMemory (void)
{

    // it is not OK to call this while there are objects on the freelist
    ASSERT (data.mmFreeList[POOL].empty(), "MM Object type " << data.className
        << " memory pre-allocation failed.");

    // allocate all objects that are missing
    for ( ; data.mmTotalObjs[POOL] < data.mmMaxObjs; data.mmTotalObjs[POOL]++)
    {

        MM_TYPE * newMmObj;
        newMmObj = ((MM_TYPE *) new char[sizeof(MM_TYPE)]);

#ifdef MM_OBJ_DUMP
        data.mmObjList[POOL].push_back(newMmObj);
#endif //MM_OBJ_DUMP

        // object is on the freelist, no deletion necessary
        newMmObj->mmCnt = MMCNT_ON_FREELIST_AND_DELETED;
        data.mmFreeList[POOL].push_back(newMmObj);

#ifdef MM_VALGRIND
        // object is on the free list and should not be accessed anymore!
        // revoke access permission from object's memory range
        VALGRIND_DISCARD ( VALGRIND_MAKE_NOACCESS (newMmObj, sizeof(MM_TYPE)));
        // allow read+write on MM meta information though
        VALGRIND_DISCARD (
            VALGRIND_MAKE_READABLE (newMmObj, sizeof(ASIM_MM_CLASS<MM_TYPE>)));
#endif

    }
}


/**
 * When we put objects onto the free list we don't call their destructor
 * to avoid cascading affects that can run us out of stack space.
 * Here is where we rectify this.
 *
 * @note @anchor lazy_delete
 * Lazy Delete - Why are objects not destructed as they are put onto the
 * free list? Assume somebody has a linked list of MM objects, and the
 * link is of MMPTR type. Assuming there are no other references to those
 * objects pending, when the head of the list is deleted and its
 * destructor called, it will destruct the associated MMPTR, which
 * decrements the refCnt of the second object on the list down to 0.
 * Now that destructor is called recursively while we are still in the
 * first destructor. The depth of this recursion is only bound by the
 * lenght of the list. With complicated webs of data structures this
 * immediate call of the destructor can easily result in aborts due to
 * stack overflows.
 * We avoid this recursion by turning it into iteration. We put the
 * objects onto the free list without deleting them yet. But the next time
 * we take something off the free list we run through it (in a loop) and
 * call the destructors of the objects that have not yet been
 * distructed.

 * This is only called from 'new' which handles the freelist locking.
 *
 *
 * Prerequisite: The mmFreeListLock[POOL] must be caught before calling this method
 *
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::DATA::FreeListObjectDestructor (void)
{
    MM_TYPE * obj;
    if (mmFreeList[POOL].empty())
    {
        return;
    }

    // note: we need two loops since the inner loop can push new
    // objects onto the free list, so the outer loop checks for that;
    for (obj = mmFreeList[POOL].back();
         obj->mmCnt == MMCNT_ON_FREELIST_NOT_DELETED;
         obj = mmFreeList[POOL].back())
    {
        for (typename ObjectVector::reverse_iterator it =
             mmFreeList[POOL].rbegin();
             it != mmFreeList[POOL].rend();
             it++)
        {
            obj = *(it);

            // note: all not yet delted items are on the top of the
            // stack, so we can stop as soon as we see a deleted one;
            if (obj->mmCnt == MMCNT_ON_FREELIST_AND_DELETED)
            {
                break;
            }

#ifdef MM_VALGRIND
            // make it legal to call the destructor on the object now
            VALGRIND_DISCARD (VALGRIND_MAKE_READABLE (obj, sizeof(MM_TYPE)));
#endif

            // Note: The destructor might have the side effect of
            // putting new objects onto this (and other) free lists;
            obj->mmCnt = MMCNT_ON_FREELIST_AND_DELETED;

            delete obj;

#ifdef MM_VALGRIND
            // restrict object access again - as it remains on the free list
            // revoke access permission from object's memory range
            VALGRIND_DISCARD (VALGRIND_MAKE_NOACCESS (obj, sizeof(MM_TYPE)));
            // allow read+write on MM meta information though
            VALGRIND_DISCARD (
                VALGRIND_MAKE_READABLE (obj, sizeof(ASIM_MM_CLASS<MM_TYPE>)));
#endif

        }
    }
}

/**
 * Create a new MM object of this MM type. The memory has been
 * provided by the operator new of this object type before this
 * constructor is run, so we know this block of memory is not on the
 * free list anymore.
 *
 * @note The mmUid and mmMagicKey are only provided when debugging is
 * enabled.
 */
template <class MM_TYPE, bool LAZY_DEST>
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::ASIM_MM_CLASS (
    MM_UID_TYPE uid,   ///< unique ID for this object
    UINT32 initCount)  ///< initial ref count for this object
    : mmCnt(initCount),
#ifdef ASIM_ENABLE_MM_DEBUG
    mmMagicKey(data.mmMagicKey),
#endif
    mmUid(uid)
{
    // nada
}

/**
 * @note The destructor has to be virtual in order to allow
 * derived classes to define their own destructor, which should
 * get called instead of this one.
 */
template <class MM_TYPE, bool LAZY_DEST>
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::~ASIM_MM_CLASS ()
{
    // nada
}

/**
 * Allocate memory for a MM object of this type. We either get a free
 * object from 'mmFreeList', or malloc a new one if 'mmFreeList' is
 * empty.
 */
template <class MM_TYPE, bool LAZY_DEST>
void *
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::operator new (
    size_t size)
{
    ASSERTX(!data.destructed);

    lockFreeList(&data.mmFreeListLock[POOL]);

#ifdef MM_PREALLOC_MEMORY

    if (data.mmFreeList[POOL].empty())
    {
        // acquire memory for max # objects on first use of pool;
        // enhances memory locality of objects in the pool
        PreAllocateMemory();
    }

#endif

    MM_TYPE * newMmObj;
    if (! data.mmFreeList[POOL].empty())
    {
        newMmObj = data.mmFreeList[POOL].back();

        if (newMmObj->mmCnt == MMCNT_ON_FREELIST_NOT_DELETED)
        {
            data.FreeListObjectDestructor();
            // the contents of the free list might have changed, so we
            // need to get the new top of stack (ie. back())
            newMmObj = data.mmFreeList[POOL].back();
        }

        ASSERT(newMmObj->mmCnt == MMCNT_ON_FREELIST_AND_DELETED,
            "MM Object type " << data.className
            << " taken from free list has not yet been destructed!");

        data.mmFreeList[POOL].pop_back();

    }
    else
    {
      // acquire memory for 1 object on demand
      if (++data.mmTotalObjs[POOL] > data.mmMaxObjs)
      {
          cout << "MEMORY FAILURE: mmMaxObjs (" << data.mmMaxObjs << ")"
               << " for " << data.className << " exceeded. Pool " << POOL << endl;

#ifdef MM_OBJ_DUMP
          data.ObjDump();
#endif
          ASSERTX(false);
      }

      newMmObj = ((MM_TYPE *) new char[size]);

#ifdef MM_OBJ_DUMP
      data.mmObjList[POOL].push_back(newMmObj);
#endif //MM_OBJ_DUMP

    }

    unlockFreeList(&data.mmFreeListLock[POOL]);

#ifdef MM_VALGRIND
    // make object address range writable, but containing invalid data
    VALGRIND_DISCARD (VALGRIND_MAKE_WRITABLE (newMmObj, size));
    // allow read+write on MM meta information
    VALGRIND_DISCARD (
        VALGRIND_MAKE_READABLE (
            (ASIM_MM_CLASS<MM_TYPE>*) newMmObj,
            sizeof (ASIM_MM_CLASS<MM_TYPE>)));
#endif

    newMmObj->mmOwnerThread = POOL;  // make a note of the original pool.
    return((void *) newMmObj);
}

/**
 * The delete operator does not really delete the object for MM types.
 * In fact, calling delete on an MM object is optional. This is to
 * support the case where it is not clear in an ASIM model where excatly the
 * last reference of an object will get destroyed. This can be data
 * dependent, e.g. in the memory system it depends on the cache
 * behavior which part of the machine holds on to an object the
 * longest. Since the ref counting already takes care of detecting
 * when the last reference to the object gets destroyed, that is also
 * the place where the object gets put on the free list.
 *
 * In this operator delete we merely check if the call to delete
 * happens at a legal place, ie. the current ref count has to be < 0.
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::operator delete (
    void * ptr,  ///< pointer to object memory
    size_t size) ///< size of memory to delete
{

    ASSERTX(ptr != NULL);
    MM_TYPE * obj = (MM_TYPE *) ptr;


    // object must be on free list already for delete operator to be legal
    if (obj->mmCnt >= 0)
    {
        cout << "mmCnt was " << obj->mmCnt << ".  Can't delete if mmCnt>=0" << endl;
        abort();
    }
    ASSERTX(obj->mmCnt < 0);
}

/**
 * Reset the maximum number of allowed objects to a new value.
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::SetMaxObjs (
    UINT32 max) ///< new max
{
    if (debugOn)
    {
        cout << data.className << "::data.mmMaxObjs is being changed." << endl
             << "Old Value = " << data.mmMaxObjs
             << ", New Value = " << max << endl;
    }
    if (data.mmTotalObjs[POOL] > max)
    {
        cerr << "ERROR: " << data.className
             << ": trying to change to max number of objects to "
             << max << " but there are already " << data.mmTotalObjs[POOL]
             << " allocated";
        ASSERTX(false);
    }

    data.mmMaxObjs = max;
}

/**
 * Dump internal MM data to cout
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::Dump (
    int seqNo)  ///< sequence number to print before object data
const
{
    cout << fmt("-10", seqNo, ": ") << fmt("15", data.className);
#ifdef ASIM_ENABLE_MM_DEBUG
    cout << " mmUid " << mmUid;
#else
    cout << "  N/A";
#endif
    cout << " mmCnt " << mmCnt << endl;
}

#ifdef ASIM_ENABLE_MM_DEBUG
/**
 * Check if this object is legal to access, i.e. if there are known
 * references to it. If not, an error is raised.
 */
template <class MM_TYPE, bool LAZY_DEST>
void
ASIM_MM_CLASS<MM_TYPE, LAZY_DEST>::MmCheckRefCnt (
    UINT32 line, ///< line number where this check is performed
    char *file)  ///< file name where this check is performed
const
{
    if (mmMagicKey != data.mmMagicKey)
    {
        // In order to prevent arbitrary references (pointers) to
        // objects that are not really of this MM type, we check that
        // the mmMagicKey of the object matches the mmMagicKey of the
        // object type it belongs to. This also allows us to check
        // violations such as running off the end of iterators and
        // storing such dereferenced iterators into smart pointers,
        // which can overwrite arbitrary memory locations.
        fflush(NULL); // flush all output streams
        cout << "ASSERTION MEMORY FAILURE in " << file
             << ", line " << line << endl;
        cout << "Object @ " << fmt_p(this) << " is not an MM object," << endl
             << "but you are trying to access it as an MM object of type "
             << data.className << endl;
        cout << "Magic key found is " << fmt_x(mmMagicKey)
             << " but expected " << fmt_x(data.mmMagicKey) << endl;
        fflush(NULL); // flush all output streams
        abort();
    }
    if (mmCnt < 0)
    {
        fflush(NULL); // flush all output streams
        cout << "ASSERTION MEMORY FAILURE in " << file
             << ", line " << line << endl;
        cout << "Object type " << data.className << " @ " << fmt_p(this)
             << " accessed/modified while on free list" << endl;
        Dump(0);
        fflush(NULL); // flush all output streams
        abort();
    }
}

#endif // ASIM_ENABLE_MM_DEBUG


#endif // _MM_

