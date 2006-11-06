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

#ifndef _WARMUP_INSTRS_
#define _WARMUP_INSTRS_

#include <list>

// ASIM core
#include "asim/syntax.h"
#include "asim/module.h"
#include "asim/stateout.h"

// ASIM public modules
#include "asim/provides/basesystem.h"
#include "asim/provides/isa.h"

/*
 * Class WARMUP_INFO
 *
 */
typedef class WARMUP_INFO_CLASS *WARMUP_INFO;
typedef class WARMUP_CLIENTS_CLASS *WARMUP_CLIENTS;
typedef class WARMUP_DATA_CLASS *WARMUP_DATA;
typedef class WARMUP_IFETCH_CLASS *WARMUP_IFETCH;
typedef class WARMUP_INSTR_CLASS *WARMUP_INSTR;
typedef class WARMUP_CALLBACK_CLASS *WARMUP_CALLBACK;
typedef class WARMUP_MANAGER_CLASS *WARMUP_MANAGER;

typedef class HW_CONTEXT_CLASS * HW_CONTEXT;


//
// WARMUP_CLIENTS_CLASS describes the information requested by clients of
// the warm-up manager.  It is passed to feeders as a hint so the feeders
// can filter the data returned to the manager in order to make warm-up
// faster.
//
class WARMUP_CLIENTS_CLASS
{
  public:
    //
    // Default values for the constructor should be no warm-up required.
    //
    WARMUP_CLIENTS_CLASS(
        bool monitorDCache = false,
        bool monitorICache = false,
        bool monitorInstrs = false)
        : monitorDCache(monitorDCache),
          monitorICache(monitorICache),
          monitorInstrs(monitorInstrs)
    {};

    bool MonitorDCache(void) const { return monitorDCache; };
    bool MonitorICache(void) const { return monitorICache; };
    bool MonitorInstrs(void) const { return monitorInstrs; };

    bool operator== (const WARMUP_CLIENTS_CLASS& cmp) const;

  private:
    bool monitorDCache;
    bool monitorICache;
    bool monitorInstrs;
};

inline bool
WARMUP_CLIENTS_CLASS::operator== (const WARMUP_CLIENTS_CLASS& cmp) const
{
    return (monitorDCache == cmp.monitorDCache) &&
           (monitorICache == cmp.monitorICache) &&
           (monitorInstrs == cmp.monitorInstrs);
};


//
// WARMUP_DATA_CLASS describes a load or store and is passed to registered
// data reference clients of the warm-up manager.
//
class WARMUP_DATA_CLASS
{
  public:
    WARMUP_DATA_CLASS(
        bool isLoad,
        UINT64 va,
        UINT64 pa,
        UINT32 bytes)
        : aInst(NULL),
          va(va),
          pa(pa),
          bytes(bytes),
          isLoad(isLoad),
          isIAddrValid(false)
    {};
    
    ~WARMUP_DATA_CLASS() {};

    UINT64 GetVA(void) const { return va; };
    UINT64 GetPA(void) const { return pa; };
    UINT32 GetBytes(void) const { return bytes; };

    //
    // A feeder may return an ASIM_MACRO_INST even for a data reference.  For
    // feeders that supply cache line values the ASIM_MACRO_INST is needed to
    // call the feeder's GetCacheLineValueFollowingInst().
    //
    void SetAsimInst(ASIM_MACRO_INST ainst) { aInst = ainst; };
    bool IsAsimInstValid(void) const { return aInst != NULL; };
    ASIM_MACRO_INST GetAsimInst(void) const
    {
        ASSERTX(aInst != NULL);
        return aInst;
    };
    
    //
    // A feeder may supply the address of the instruction generating the
    // data reference.  It can be passed to clients using these methods:
    //
    void SetInstrAddr(UINT64 iVA, UINT64 iPA)
    {
        isIAddrValid = true;
        instrVA = iVA;
        instrPA = iPA;
    }
    bool IsInstrAddrValid(void) const { return isIAddrValid; }
    UINT64 GetInstrVA(void) const { return instrVA; }
    UINT64 GetInstrPA(void) const { return instrPA; }

    bool IsLoad() { return isLoad; }

  private:
    ASIM_MACRO_INST aInst;

    UINT64 instrVA;
    UINT64 instrPA;

    const UINT64 va;
    const UINT64 pa;
    const UINT32 bytes;

    const bool isLoad;
    bool isIAddrValid;
};
    

//
// WARMUP_IFETCH_CLASS describes instruction fetches.  A shared instruction
// and data cache should register for both warm-up data references and ifetch.
//
// The feeder returns only a VA and PA, not the size of an instruction since
// only the model knows the line size and fetch rules.  If an instruction might
// span a page the feeder should claim to fetch from both pages, though
// getting this wrong in the warm-up code probably won't be too harmful.
//
//
class WARMUP_IFETCH_CLASS
{
  public:
    WARMUP_IFETCH_CLASS(
        UINT64 va,
        UINT64 pa)
        : va(va),
          pa(pa)
    {};

    ~WARMUP_IFETCH_CLASS() {};

    UINT64 GetVA(void) const { return va; };
    UINT64 GetPA(void) const { return pa; };

  private:
    const UINT64 va;
    const UINT64 pa;
};


//
// WARMUP_INSTR_CLASS describes a control instruction for warming up
// branch, return or other predictors.
//
class WARMUP_INSTR_CLASS
{
  public:
    WARMUP_INSTR_CLASS(
        ASIM_MACRO_INST aInst)
        : aInst(aInst)
    {};
    
    ~WARMUP_INSTR_CLASS() {};

    ASIM_MACRO_INST GetAsimInst(void) const { return aInst; };

  private:
    const ASIM_MACRO_INST aInst;
};
    

//
// Classes that need to receive warm-up data must be derived from the
// WARMUP_CALLBACK_CLASS and must declare their own virtual function
// to handle the warm-up data.
//
class WARMUP_CALLBACK_CLASS
{
  public:
    WARMUP_CALLBACK_CLASS(void) {};
    virtual ~WARMUP_CALLBACK_CLASS() {};

    enum WARMUP_PHASE
    {
        WARMUP_START,
        WARMUP_END
    };

    virtual void WarmUpPhase(WARMUP_PHASE phase)
    {
        ASIMERROR("No warm-up phase handler defined in derived class");
    };

    virtual void WarmUpTick(void)
    {
        ASIMERROR("No warm-up tick handler defined in derived class");
    };

    virtual void WarmUpData(HW_CONTEXT hwc, const WARMUP_DATA wData)
    {
        ASIMERROR("No warm-up data handler defined in derived class");
    };

    virtual void WarmUpIFetch(HW_CONTEXT hwc, const WARMUP_IFETCH wIfetch)
    {
        ASIMERROR("No warm-up instruction fetch handler defined in derived class");
    };

    virtual void WarmUpInstr(HW_CONTEXT hwc, const WARMUP_INSTR wInstr)
    {
        ASIMERROR("No warm-up instrunction handler defined in derived class");
    };
};


//
// WARMUP_INFO_CLASS is a private class solely for communication between
// the warm-up manager and a feeder.  Data is extracted from the class
// by the warm-up manager and handed out to clients of the warm-up manager
// using other data structures depending on the type of warm-up information.
//
class WARMUP_INFO_CLASS
{
  public:
    WARMUP_INFO_CLASS(void)
        : aInst(NULL),
          swContext(NULL),
          nLoads(0),
          nStores(0),
          isIFetch(false),
          isCtrlTransfer(false)
    {};

    ~WARMUP_INFO_CLASS() {};

    bool IsIFetch(void) const { return isIFetch; };
    bool IsDataRef(void) const { return IsLoad() || IsStore(); };
    bool IsLoad(void) const { return (nLoads > 0); };
    bool IsStore(void) const { return (nStores > 0); };
    bool IsCtrlTransfer(void) const { return isCtrlTransfer; };

    UINT64 GetIFetchVA(void) const { ASSERTX(IsIFetch()); return instrVA; };
    UINT64 GetIFetchPA(void) const { ASSERTX(IsIFetch()); return instrPA; };

    UINT32 NLoads(void) const { return nLoads; };
    UINT64 GetLoadVA(UINT32 n) const { ASSERTX(nLoads > n); return loads[n].va; };
    UINT64 GetLoadPA(UINT32 n) const { ASSERTX(nLoads > n); return loads[n].pa; };
    UINT32 GetLoadBytes(UINT32 n) const { ASSERTX(nLoads > n); return loads[n].nBytes; };

    UINT32 NStores(void) const { return nStores; };
    UINT64 GetStoreVA(UINT32 n) const { ASSERTX(nStores > n); return stores[n].va; };
    UINT64 GetStorePA(UINT32 n) const { ASSERTX(nStores > n); return stores[n].pa; };
    UINT32 GetStoreBytes(UINT32 n) const { ASSERTX(nStores > n); return stores[n].nBytes; };

    //
    // Call to indicate instruction fetch
    //
    void NoteIFetch(
        UINT64 va,
        UINT64 pa)
    {
        isIFetch = true;
        instrVA = va;
        instrPA = pa;
    };

    //
    // Call to indicate warm-up is a data reference
    //
    void NoteLoad(
        UINT64 va,
        UINT64 pa,
        UINT32 bytes)
    {
        // If an instruction refers to too many address drop them on the floor

        if (nLoads < MAX_MEM_REFS)
        {
            loads[nLoads].va = va;
            loads[nLoads].pa = pa;
            loads[nLoads].nBytes = bytes;
            nLoads += 1;
        }
    };

    void NoteStore(
        UINT64 va,
        UINT64 pa,
        UINT32 bytes)
    {
        // If an instruction refers to too many address drop them on the floor

        if (nStores < MAX_MEM_REFS)
        {
            stores[nStores].va = va;
            stores[nStores].pa = pa;
            stores[nStores].nBytes = bytes;
            nStores += 1;
        }
    };

    //
    // Call to indicate warm-up is a control transfer instruction.
    // Source and target addresses are stored in the ASIM_MACRO_INST.
    //
    void NoteCtrlTransfer(void) { isCtrlTransfer = true; };

    //
    // SW Context is required by InitAsimInst to allocate an ASIM_MACRO_INST.
    // The SW context should call this as the warm-up request passes through.
    //
    void NoteSWContext(SW_CONTEXT swc) { swContext = swc; };
    SW_CONTEXT GetSWC() const { return swContext; };

    //
    // Allocate an ASIM_INST.  Should be called by a feeder.
    //
    ASIM_MACRO_INST InitAsimInst(void);

    bool IsAsimInstValid(void) const { return aInst != NULL; };
    ASIM_MACRO_INST GetAsimInst(void) const
    {
        ASSERT(IsAsimInstValid(), "ASIM_MACRO_INST was never initialized");
        return aInst;
    };

  private:
    ASIM_MACRO_INST aInst;
    SW_CONTEXT swContext;

    UINT64 instrVA;
    UINT64 instrPA;

    struct MEM_REF
    {
        UINT64 va;
        UINT64 pa;
        UINT32 nBytes;
    };

    enum
    {
        MAX_MEM_REFS = 20
    };

    MEM_REF loads[MAX_MEM_REFS];
    MEM_REF stores[MAX_MEM_REFS];
    UINT32 nLoads;
    UINT32 nStores;

    bool isIFetch;
    bool isCtrlTransfer;
};


class WARMUP_MANAGER_CLASS : public ASIM_MODULE_CLASS
{
  public:
    WARMUP_MANAGER_CLASS(ASIM_MODULE parent, const char *name);
    ~WARMUP_MANAGER_CLASS() {};

    // Called by a system
    void DoWarmUp(void);

  public:
    // Call to tell warm-up manager about each hardware context
    void RegisterHWC(HW_CONTEXT hwc);

    // Register to be notified when warm-up phase begins and ends
    void RegisterForPhases(WARMUP_CALLBACK cbk);

    // Register to be notified each time a warm-up loop for one call to the
    // feeder completes.  For feeders that provide data by executing code this
    // can be an indicator of time passing.  For feeders that provide warm-up
    // from a table this is meaningless.
    void RegisterForTicks(WARMUP_CALLBACK cbk);

    // Register to be notified about data references.  Pass NULL for
    // hwc to be notified about all data or pass a hwc to be called
    // only for a specific context.
    void RegisterForData(WARMUP_CALLBACK cbk, HW_CONTEXT hwc);

    // Register to be notified about fetched instructions.  Set lineBytes to
    // eliminate sequential callbacks about the same cache line.
    // Pass NULL for hwc to be notified about all fetches or pass a hwc to
    // be called only for a specific context.
    void RegisterForIFetch(WARMUP_CALLBACK cbk, HW_CONTEXT hwc, UINT32 lineBytes);

    // Register to be notified about control transfer instructions.
    // Pass NULL to be notified for all hardware context.
    //
    // The handler is called only for control transfers.  The handler
    // can generally figure out the lines touched between branches.
    // NOTE: The feeder is not required to deal with interrupts or
    // exceptions cleanly during warm-up mode.  Control may suddenly
    // switch to a trap handler during warm-up without notice.  The
    // handler is expected to recover from unannounced changes in
    // the stream.  This should be an extremely small fraction of
    // the warm-up stream and, consequently, not a problem.
    void RegisterForInstrs(WARMUP_CALLBACK cbk, HW_CONTEXT hwc);

  private:
    //
    // Most of the callback lists are just lists of the basic WARMUP_CALLBACK.
    // IFETCH lists need to store the size of a cache line and the last line
    // noted for each callback.
    //
    class IFETCH_CALLBACK_CLASS
    {
      public:
        IFETCH_CALLBACK_CLASS(WARMUP_CALLBACK cbk, UINT32 lineBytes) :
            cbk(cbk),
            lastVA(0)
        {
            ASSERT((lineBytes & (lineBytes - 1)) == 0, "IFetch warm-up line size must be power of 2");
            ASSERTX(lineBytes != 0);

            lineMask = lineBytes - 1;
            lineMask = ~lineMask;
        };

        ~IFETCH_CALLBACK_CLASS() {};

        void CallIfNewLine(HW_CONTEXT hwc, WARMUP_IFETCH wFetch)
        {
            UINT64 nextVA = wFetch->GetVA() & lineMask;
            if (nextVA != lastVA)
            {
                lastVA = nextVA;
                cbk->WarmUpIFetch(hwc, wFetch);
            }
        };

      private:
        WARMUP_CALLBACK cbk;
        UINT64 lineMask;
        UINT64 lastVA;
    };
    typedef IFETCH_CALLBACK_CLASS *IFETCH_CALLBACK;
        

    typedef list<WARMUP_CALLBACK> PHASE_CALLBACK_LIST;
    typedef list<WARMUP_CALLBACK> TICK_CALLBACK_LIST;
    typedef list<WARMUP_CALLBACK> DATA_CALLBACK_LIST;
    typedef list<IFETCH_CALLBACK> IFETCH_CALLBACK_LIST;
    typedef list<WARMUP_CALLBACK> INSTR_CALLBACK_LIST;

    class WARMUP_HWC_CLASS
    {
      public:
        WARMUP_HWC_CLASS(HW_CONTEXT hwc);
        ~WARMUP_HWC_CLASS() {};

        HW_CONTEXT hwc;

        DATA_CALLBACK_LIST   dataCallbacks;
        IFETCH_CALLBACK_LIST ifetchCallbacks;
        INSTR_CALLBACK_LIST  instrCallbacks;

        UINT64 hwcUID;
        UINT64 nDataInits;
        UINT64 nIFetchInits;
        UINT64 nCtrlInits;
        UINT64 nEmptyInits;
    };

    typedef WARMUP_HWC_CLASS * WARMUP_HWC;
    typedef list<WARMUP_HWC> WARMUP_HWC_LIST;

    WARMUP_HWC_LIST hwcs;

    PHASE_CALLBACK_LIST  globalPhaseCallbacks;
    TICK_CALLBACK_LIST   globalTickCallbacks;
    DATA_CALLBACK_LIST   globalDataCallbacks;
    IFETCH_CALLBACK_LIST globalIFetchCallbacks;
    INSTR_CALLBACK_LIST  globalInstrCallbacks;

    UINT32 nDataCallbacks;
    UINT32 nIFetchCallbacks;
    UINT32 nInstrCallbacks;

    WARMUP_HWC FindHWC(HW_CONTEXT hwc)
    {
        for (WARMUP_HWC_LIST::iterator whwc = hwcs.begin();
             whwc != hwcs.end();
             whwc++)
        {
            if ((*whwc)->hwc == hwc)
            {
                return *whwc;
            }
        }

        ASIMERROR("Requested HWC not registered");
        return NULL;
    };

    void CallPhaseCallbacks(WARMUP_CALLBACK_CLASS::WARMUP_PHASE phase)
    {
        PHASE_CALLBACK_LIST::iterator cbk = globalPhaseCallbacks.begin();
        while (cbk != globalPhaseCallbacks.end())
        {
            (*cbk)->WarmUpPhase(phase);
            cbk++;
        }
    };
    
    void CallTickCallbacks(void)
    {
        TICK_CALLBACK_LIST::iterator cbk = globalTickCallbacks.begin();
        while (cbk != globalTickCallbacks.end())
        {
            (*cbk)->WarmUpTick();
            cbk++;
        }
    };
    
    void CallDataCallbacks(const DATA_CALLBACK_LIST cbkList,
                           HW_CONTEXT hwc,
                           WARMUP_DATA wData)
    {
        DATA_CALLBACK_LIST::const_iterator cbk = cbkList.begin();
        while (cbk != cbkList.end())
        {
            (*cbk)->WarmUpData(hwc, wData);
            cbk++;
        }
    };
    
    void CallIFetchCallbacks(const IFETCH_CALLBACK_LIST cbkList,
                             HW_CONTEXT hwc,
                             WARMUP_IFETCH wFetch)
    {
        IFETCH_CALLBACK_LIST::const_iterator cbk = cbkList.begin();
        while (cbk != cbkList.end())
        {
            (*cbk)->CallIfNewLine(hwc, wFetch);
            cbk++;
        }
    };
    
    void CallInstrCallbacks(const INSTR_CALLBACK_LIST cbkList,
                            HW_CONTEXT hwc,
                            WARMUP_INSTR wInstr)
    {
        INSTR_CALLBACK_LIST::const_iterator cbk = cbkList.begin();
        while (cbk != cbkList.end())
        {
            (*cbk)->WarmUpInstr(hwc, wInstr);
            cbk++;
        }
    };
};

#endif /* _WARMUP_INSTRS_ */
