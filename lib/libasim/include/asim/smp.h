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
#include "asim/mesg.h"

// Some globals for threads.  we don't want to be passing a object's owner's
// threadID all, and if there's an obvious way to have distinct IDs available,
// I'm overlooking it.

extern pthread_key_t asim_thread_id_key;

void free_asim_thread_id(void * arg);

void init_asim_thread_id();
INT32 set_asim_thread_id( UINT32 uniqueThreadID );

INT32 get_asim_thread_id();

#endif

