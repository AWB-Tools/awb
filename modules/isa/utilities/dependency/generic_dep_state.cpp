/*****************************************************************************
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
 * @brief Generic (non-stacked) state of dependency info
 *
 */

#include "asim/provides/generic_dependency_state.h"
#include "asim/provides/dependency_objects.h"

/**
 * constructor
 */
GENERIC_DEP_STATE_CLASS::GENERIC_DEP_STATE_CLASS(
    UINT32 numDep,     ///< number of dependencies in this class
    UINT32 numRotRegs) ///< number of rotating registers (0 if not specified)
    : numDependencies(numDep), 
    numRotatingRegs(numRotRegs),
    startOfRotation(numDep - numRotRegs)
{
    // initialize table of output dependencies
    producers = new OUTPUT_DEP[numDep];
    
    for (UINT32 k = 0; k < numDep; k++) 
    {
        // init all dependencies to NULL to start
        producers[k] = NULL;
    }        
}   

/**
 * destructor
 */
GENERIC_DEP_STATE_CLASS::~GENERIC_DEP_STATE_CLASS()
{
    // cleanup smart pointers
    Terminator();
    // and delete array
    delete [] producers;
}

/**
 * End-of-program termination of state held in the state.
 * Discovery of as many objects as we can reach and NULLing of all
 * associated MM pointers.
 */
void
GENERIC_DEP_STATE_CLASS::Terminator(void)
{
    for (UINT32 k = 0; k < numDependencies; k++) 
    {
        if (producers[k])
        {
            // clean links in prev direction
            OUTPUT_DEP prod = producers[k];
            ASSERTX (prod->GetNextProducer() == NULL);
            while (prod) 
            {
                OUTPUT_DEP temp = prod->GetPrevProducer();
                if (temp) 
                {
                    temp->SetNextProducer(NULL);
                    prod->SetPrevProducer(NULL);
                }

                if (prod->GetInst())
                {
                    prod->GetInst()->NullDstDep(prod);
                    prod->NullInst();
                }
                prod->RemoveDependencyLinks();

                prod = temp;
            }
        }

        producers[k] = NULL;
    }
}


/**
 * given a virtual register number and a rotating register base, return the
 * physical (ASIM's physical) register number
 */
UINT32
GENERIC_DEP_STATE_CLASS::GetRotatedRegNum(
    UINT32 virtualNum,            ///> virtual reg. num
    UINT32 rrb)               ///> register rotation base
{
    UINT32 rotatedNum;

    if (virtualNum >=numDependencies)
    {
       cout<<"virtualNum"<<virtualNum<<endl;
       cout<<"numDependencies"<<numDependencies<<endl;

    }
    ASSERTX(virtualNum < numDependencies);

    // if virtualNum is less than where the rotation starts, the physical
    // register shouldn't be rotated
    if (virtualNum < startOfRotation) {
        rotatedNum = virtualNum;
    }
    else
    {
        ASSERTX(numRotatingRegs > 0);

        // calc. the rotated register number
        rotatedNum = (virtualNum - startOfRotation + rrb) % numRotatingRegs + startOfRotation;
    }
    
    ASSERTX(rotatedNum < numDependencies);

    return(rotatedNum);
}

/**
 * Given a virtual register number and a register rotation base, this method
 * returns the OUTPUT_DEP that produces this register
 */
OUTPUT_DEP 
GENERIC_DEP_STATE_CLASS::GetProducer(
    UINT32 virtualNum,             ///> virtual register number
    UINT32 *physRegNum,            ///< physical register number
    UINT32 rrb)                ///> register rotation base
{ 
    UINT32 rotatedNum = GetRotatedRegNum(virtualNum, rrb);

    *physRegNum = rotatedNum;

    return producers[rotatedNum];
}
/**
 * Given a physical register number returns 
 *the OUTPUT_DEP that produces this register
 */
OUTPUT_DEP 
GENERIC_DEP_STATE_CLASS::GetProducer(UINT32 physRegNum)
{
    return producers[physRegNum];
}

/**
 * Given a virtual register number and a register rotation base and a producer,
 * this method changes the dependency state that so that this producer gets
 * recorded as the producer of this virtual register number
 */
void 
GENERIC_DEP_STATE_CLASS::SetProducer(
    OUTPUT_DEP prod,               ///> new producer
    UINT32 virtualNum,             ///> virtual register number
    UINT32 rrb)                    ///> register rotation base
{ 
    UINT32 rotatedNum = GetRotatedRegNum(virtualNum, rrb);

    prod->SetPhysRegNum(rotatedNum);

    producers[rotatedNum] = prod; 
}

/**
 * This is called when an instruction has retired and is cleaning up
 * it's dependency information.  It removes the producer from the
 * register stack.
 */
void
GENERIC_DEP_STATE_CLASS::RemoveOutputDependency(
    OUTPUT_DEP producer)
{
    // get the index into the register stack and remove it.
    UINT32 physRegNum = producer->GetPhysRegNum();
    
    // if the producer is associated wiht an instruction that has been killed
    // buy not mapped yet, than it is possible for the physical register number
    // to be UINT32_MAX.  If that's the case, there's nothing to clean up, so
    // return.
    if (physRegNum == UINT32_MAX)
    {
        return;
    }
    
    ASSERTX(physRegNum < numDependencies);

    if (producers[physRegNum] == producer)
    {
        producers[physRegNum] = NULL;
    }
}

/**
 * Restore the dependency state after an exception
 */
void
GENERIC_DEP_STATE_CLASS::RestoreDependencyState(void)
{
    UINT32 ctr = 0;
    while (ctr < numDependencies)
    {
        OUTPUT_DEP temp;
        OUTPUT_DEP prod = producers[ctr];

        // ... looking for any that point to a killed instruction
        if ((prod != NULL) && prod->GetInst() && prod->GetInst()->IsKilled())
        {
            TRACE(Trace_Qbox, prod->DumpTrace());

            // prod->nextWriter should be null, because it is the
            // the most recent producer to be inserted into the producer list
            ASSERTX(prod->GetNextProducer() == NULL);
            
            // if it points to a killed instruction, follow chain of
            // instructions that write this output register until the first
            // is found that ISN'T killed.  remove dependency pointers for
            // killed instructions while looking for first non-killed
            // instr.

            // if instruction is killed, remove dependency.  Also, if there's no
            // instruction for this dependency, the instruction was probably
            // killed and removed, so remove dependency
            while ((prod != NULL) && 
                   ((prod->GetInst() == NULL) || (prod->GetInst()->IsKilled())))
            {
                
                temp = prod->GetPrevProducer();
                if (temp) 
                {
                    temp->SetNextProducer(NULL);
                    prod->SetPrevProducer(NULL);
                }

                // These should really be in a better spot, since they're done
                // elsewhere in a similar manner (CleanUpProdDepLinks() in
                // dep_state.cpp).  However, for now, we'll put them here
                // because this must be done for proper ref counting
                // TO DO: Clean this up, because a base class is now dependent
                // on a particular implementation of CPU_INST - Eric
                if (prod->GetInst())
                {
                    prod->GetInst()->NullDstDep(prod);
                    prod->NullInst();
                }
                prod->RemoveDependencyLinks();

                prod = temp;
            }
            
            // once found first non-killed instruction that writes output
            // register, set the dep state to indicate it's the instruction that
            // produces this register value (to restore machine state
            // before wrong path instructions started executing).
            // will be assigned NULL if there was no prevUser
            producers[ctr] = prod;
        }
        ctr++;
    }

}

/**
 * Dump state of entire dep table
 */
void
GENERIC_DEP_STATE_CLASS::DumpState()
{
    UINT32 ctr = 0;
    while (ctr < numDependencies)
    {
        OUTPUT_DEP prod = producers[ctr];
        
        if (prod)
        {
//            cout << "\n******** producers[" << ctr << "]\n";
        }
        while (prod)
        {
            prod->DumpTrace();
            if (prod->GetInst())
            {
//                cout << "\t\t" << prod->GetInst() << endl;
            }
            else
            {
//                cout << "\t\tNo inst\n";
            }

            prod = prod->GetPrevProducer();
        }
        ctr++;
    }
}
