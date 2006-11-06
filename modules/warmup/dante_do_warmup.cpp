/*****************************************************************************
 *
 * @brief Warm-up manager algorithm for Dante/Asim models
 *
 * @author Eric Borch
 * 
 *Copyright (C) 2006 Intel Corporation
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

#include <sstream>
#include <string.h>

// ASIM public modules
#include "asim/provides/warmup_manager.h"

#include "dante_export.h"

void
WARMUP_MANAGER_CLASS::DoWarmUp(void)
{
    T1("Warmup:  Enter"); 

    //
    // Start by telling all HWCs what information has been requested.
    // An intelligent feeder can then limit the information returned
    // only to what is needed.
    //
    // Every HWC is called in case there is more than one feeder active,
    // since we can't easily figure out which feeder is bound to each
    // HWC.
    //
/*    WARMUP_CLIENTS_CLASS clientInfo(nDataCallbacks != 0,
                                    nIFetchCallbacks != 0,
                                    nInstrCallbacks != 0);

    if (! ENABLE_WARMUP)
    {
        // Warm-up is disabled.
        clientInfo = WARMUP_CLIENTS_CLASS(false, false, false);
    }

    for (WARMUP_HWC_LIST::iterator whwc = hwcs.begin();
         whwc != hwcs.end();
         whwc++)
    {
        HW_CONTEXT hwc = (*whwc)->hwc;
        hwc->WarmUpClientInfo(&clientInfo);
    }
*/
//    CallPhaseCallbacks(WARMUP_CALLBACK_CLASS::WARMUP_START);

    if (NUM_WARMUP_INSTR > 0)
    {
        ostringstream warmup_str;
        warmup_str << "warm " << NUM_WARMUP_INSTR;
        
        // apparently the ostringstream c_str() returns a "const char *", but
        // DanteExecuteString wants a "char *".  So, cast away const-ness.
        DanteExecuteString(const_cast<char *>(warmup_str.str().c_str()));
    }

//    CallPhaseCallbacks(WARMUP_CALLBACK_CLASS::WARMUP_END);

    T1("Warmup:  Exit"); 
}
