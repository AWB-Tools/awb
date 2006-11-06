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
 * @author David Goodwin
 * @brief Asynchronous exception delivery (multicast).
 */

#ifndef _EXCEPT_
#define _EXCEPT_

// generic
#include <string.h>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"

#define MAX_EXCEPTIONS 	64

typedef class ASIM_MODULE_CLASS *ASIM_MODULE;
typedef class ASIM_EXCEPTDATA_CLASS *ASIM_EXCEPTDATA;


/*
 * ASIM_EXCEPTION
 *
 * Type for exceptions.
 */
typedef UINT32 ASIM_EXCEPTION;


/*
 * Class ASIM_EXCEPTDATA
 *
 * Exception data sent when a module is notified of
 * an exception.
 */

typedef class ASIM_EXCEPTDATA_CLASS *ASIM_EXCEPTDATA;
class ASIM_EXCEPTDATA_CLASS
{
    private:
        static UINT64 uniqueExceptId;
        
        const UINT64 uid;
        const ASIM_EXCEPTION type;

    public:
        ASIM_EXCEPTDATA_CLASS (const ASIM_EXCEPTION t) 
	    : uid(uniqueExceptId), type(t)
	{
            uniqueExceptId++;
        }

        virtual ~ASIM_EXCEPTDATA_CLASS () { }

        /*
         * Accessors...
         */
        UINT32 Type (void) const { return(type); }
        UINT64 Uid (void) const { return(uid); }

};



/*
 * Class ASIM_EXCEPT
 *
 * Exception handler. Relays exceptions to modules
 * that have registered to recieve those exceptions.
 *
 */
typedef class ASIM_EXCEPT_CLASS *ASIM_EXCEPT;
class ASIM_EXCEPT_CLASS
{
    public:
        /*
         * Register that module 'm' wants to be informed of
         * 'xcp' type exceptions.
         */
        virtual void Register (ASIM_MODULE m, ASIM_EXCEPTION xcp) =0;

        /*
         * Notify all modules of exception 'data'.
         */
        virtual void Raise (ASIM_EXCEPTDATA data, UINT64 cycle) =0;
            
};

#endif /* _EXCEPT_ */
