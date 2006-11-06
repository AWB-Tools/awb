/*
 * Copyright (C) 1998-2006 Intel Corporation
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
 */

//
// Author:  Roger Espasa
//

/********************************************************************
 *
 * Awb definitions
 *
 * %AWB_START
 *
 * %name 
 * %desc
 * %provides XX_PROVIDES_XX
 * %public XX_BOX_XX.h
 * %private XX_BOX_XX.cpp
 *
 * %AWB_END
 *******************************************************************/


#ifndef _XX_UBOX_XX_
#define _XX_UBOX_XX_

// ASIM core
#include "syntax.h"
#include "mesg.h"
#include "module.h"

// ASIM public modules
#include "asim/provides/aranainst.h"
#include "asim/provides/cpubufs.h"


XX_BUFFER_ENUM_XX



typedef class XX_BOX_XX_class : public ASIM_MODULE_CLASS {

    private:

        //
        // Modules contained in this box
        //

        //
        // Buffers...
        //
	XX_BUFFER_DECLARATIONS_XX

	//
	// Events
	//
        XX_EVENT_DECLARATIONS_XX

	//
	// Variables
	//
 
	//
	// Private functions
	//

    public:

	// Constructor
        XX_BOX_XX_class (ASIM_MODULE p, CHAR *n, ASIM_EXCEPT e);

	// Destructor
        ~XX_BOX_XX_class ();

	// Required by ASIM
	BOOL InitModule (VOID);

	// Required by ASIM
        ASIM_BUFFER *GetBufferPtr (UINT32 buf);

        // Handle exceptions...
        VOID HandleException (ASIM_EXCEPTDATA data, UINT64 cycle);

        // Cycle of work...
        VOID Clock (UINT64 cycle);


} XX_UBOX_XX_CLASS, *XX_UBOX_XX;


#endif // XX_UBOX_XX 
