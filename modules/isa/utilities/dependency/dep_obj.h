/*****************************************************************************
 * dependency.h - tracks dependencies between objects (usually instructions)
 *
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
 * @author Eric Borch
 * @brief Generic dependency tracking between instructions.
 */

#ifndef _DEPENDENCY_
#define _DEPENDENCY_

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/mm.h"
#include "asim/trace.h"
#include "asim/atomic.h"

//typedef class mmptr<class TINYOP_CLASS> CPU_INST;
typedef class mmptr<class OUTPUT_DEP_CLASS> OUTPUT_DEP;
typedef class mmptr<class INPUT_DEP_CLASS> INPUT_DEP;

// ASIM public modules -- BAD! in asim-core
#include "asim/provides/cpu_inst.h"
#include "asim/arch_register.h"

//typedef class mmptr<class TINYOP_CLASS> CPU_INST;

// each instruction can have multiple dependencies, and sometimes dependencies
// can live beyond when their instructions die, so let's say we need 4 times as
// many dependency objects as instructions.
#define MAX_INPUT_DEP MAX_CPU_INST * 4
#define MAX_OUTPUT_DEP MAX_CPU_INST * 4

const UINT64 NO_PRODUCER = UINT64_MAX;

class DEPENDENCY_CLASS
{
  private:
    // pointer back to the inst this dependency belongs to.
    CPU_INST inst;

    ARCH_REGISTER_TYPE type;
    UINT64 uid;
    UINT32 virtRegNum;
    UINT32 physRegNum;
    UINT32 opnd;
    UINT32 fusionSlot;
    UINT32 partialRegNum;
    UINT32 regMask;

    bool rename;

  public:
    DEPENDENCY_CLASS(CPU_INST i, ARCH_REGISTER_TYPE t, UINT64 u, UINT32 n, UINT32 o = 0, UINT32 p = UINT32_MAX);
    ~DEPENDENCY_CLASS();
	
    // Accessors
    CPU_INST GetInst();
    ARCH_REGISTER_TYPE GetDepType();
    UINT32 GetVirtRegNum();
    UINT32 GetPhysRegNum();
    UINT32 GetOpnd();
    UINT64 GetUid();

    //support for fusion need to know which slot this come from
    UINT32 GetFusionSlot() { return fusionSlot; }
    void SetFusionSlot(UINT32 arg) { fusionSlot=arg; }

    //support for x86 partial and masked writes
    UINT32 GetPartialRegNum();
    UINT32 GetRegMask();
    void SetRegMask(UINT32 arg);
    void SetPartialRegNum(UINT32 arg);

    bool GetRename();

    // Modifiers
    void NullInst();
    void SetPhysRegNum(UINT32 n);
};


class INPUT_DEP_CLASS :      
    public ASIM_MM_CLASS<INPUT_DEP_CLASS>,
    public DEPENDENCY_CLASS
{
  private:
    static UID_GEN64 uniqueId;

    // If we have a physical register number, I'm not sure if this is needed.
    // Eric
    UINT64 producerUid;

    // pointer to next consumer that uses this input
    INPUT_DEP nextConsumer;
    INPUT_DEP prevConsumer;

    
    // pointer to inst that produced this input
    OUTPUT_DEP producer;
    
    // cycle this input dependency can first issue
    UINT64 cycleReadyForIssue;
    
  public:
    INPUT_DEP_CLASS(CPU_INST i, ARCH_REGISTER_TYPE t, UINT32 n, UINT32 o = 0);

    ~INPUT_DEP_CLASS();
    
    // Accessors
    INPUT_DEP GetNextConsumer();
    INPUT_DEP GetPrevConsumer();

    OUTPUT_DEP GetProducer();
    UINT64 GetCycleReadyForIssue();
    UINT64 GetProducerUid();
    
    // Modifiers
    void SetNextConsumer(INPUT_DEP nc);
    void SetPrevConsumer(INPUT_DEP pc);

    void SetProducer(OUTPUT_DEP p);
    void SetCycleReadyForIssue(UINT64 cr);
    
    // this builds the RAW dependency info
    void BuildRAWDependencyGraph(OUTPUT_DEP output, UINT64 cycle);

    // remove dependency links (after it's killed or committed)
    void RemoveDependencyLinks();

    //this method works like the RemoveDependencyLinks with two
    //differences a) it uses the prev pointer instead of walking
    //the chain of consumers and b)it only removes this input
    //dependency
    void RemoveSingleDependencyLink();     


    // clean up links (dep and inst)
    void CleanUpLinks();

    // dump state
    void DumpTrace();
    string TraceToString();
};


class OUTPUT_DEP_CLASS : 
    public ASIM_MM_CLASS<OUTPUT_DEP_CLASS>,
    public DEPENDENCY_CLASS
{
  private:
    static UID_GEN64 uniqueId;

    // points to the youngest consumer of this dependency
    INPUT_DEP consumer;
    
    // Cycle this dependency object is actually produced
    UINT64 cycleValueProduced;

    // cycle this dependency can first issue, used for
    // WAW and WAR hazards
    UINT64 cycleReadyForIssue;
    
    // cycle consumers can issue
    UINT64 cycleDependentsCanIssue;
    
    // cycle this dependency object issued (for controlling when dependents issue)
    UINT64 cycleIssued;

    // execution latency for THIS operand
    UINT32 executionLatency;

    // value produced, not used now, maybe for prediction?
    UINT64 value;

    // poisoned
    bool poisoned;

    // these point to the previous and next instruction that 
    // produce (write) this particular output.  primarily for exception
    // recovery.  Kind of a hack, in that the dependency code SHOULDN'T have
    // members to deal with how a particular implementation deals with
    // exceptions, but this makes it much easier.  Can also be used
    // for WAW and WAR hazards.  TO DO: Should this be changed?
    OUTPUT_DEP prevProducer;
    OUTPUT_DEP nextProducer;

  public:
    OUTPUT_DEP_CLASS(CPU_INST i, ARCH_REGISTER_TYPE t, UINT32 n, UINT32 o = 0, UINT32 p = UINT32_MAX);
    
    ~OUTPUT_DEP_CLASS(); 
    
    // Accessors
    INPUT_DEP GetConsumer();
    UINT64 GetCycleValueProduced() const;
    UINT64 GetCycleIssued() const;
    UINT64 GetCycleDependentsCanIssue() const;
    OUTPUT_DEP GetNextProducer();
    OUTPUT_DEP GetPrevProducer();
    UINT64 GetCycleReadyForIssue() const;
    UINT32 GetExecutionLatency() const;
    bool GetPoisoned() const;

    // Modifiers
    void SetConsumer(INPUT_DEP c);
    void SetCycleValueProduced(const UINT64 cr);
    void SetCycleIssued(const UINT64 ci);
    void SetCycleDependentsCanIssue(const UINT64 cdci);
    void SetNextProducer(OUTPUT_DEP n);
    void SetPrevProducer(OUTPUT_DEP p);
    void SetCycleReadyForIssue(const UINT64 cr);
    void SetExecutionLatency(const UINT32 lat);
    void SetPoisoned(const bool p);

    // this connects the output dependency objects for clean up after
    // exceptions, or for WAW hazards, if you want them.
    void BuildOutputDependencyGraph(OUTPUT_DEP output);
    
    // this ignores WAW hazards, and sets the output dependency ready for issue immediately
    void IgnoreWAWDependencies(UINT64 cycle);

    // this observes WAW hazards, and sets the output dependency ready for issue
    // based upon the prev producers state.
    void ObserveWAWDependencies(OUTPUT_DEP output, UINT64 cycle);

    // signal dependents when they can issue

    void SignalNextProducer(UINT64 cycle);
    void SignalProducerToStopIssue();
    void UpdateNextProducerIssueTime();
    void SignalConsumers(UINT64 cycle);
    void UpdateConsumersIssueTime();
    void SignalConsumersToStopIssue();


    // remove dependency links (after it's killed or committed)
    void RemoveDependencyLinks();

    // clean up all links (dep and inst)
    void CleanUpLinks();

    // dump state
    void DumpTrace();
    string TraceToString();
};



inline 
DEPENDENCY_CLASS::DEPENDENCY_CLASS(
    CPU_INST i, 
    ARCH_REGISTER_TYPE t, 
    UINT64 u, 
    UINT32 n, 
    UINT32 o,
    UINT32 p)
    : inst(i), 
      type(t), 
      uid(u), 
      virtRegNum(n), 
      physRegNum(p),
      opnd(o),
      rename(true)
{
    ASSERT(i, "Created a dependency type that has no inst!\n");
//    ASSERT(type != DEP_INVALID, "Created a dependency object with INVALID
//    type\n");
    if (p != UINT32_MAX)
    {
        rename = false;
    }
    partialRegNum=n;
    regMask=0;
}

    
inline 
DEPENDENCY_CLASS::~DEPENDENCY_CLASS() 
{
    // inst is a "const", so this won't work.  But, inst MUST be nulled
    // out for reference counting purposes.  Will the destructor cause inst
    // to go out of scope, thereby nulling it out? Eric
    inst = NULL;
}

//support for x86 partial and masked writes
inline UINT32 
DEPENDENCY_CLASS::GetPartialRegNum()
{
    return partialRegNum;
}
inline UINT32 
DEPENDENCY_CLASS::GetRegMask()
{
    return regMask;
}
inline void 
DEPENDENCY_CLASS::SetRegMask(
    UINT32 arg)
{
    regMask=arg;
}
inline void 
DEPENDENCY_CLASS::SetPartialRegNum(
    UINT32 arg)
{
    partialRegNum=arg;
}


inline CPU_INST 
DEPENDENCY_CLASS::GetInst() 
{ 
    return inst; 
}

inline ARCH_REGISTER_TYPE
DEPENDENCY_CLASS::GetDepType() 
{
    return type; 
}

inline UINT32 
DEPENDENCY_CLASS::GetVirtRegNum() 
{ 
    return virtRegNum; 
}

inline UINT32 
DEPENDENCY_CLASS::GetPhysRegNum() 
{ 
//    ASSERTX(physRegNum != UINT32_MAX);
    return physRegNum; 
}

inline UINT32 
DEPENDENCY_CLASS::GetOpnd() 
{ 
    return opnd; 
}

inline UINT64
DEPENDENCY_CLASS::GetUid()
{ 
    return uid; 
}

inline bool
DEPENDENCY_CLASS::GetRename()
{ 
    return rename;
}

// need to set "inst" to null once we're done with it so the reference
// counting works.
inline void 
DEPENDENCY_CLASS::NullInst()
{ 
    inst = NULL; 
}



inline 
INPUT_DEP_CLASS::INPUT_DEP_CLASS(
    CPU_INST i, 
    ARCH_REGISTER_TYPE t, 
    UINT32 n, 
    UINT32 o) :
    ASIM_MM_CLASS<INPUT_DEP_CLASS>(uniqueId++, 0),  
    DEPENDENCY_CLASS(i, t, ASIM_MM_CLASS<INPUT_DEP_CLASS>::GetMMUid(), n, o),     
    producerUid(NO_PRODUCER),
    nextConsumer(NULL), 
    producer(NULL), 
    cycleReadyForIssue(UINT64_MAX)
{
}

inline 
INPUT_DEP_CLASS::~INPUT_DEP_CLASS() 
{
    ASSERTX(nextConsumer == NULL);
    ASSERTX(producer == NULL);
}


inline INPUT_DEP 
INPUT_DEP_CLASS::GetNextConsumer() 
{ 
    return nextConsumer; 
}
inline INPUT_DEP 
INPUT_DEP_CLASS::GetPrevConsumer() 
{ 
    return prevConsumer; 
}


inline OUTPUT_DEP 
INPUT_DEP_CLASS::GetProducer() 
{ 
    return producer; 
}

inline UINT64
INPUT_DEP_CLASS::GetProducerUid() 
{ 
    return producerUid;
}

inline UINT64 
INPUT_DEP_CLASS::GetCycleReadyForIssue() 
{ 
    return cycleReadyForIssue; 
}
    
inline void
DEPENDENCY_CLASS::SetPhysRegNum(
    UINT32 n) 
{ 
//    ASSERTX(physRegNum == UINT32_MAX);
    physRegNum = n;
}

inline void 
INPUT_DEP_CLASS::SetNextConsumer(
    INPUT_DEP nc) 
{ 
    nextConsumer = nc; 
}

inline void 
INPUT_DEP_CLASS::SetPrevConsumer(
    INPUT_DEP pc) 
{ 
    prevConsumer = pc; 
}

inline void 
INPUT_DEP_CLASS::SetProducer(
    OUTPUT_DEP p) 
{ 
    producer = p;
    if (p)
    {
        producerUid = p->GetUid();
    }
    else
    {
        producerUid = NO_PRODUCER;
    }
}

inline void 
INPUT_DEP_CLASS::SetCycleReadyForIssue(
    const UINT64 cr)
{ 
    cycleReadyForIssue = cr; 
}

inline void 
INPUT_DEP_CLASS::DumpTrace()
{
    cout << TraceToString();
}

inline string
INPUT_DEP_CLASS::TraceToString()
{
    std::ostringstream buf;
    buf << "\tINPUT_DEP: DepType = " << GetDepType() << " VirtRegNumber = " << GetVirtRegNum();
    buf << " DepOpnd = " << GetOpnd() << " iid = " << GetUid();
    return(buf.str());
}

// output operator
inline std::ostream & 
operator << (std::ostream & os, INPUT_DEP dep)
{
    os << dep->TraceToString();
    return os;
}
    
inline OUTPUT_DEP_CLASS::OUTPUT_DEP_CLASS(
    CPU_INST i, 
    ARCH_REGISTER_TYPE t, 
    UINT32 n, 
    UINT32 o, 
    UINT32 p) 
    : ASIM_MM_CLASS<OUTPUT_DEP_CLASS>(uniqueId++, 0), 
    DEPENDENCY_CLASS(i, t, ASIM_MM_CLASS<OUTPUT_DEP_CLASS>::GetMMUid(), n, o, p),
    consumer(NULL),
    cycleValueProduced(UINT64_MAX),
    cycleReadyForIssue(UINT64_MAX),
    cycleDependentsCanIssue(UINT64_MAX),
    cycleIssued(UINT64_MAX),
    executionLatency(UINT32_MAX),
    poisoned(false),
    prevProducer(NULL),
    nextProducer(NULL)
{ }

inline 
OUTPUT_DEP_CLASS::~OUTPUT_DEP_CLASS() 
{
    ASSERTX(prevProducer == NULL);
    ASSERTX(nextProducer == NULL);
    ASSERTX(consumer == NULL);
}

inline INPUT_DEP 
OUTPUT_DEP_CLASS::GetConsumer() 
{ 
    return consumer; 
}

inline UINT64 
OUTPUT_DEP_CLASS::GetCycleValueProduced() const 
{ 
    return cycleValueProduced;
}

inline UINT64 
OUTPUT_DEP_CLASS::GetCycleIssued() const
{ 
    return cycleIssued;
}

inline UINT64 
OUTPUT_DEP_CLASS::GetCycleDependentsCanIssue() const 
{ 
    return cycleDependentsCanIssue; 
}

inline OUTPUT_DEP 
OUTPUT_DEP_CLASS::GetNextProducer()
{ 
    return nextProducer; 
}

inline OUTPUT_DEP 
OUTPUT_DEP_CLASS::GetPrevProducer()
{ 
    return prevProducer; 
}

inline UINT64 
OUTPUT_DEP_CLASS::GetCycleReadyForIssue() const
{ 
    return cycleReadyForIssue; 
}
    
inline UINT32
OUTPUT_DEP_CLASS::GetExecutionLatency() const
{
    // before returning it, let's make sure it was set
    ASSERTX(executionLatency != UINT32_MAX);
    return executionLatency;
}

inline bool
OUTPUT_DEP_CLASS::GetPoisoned() const
{
    return poisoned;
}

inline void 
OUTPUT_DEP_CLASS::SetConsumer(
    INPUT_DEP c) 
{ 
    consumer = c; 
};

inline void 
OUTPUT_DEP_CLASS::SetCycleValueProduced(
    const UINT64 cr) 
{
    // I'd like this assertion to be here, but the way the model works right
    // now, it just doesn't work.  
//    ASSERTX(cycleValueProduced == UINT64_MAX);
    cycleValueProduced = cr; 
}

inline void 
OUTPUT_DEP_CLASS::SetCycleIssued(
    const UINT64 ci) 
{ 
    cycleIssued = ci; 
}

inline void 
OUTPUT_DEP_CLASS::SetCycleDependentsCanIssue(
    const UINT64 cdci) 
{ 
    cycleDependentsCanIssue = cdci; 
}

inline void 
OUTPUT_DEP_CLASS::SetNextProducer(
    OUTPUT_DEP n) 
{ 
    nextProducer = n; 
}

inline void 
OUTPUT_DEP_CLASS::SetPrevProducer(
    OUTPUT_DEP p) 
{ 
    prevProducer = p; 
}

inline void 
OUTPUT_DEP_CLASS::SetCycleReadyForIssue(
    const UINT64 cr) 
{ 
    cycleReadyForIssue = cr; 
}

inline void
OUTPUT_DEP_CLASS::SetExecutionLatency(
    const UINT32 lat)
{
    executionLatency = lat;
}

inline void
OUTPUT_DEP_CLASS::SetPoisoned(
    const bool p)
{
    poisoned = p;
}

inline void 
OUTPUT_DEP_CLASS::DumpTrace()
{
    cout << TraceToString() << endl;
}

inline string 
OUTPUT_DEP_CLASS::TraceToString()
{
    std::ostringstream buf;
    buf << "\tOUTPUT_DEP: DepType = " << GetDepType() << " VirtRegNumber = " << GetVirtRegNum();
    buf << " DepOpnd = " << GetOpnd() << " oid = " << GetUid();
    return(buf.str());
}

// output operator
inline std::ostream & 
operator << (std::ostream & os, OUTPUT_DEP dep)
{
    os << dep->TraceToString();
    return os;
}

#endif /* _DEPENDENCY_ */


// should cycles issued & cycle committed be in the dependency code, or should
// we be checking the inst, since that's really where the data is?
