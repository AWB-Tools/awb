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

using namespace std;


// Some globals for threads.  we don't want to be passing a object's owner's
// threadID all, and if there's an obvious way to have distinct IDs available,
// I'm overlooking it.

pthread_key_t asim_thread_id_key;

void free_asim_thread_id(void * arg)
{
    INT32 * id = (INT32 * )arg;

    delete id;
}

void init_asim_thread_id()
{    
    int rval = pthread_key_create(&asim_thread_id_key, free_asim_thread_id);
    
    VERIFYX(rval==0);
}

INT32 get_asim_thread_id()
{
    INT32 * id = NULL;        

    id = (INT32 *)pthread_getspecific(asim_thread_id_key);

    if(id==NULL)  // This means that thread 0, master, was calling.
    {
        return 0;
    }
   
    return *id;
}

INT32 set_asim_thread_id(UINT32 uniqueThreadID)
{
    INT32 * id = new INT32;  //will free this with free_asim_thread_id()_
    *id = (INT32)uniqueThreadID;

    VERIFYX(pthread_getspecific(asim_thread_id_key) == NULL);
    int rval = pthread_setspecific(asim_thread_id_key, (void *)id);
    VERIFYX(rval==0);
    
    return *id;
}

