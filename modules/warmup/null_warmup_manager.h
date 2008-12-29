/*****************************************************************************
 *
 * @brief Warm-up manager - NULL implementation
 *
 * @author Carl Beckmann (based on Michael Adler's instruction-based warmup manager)
 * 
 * Copyright (c) 2008 Intel Corporation, all rights reserved.
 * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
 * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
 *
 *****************************************************************************/

#ifndef _WARMUP_MANAGER_
#define _WARMUP_MANAGER_

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
        bool monitorDCache,
        bool monitorICache,
        bool monitorInstrs)
    {};

    bool MonitorDCache(void) const { return false; };
    bool MonitorICache(void) const { return false; };
    bool MonitorInstrs(void) const { return false; };

    bool operator== (const WARMUP_CLIENTS_CLASS& cmp) const { return false; }
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
    {};

    UINT64 GetVA(void)    const { return 0; };
    UINT64 GetPA(void)    const { return 0; };
    UINT32 GetBytes(void) const { return 0; };

    //
    // A feeder may return an ASIM_MACRO_INST even for a data reference.  For
    // feeders that supply cache line values the ASIM_MACRO_INST is needed to
    // call the feeder's GetCacheLineValueFollowingInst().
    //
    void SetAsimInst(ASIM_MACRO_INST ainst) {};
    bool IsAsimInstValid(void)        const { return false; };
    ASIM_MACRO_INST GetAsimInst(void) const { return NULL;  };
    
    //
    // A feeder may supply the address of the instruction generating the
    // data reference.  It can be passed to clients using these methods:
    //
    void SetInstrAddr(UINT64 iVA, UINT64 iPA) {}
    bool IsInstrAddrValid(void) const { return false; }
    UINT64 GetInstrVA(void)     const { return 0;     }
    UINT64 GetInstrPA(void)     const { return 0;     }

    bool IsLoad()                     { return false; }
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
    {};

    UINT64 GetVA(void) const { return 0; };
    UINT64 GetPA(void) const { return 0; };
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
    {};

    ASIM_MACRO_INST GetAsimInst(void) const { return NULL; };
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
    {};

    bool IsIFetch(void) const        { return false; }
    bool IsDataRef(void) const       { return false; }
    bool IsLoad(void) const          { return false; }
    bool IsStore(void) const         { return false; }
    bool IsCtrlTransfer(void) const  { return false; }

    UINT64 GetIFetchVA(void) const       { return 0; }
    UINT64 GetIFetchPA(void) const       { return 0; }

    UINT32 NLoads(void) const            { return 0; }
    UINT64 GetLoadVA(UINT32 n) const     { return 0; }
    UINT64 GetLoadPA(UINT32 n) const     { return 0; }
    UINT32 GetLoadBytes(UINT32 n) const  { return 0; }

    UINT32 NStores(void) const           { return 0; }
    UINT64 GetStoreVA(UINT32 n) const    { return 0; }
    UINT64 GetStorePA(UINT32 n) const    { return 0; }
    UINT32 GetStoreBytes(UINT32 n) const { return 0; }

    //
    // Call to indicate instruction fetch
    //
    void NoteIFetch(
        UINT64 va,
        UINT64 pa)
    {}

    //
    // Call to indicate warm-up is a data reference
    //
    void NoteLoad(
        UINT64 va,
        UINT64 pa,
        UINT32 bytes)
    {}

    void NoteStore(
        UINT64 va,
        UINT64 pa,
        UINT32 bytes)
    {}

    //
    // Call to indicate warm-up is a control transfer instruction.
    // Source and target addresses are stored in the ASIM_MACRO_INST.
    //
    void NoteCtrlTransfer(void) {}

    //
    // SW Context is required by InitAsimInst to allocate an ASIM_MACRO_INST.
    // The SW context should call this as the warm-up request passes through.
    //
    void NoteSWContext(SW_CONTEXT swc)      {}
    SW_CONTEXT GetSWC() const               { return 0;     }

    //
    // Allocate an ASIM_INST.  Should be called by a feeder.
    //
    ASIM_MACRO_INST InitAsimInst(void)      { return NULL;  }

    bool IsAsimInstValid(void) const        { return false; }
    ASIM_MACRO_INST GetAsimInst(void) const { return NULL;  }
};


class WARMUP_MANAGER_CLASS : public ASIM_MODULE_CLASS
{
  public:
    WARMUP_MANAGER_CLASS(ASIM_MODULE parent, const char *name)
      : ASIM_MODULE_CLASS(parent, name)
    {}

    // Called by a system
    void DoWarmUp(void) {}

    // Call to tell warm-up manager about each hardware context
    void RegisterHWC(HW_CONTEXT hwc) {}

    // Register to be notified when warm-up phase begins and ends
    void RegisterForPhases(WARMUP_CALLBACK cbk) {}

    // Register to be notified each time a warm-up loop for one call to the
    // feeder completes.  For feeders that provide data by executing code this
    // can be an indicator of time passing.  For feeders that provide warm-up
    // from a table this is meaningless.
    void RegisterForTicks(WARMUP_CALLBACK cbk) {}

    // Register to be notified about data references.  Pass NULL for
    // hwc to be notified about all data or pass a hwc to be called
    // only for a specific context.
    void RegisterForData(WARMUP_CALLBACK cbk, HW_CONTEXT hwc) {}

    // Register to be notified about fetched instructions.  Set lineBytes to
    // eliminate sequential callbacks about the same cache line.
    // Pass NULL for hwc to be notified about all fetches or pass a hwc to
    // be called only for a specific context.
    void RegisterForIFetch(WARMUP_CALLBACK cbk, HW_CONTEXT hwc, UINT32 lineBytes) {}

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
    void RegisterForInstrs(WARMUP_CALLBACK cbk, HW_CONTEXT hwc) {}
};

#endif /* _WARMUP_MANAGER_ */
