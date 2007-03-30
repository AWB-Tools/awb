/*****************************************************************************
 *
 * @brief Definitions to support pthreads implemenation.
 *
 * @author Kenneth Barr, Ramon Matas Navarro
 * 
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


#include "asim/smp.h"
#include "asim/atomic.h"
#include <iostream>
#include "asim/mesg.h"


ATOMIC32_CLASS ASIM_SMP_THREAD_HANDLE_CLASS::threadUidGen = 0;

ASIM_SMP_THREAD_HANDLE ASIM_SMP_CLASS::mainThread;
pthread_key_t ASIM_SMP_CLASS::threadLocalKey;
UINT32 ASIM_SMP_CLASS::maxThreads = 0;
ATOMIC32_CLASS ASIM_SMP_CLASS::activeThreads = 0;

#ifdef TLS_AVAILABLE
__thread INT32 ASIM_SMP_RunningThreadNumber = 0;
#endif


ASIM_SMP_THREAD_HANDLE_CLASS::ASIM_SMP_THREAD_HANDLE_CLASS()
    : threadCreateArg(NULL),
      threadNumber(-1)
{
    threadId = threadUidGen++;
};


void
ASIM_SMP_THREAD_HANDLE_CLASS::NoteThreadActive(INT32 tNum)
{
    VERIFYX(threadNumber == -1);
    threadNumber = tNum;
};



void
ASIM_SMP_CLASS::Init(
    UINT32 staticMaxThreads,
    UINT32 dynamicMaxThreads)
{
    //
    // Dynamic maximum number of threads configured in the model.  If
    // dynamicMaxThreads is 0 then use staticMaxThreads.
    //
    if (dynamicMaxThreads > 0)
    {
        VERIFYX(dynamicMaxThreads <= staticMaxThreads);
        maxThreads = dynamicMaxThreads;
    }
    else
    {
        maxThreads = staticMaxThreads;
    }

    mainThread = new ASIM_SMP_THREAD_HANDLE_CLASS();
    mainThread->NoteThreadActive(activeThreads++);
#ifdef TLS_AVAILABLE
    ASIM_SMP_RunningThreadNumber = mainThread->threadNumber;
#endif

    VERIFY(mainThread->GetThreadId() == 0, "ASIM_SMP_CLASS::Init() called more than once");

    VERIFYX(0 == pthread_key_create(&threadLocalKey, NULL));
    VERIFYX(0 == pthread_setspecific(threadLocalKey, mainThread));

    VERIFYX(GetRunningThreadNumber() == 0);
}


void
ASIM_SMP_CLASS::CreateThread(
    pthread_t *thread,
    pthread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg,
    ASIM_SMP_THREAD_HANDLE threadHandle)
{
    threadHandle->start_routine = start_routine;
    threadHandle->threadCreateArg = arg;
    threadHandle->threadNumber = activeThreads++;

    VERIFY(UINT32(threadHandle->threadNumber) < ASIM_SMP_CLASS::GetMaxThreads(),
           "Thread limit exceeded (" << ASIM_SMP_CLASS::GetMaxThreads() << ")");

    VERIFYX(0 == pthread_create(thread, attr, &ThreadEntry, threadHandle));
}


void *
ASIM_SMP_CLASS::ThreadEntry(void *arg)
{
    //
    // All threads enter through this function...
    //

    ASIM_SMP_THREAD_HANDLE threadHandle = ASIM_SMP_THREAD_HANDLE(arg);

#ifdef TLS_AVAILABLE
    ASIM_SMP_RunningThreadNumber = threadHandle->threadNumber;
#endif

    VERIFYX(0 == pthread_setspecific(threadLocalKey, threadHandle));

    cout << "Thread " << GetRunningThreadNumber() << " created." << endl;
    
    // Call the real entry function
    return (*(threadHandle->start_routine))(threadHandle->threadCreateArg);
}


ASIM_SMP_THREAD_HANDLE
ASIM_SMP_CLASS::GetThreadHandle(void)
{
    ASIM_SMP_THREAD_HANDLE threadHandle =
        (ASIM_SMP_THREAD_HANDLE) pthread_getspecific(threadLocalKey);

    VERIFYX(threadHandle != NULL);
    return threadHandle;
}
