/*
 *Copyright (C) 2002-2006 Intel Corporation
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
 * @brief Driver for "Architects Workbench", awb
 */

// generic
#include <string.h>

// ASIM core
#include "asim/syntax.h"

// ASIM public modules
#include "asim/provides/controller.h"

// ASIM local module
#include "awb.h"

bool statsOn = false; 

/*
 * File holding commands for awb to execute after initializing.
 */
char *awbCmdFile = "";
 
/*
 * If non-null, this is the full pathname of a workbench to
 * use in place of the default workbench.
 */
char *overrideWorkbench = NULL;


static ASIM_THREAD PmThreadDescToPtr (char *desc);


void
AWB_Initialize (void)
{
    EVENT(ASIM_DRAL_EVENT_CLASS::InitEvent());
  //  CMD_Stop();
}

void
AWB_Activate (void)
/*
 * Activate awb if it is not already running.
 */
{
  // CMD_ScheduleThread(PmThreadDescToPtr("0"),ACTION_NOW,0);
  // CMD_Stop();
  CMD_Start();
}




void
AWB_Progress (AWB_PROGRESSTYPE type, const char *args)
/*
 * Exit awb...
 */
{
}    


void
AWB_InformProgress (void)
/*
 * Notify the workbench of the progress events that are pending,
 * and wait for it to acknowledge that it has processed all the
 * events.
 */
{
}

void
AWB_Exit (void)
/*
 * Exit awb...
 */
{
}


static ASIM_THREAD
PmThreadDescToPtr (char *desc)
/*
 * Return pointer to the ASIM_THREAD object specified by 'desc'.
 * Return NULL on error.
 */
{
    ASIM_THREAD thread = NULL;
    if (sscanf(desc, "THREAD__%p", &thread) != 1)
        return(NULL);

    return(thread);
}

