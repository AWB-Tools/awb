/*****************************************************************************
 *
 * @brief Declarations to support pthreads implemenation.
 *
 * @author Kenneth Barr
 * @todo use ASIM infrastructure to set number of threads instead of #define
 * 
 * @note
 * Files that have thread safety issues should include this header so they
 * can call get_asim_thread_id() and find out about the number of implementation threads
 *
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
 *
 *****************************************************************************/

#ifndef ASIM_SMP_H
#define ASIM_SMP_H

#include "asim/atomic.h"
#include <pthread.h>


//
// SMP_THREAD_HANDLE_CLASS --
//   This class holds thread-local storage.
//
class ASIM_SMP_THREAD_HANDLE_CLASS
{
  public:
    ASIM_SMP_THREAD_HANDLE_CLASS();
    ~ASIM_SMP_THREAD_HANDLE_CLASS() {};

    //
    // The running thread id is a dense thread numbering of running threads.
    // Valid values are between 0 and MaxThreads-1.  Thread numbers are not
    // assigned until a thread is finally created for a handle.  Use this
    // value if you want dense IDs of running threads.
    // 
    INT32 GetRunningThreadNumber() const
    {
        return (ThreadIsRunning() ? threadNumber : -1);
    };

    //
    // Thread ID is assigned when a thread handle is constructed.  If no
    // pthread is ever created for a thread, the thread ID numbers will grow
    // larger than the thread numbers described above.  Use the thread ID
    // if you need a unique value for all thread handles.
    //
    UINT32 GetThreadId() const { return threadId; };

    bool ThreadIsRunning() const { return threadLive; };

    friend class ASIM_SMP_CLASS;

  private:
    // Only allow ASIM_SMP_CLASS to claim that a thread is running
    void NoteThreadStart();

    typedef void* voidp;
    volatile voidp threadCreateArg;
    volatile UINT32 threadId;
    ATOMIC_INT32 threadLive;
    volatile INT32 threadNumber;

    // This helps ThreadEntry find the right entry function for the thread
    void *(*start_routine)(void *);

    static ATOMIC32_CLASS threadUidGen;
};

typedef ASIM_SMP_THREAD_HANDLE_CLASS *ASIM_SMP_THREAD_HANDLE;



class ASIM_SMP_CLASS
{
  public:
    //
    // Init --
    //  Systems usually have two parameters:  MAX_THREADS is the static maximum
    //  number of threads allowed.  LIMIT_THREADS is a dynamic parameter that
    //  may reduce the thread limit for a given run.  Init() accepts both
    //  to avoid replicating the same code through all systems.
    //
    static void Init(UINT32 staticMaxThreads,
                     UINT32 dynamicMaxThreads);

    static void CreateThread(
        pthread_t *thread,
        pthread_attr_t *attr,
        void *(*start_routine)(void *),
        void *arg,
        ASIM_SMP_THREAD_HANDLE threadHandle);

    static ASIM_SMP_THREAD_HANDLE GetThreadHandle(void);

    //
    // Total of all running threads
    //
    static UINT32 GetTotalRunningThreads(void) { return activeThreads; };

    //
    // Total of all thread handles, including those not running.
    //
    static UINT32 GetTotalThreadHandles(void)
    {
        return ASIM_SMP_THREAD_HANDLE_CLASS::threadUidGen;
    };

    static UINT32 GetRunningThreadNumber(void)
    {
        if (GetTotalRunningThreads() <= 1)
        {
            return 0;
        }
        else
        {
            return GetThreadHandle()->GetRunningThreadNumber();
        }
    };

    static UINT32 GetMaxThreads(void) { return maxThreads; };

    static ASIM_SMP_THREAD_HANDLE GetMainThreadHandle(void) { return mainThread; };

  private:
    static ASIM_SMP_THREAD_HANDLE mainThread;

    static pthread_key_t threadLocalKey;

    static void *ThreadEntry(void *arg);

    static UINT32 maxThreads;
    static ATOMIC32_CLASS activeThreads;
};

#endif

