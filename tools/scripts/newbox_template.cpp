//
// Copyright (C) 2002-2006 Intel Corporation
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

//
// Author:  Roger Espasa
//


#include "XX_PROVIDES_XX.h"


XX_UBOX_XX_CLASS::XX_BOX_XX_class (ASIM_MODULE p, CHAR *n, ASIM_EXCEPT except) : ASIM_MODULE_CLASS(p, n, except)
{

 // 
 // Create Buffers
 //
XX_CREATE_BUFFERS_XX


 //
 // Register Events
 //
XX_REGISTER_EVENTS_XX

 //
 // Register Exceptions
 //
XX_REGISTER_EXCEPTIONS_XX
}

XX_UBOX_XX_CLASS::~XX_BOX_XX_class ()
{
XX_DELETE_BUFFERS_XX
}

BOOL
XX_UBOX_XX_CLASS::InitModule (VOID)
{
 BOOL ok = TRUE;

 printf("Init Module %s\n",Name());

XX_PRINT_BUF_INFO_XX

 printf("Init Module %s: Initing Submodules:\n",Name());

 if ( !ok ) printf("%s: InitModule failed\n", Name());

 return ok;
}


ASIM_BUFFER *
XX_UBOX_XX_CLASS::GetBufferPtr (UINT32 buf)
{
 switch (buf) {
XX_GET_BUFFER_PTR_XX
  default: ASIMERROR("%s does not contain buffer "FMT32U".\n", Name(), buf);
 }

 return(NULL);  // unreachable
}


//
// Handle exceptions...
//
VOID
XX_UBOX_XX_CLASS::HandleException (ASIM_EXCEPTDATA data, UINT64 cycle)
{   
 TRACE(XX_TRACE_XX, printf("\t%lu: %s received exception type %u\n", cycle, Name(), data->Type()));

 switch (data->Type()) {
XX_EXCEPTION_XX

  default:
    ASIMERROR("%s received unexpected exception type %d\n", Name(), data->Type());
 }
}

//
// Do a cycle worth of work
//
VOID
XX_UBOX_XX_CLASS::Clock (UINT64 cycle)
{
 TRACE(XX_TRACE_XX, printf("%lu: %s\n", cycle, Name()));
    
 //
 // Check input buffers to see if they are empty
 //
XX_CHECK_BUF_XX
}
