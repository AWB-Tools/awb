/*****************************************************************************
 *
 * @brief Warm-up manager algorithm for instruction based models
 *
 * @author Michael Adler (alg created by Eric Borch)
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

// ASIM public modules
#include "asim/provides/warmup_manager.h"

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
    WARMUP_CLIENTS_CLASS clientInfo(nDataCallbacks != 0,
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

    CallPhaseCallbacks(WARMUP_CALLBACK_CLASS::WARMUP_START);

    //
    // Keep fetching warm-up info from feeder until no more is available.
    //
    bool warmUpMore = true;
    while (warmUpMore)
    {
        warmUpMore = false;
        for (WARMUP_HWC_LIST::iterator whwc = hwcs.begin();
             whwc != hwcs.end();
             whwc++)
        {
            HW_CONTEXT hwc = (*whwc)->hwc;
            WARMUP_INFO_CLASS wInfo;

            if (hwc->WarmUp(&wInfo))
            {
                if (ENABLE_WARMUP)
                {
                    //
                    // Got warm-up info from the feeder...
                    //
                    bool hadData = false;

                    // Is it a control transfer instruction?
                    if (wInfo.IsCtrlTransfer())
                    {
                        (*whwc)->nCtrlInits += 1;
                        WARMUP_INSTR_CLASS wInstr(wInfo.GetAsimInst());

                        CallInstrCallbacks(globalInstrCallbacks, hwc, &wInstr);
                        CallInstrCallbacks((*whwc)->instrCallbacks, hwc, &wInstr);

                        hadData = true;
                    }

                    if (wInfo.IsIFetch())
                    {
                        (*whwc)->nIFetchInits += 1;
                        WARMUP_IFETCH_CLASS wIFetch(wInfo.GetIFetchVA(),
                                                    wInfo.GetIFetchPA());

                        CallIFetchCallbacks(globalIFetchCallbacks, hwc, &wIFetch);
                        CallIFetchCallbacks((*whwc)->ifetchCallbacks, hwc, &wIFetch);

                        hadData = true;
                    }

                    // Is it a data reference?
                    for (UINT32 i = 0; i < wInfo.NLoads(); i++)
                    {
                        (*whwc)->nDataInits += 1;
                        WARMUP_DATA_CLASS wData(true,
                                                wInfo.GetLoadVA(i),
                                                wInfo.GetLoadPA(i),
                                                wInfo.GetLoadBytes(i));
                        if (wInfo.IsAsimInstValid())
                        {
                            wData.SetAsimInst(wInfo.GetAsimInst());
                        }
                        if (wInfo.IsIFetch())
                        {
                            wData.SetInstrAddr(wInfo.GetIFetchVA(),
                                               wInfo.GetIFetchPA());
                        }

                        CallDataCallbacks(globalDataCallbacks, hwc, &wData);
                        CallDataCallbacks((*whwc)->dataCallbacks, hwc, &wData);

                        hadData = true;
                    }

                    for (UINT32 i = 0; i < wInfo.NStores(); i++)
                    {
                        (*whwc)->nDataInits += 1;
                        WARMUP_DATA_CLASS wData(false,
                                                wInfo.GetStoreVA(i),
                                                wInfo.GetStorePA(i),
                                                wInfo.GetStoreBytes(i));
                        if (wInfo.IsAsimInstValid())
                        {
                            wData.SetAsimInst(wInfo.GetAsimInst());
                        }
                        if (wInfo.IsIFetch())
                        {
                            wData.SetInstrAddr(wInfo.GetIFetchVA(),
                                               wInfo.GetIFetchPA());
                        }

                        CallDataCallbacks(globalDataCallbacks, hwc, &wData);
                        CallDataCallbacks((*whwc)->dataCallbacks, hwc, &wData);

                        hadData = true;
                    }

                    if (! hadData)
                    {
                        (*whwc)->nEmptyInits += 1;
                    }
                }
                
                warmUpMore = true;
            }
        }

        if (warmUpMore && ENABLE_WARMUP)
        {
            CallTickCallbacks();
        }
    }

    CallPhaseCallbacks(WARMUP_CALLBACK_CLASS::WARMUP_END);

    T1("Warmup:  Exit"); 
}
