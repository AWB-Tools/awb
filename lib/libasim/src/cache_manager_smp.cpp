/*
 * **********************************************************************
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

/*****************************************************************************
*
* @brief Source file for thread safe Cache Manager
*
* @author Carl Beckmann
*
*****************************************************************************/

#include "asim/cache_manager_smp.h"


//
// the constructor and destructor need to initialize, or destroy, the threading lock
//
CACHE_MANAGER_SMP::CACHE_MANAGER_SMP()
{
    pthread_mutex_init( &mutex, NULL );
}

CACHE_MANAGER_SMP::~CACHE_MANAGER_SMP()
{
    pthread_mutex_destroy( &mutex );
}

//
// macros for entering/exiting the critical section
//
#define ENTER_CACHE_MANAGER pthread_mutex_lock  ( &mutex )
#define LEAVE_CACHE_MANAGER pthread_mutex_unlock( &mutex )

//
// return a ref to the singleton instance of this class
//
CACHE_MANAGER&
CACHE_MANAGER_SMP::GetInstance()
{
    static CACHE_MANAGER_SMP the_manager;
    return the_manager;
}

//
// these methods simply call the inherited methods inside a critical section
//
void
CACHE_MANAGER_SMP::Register(std::string level)
{
    ENTER_CACHE_MANAGER;
    CACHE_MANAGER::Register( level );
    LEAVE_CACHE_MANAGER;
}

CACHE_MANAGER::LINE_MANAGER *
CACHE_MANAGER_SMP::find_line_manager(std::string level)
{
    LINE_MANAGER *line_manager;
    ENTER_CACHE_MANAGER;
    line_manager = CACHE_MANAGER::find_line_manager( level );
    LEAVE_CACHE_MANAGER;
    return line_manager;
}
