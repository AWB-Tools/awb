/*****************************************************************************
 *
 * @brief Warm-up manager for instruction based models
 *
 * @author Michael Adler
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

// ASIM core
#include "asim/syntax.h"

// ASIM public modules
#include "asim/provides/warmup_manager.h"
#include "asim/provides/hardware_context.h"
#include "asim/provides/instfeeder_interface.h"

WARMUP_MANAGER_CLASS::WARMUP_MANAGER_CLASS(
    ASIM_MODULE parent,
    const char *name)
    : ASIM_MODULE_CLASS(parent, name),
      nDataCallbacks(0),
      nIFetchCallbacks(0),
      nInstrCallbacks(0)
{
}

//
// Note: The WARMUP_MANAGER_CLASS:DoWarmUp(void) method is now in
// warmup_instrs_alg.cpp.  This needed to be separated out so that
// we could have alternate methods for warming models that do not use Asim
// feeders.  Eric


WARMUP_MANAGER_CLASS::~WARMUP_MANAGER_CLASS()
{
    for (WARMUP_HWC_LIST::iterator whwc = hwcs.begin();
         whwc != hwcs.end();
         whwc++)
    {
        delete (*whwc);
    }
}

void
WARMUP_MANAGER_CLASS::RegisterHWC(HW_CONTEXT hwc)
{
    T1("Warmup:  Register HWC uid=" << hwc->GetUID()); 

    WARMUP_HWC whwc = new WARMUP_HWC_CLASS(hwc);
    hwcs.push_back(whwc);

    RegisterState(&whwc->hwcUID, "warmupHwcUID",
                  "Hardware context UID");
    RegisterState(&whwc->nDataInits, "warmupDataRefs",
                  "Number of data references parsed during warm-up");
    RegisterState(&whwc->nIFetchInits, "warmupIFetchRefs",
                  "Number of instruction fetches parsed during warm-up");
    RegisterState(&whwc->nCtrlInits, "warmupCtrlRefs",
                  "Number of control transfer instructions parsed during warm-up");
    RegisterState(&whwc->nEmptyInits, "warmupEmptyRefs",
                  "Number of positive responses from feeder with no warm-up data");
}


void
WARMUP_MANAGER_CLASS::RegisterForPhases(
    WARMUP_CALLBACK cbk)
{
    globalPhaseCallbacks.push_back(cbk);
}


void
WARMUP_MANAGER_CLASS::RegisterForTicks(
    WARMUP_CALLBACK cbk)
{
    globalTickCallbacks.push_back(cbk);
}


void
WARMUP_MANAGER_CLASS::RegisterForData(
    WARMUP_CALLBACK cbk,
    HW_CONTEXT hwc)
{
    nDataCallbacks += 1;

    if (hwc == NULL)
    {
        globalDataCallbacks.push_back(cbk);
    }
    else
    {
        FindHWC(hwc)->dataCallbacks.push_back(cbk);
    }
}


void
WARMUP_MANAGER_CLASS::RegisterForIFetch(
    WARMUP_CALLBACK cbk,
    HW_CONTEXT hwc,
    UINT32 lineBytes)
{
    IFETCH_CALLBACK ifCbk = new IFETCH_CALLBACK_CLASS(cbk, lineBytes);

    nIFetchCallbacks += 1;

    if (hwc == NULL)
    {
        globalIFetchCallbacks.push_back(ifCbk);
    }
    else
    {
        FindHWC(hwc)->ifetchCallbacks.push_back(ifCbk);
    }
}


void
WARMUP_MANAGER_CLASS::RegisterForInstrs(
    WARMUP_CALLBACK cbk,
    HW_CONTEXT hwc)
{
    nInstrCallbacks += 1;

    if (hwc == NULL)
    {
        globalInstrCallbacks.push_back(cbk);
    }
    else
    {
        FindHWC(hwc)->instrCallbacks.push_back(cbk);
    }
}



WARMUP_MANAGER_CLASS::WARMUP_HWC_CLASS::WARMUP_HWC_CLASS(HW_CONTEXT hwc)
    : hwc(hwc),
      nDataInits(0),
      nIFetchInits(0),
      nCtrlInits(0),
      nEmptyInits(0)
{
    hwcUID = hwc->GetUID();
};


WARMUP_MANAGER_CLASS::WARMUP_HWC_CLASS::~WARMUP_HWC_CLASS()
{
    for (IFETCH_CALLBACK_LIST::iterator i = ifetchCallbacks.begin();
         i != ifetchCallbacks.end();
         i++)
    {
        delete (*i);
    }

    return;
}

// ---------------------------------------------------------------------
// WARMUP_INFO_CLASS --
// ---------------------------------------------------------------------

//
// Allocate an ASIM_INST.  Should be called by a feeder.
//
ASIM_MACRO_INST
WARMUP_INFO_CLASS::InitAsimInst(void)
{
    ASSERTX(aInst == NULL);
    ASSERTX(swContext != NULL);

    aInst = new ASIM_MACRO_INST_CLASS(swContext);
    return aInst;
};
