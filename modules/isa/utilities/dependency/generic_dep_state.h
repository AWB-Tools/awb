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

#ifndef _GENERIC_DEP_STATE_
#define _GENERIC_DEP_STATE_

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/mm.h"

typedef class mmptr<class OUTPUT_DEP_CLASS> OUTPUT_DEP;

/**
 * @brief Generic (non-stacked) state of dependency info.
 *
 * This class contains dependency state for non-stacked dependencies.  It also
 * supports register rotation, but this can easily be disabled by setting the
 * rotation size to zero upon construction.  Thus, it should be genereic enough
 * to work for any architecture.
 */
class GENERIC_DEP_STATE_CLASS
{
  private:
    const UINT32 numDependencies;     ///< number of static dependencies
    const UINT32 numRotatingRegs;     ///< number of rotating registers
    const UINT32 startOfRotation;     ///< start of the rotating region

    OUTPUT_DEP* producers;            ///< table of producers

        
    UINT32 GetRotatedRegNum(UINT32 virtualNum, UINT32 rrb = 0);

  public:
    // constructor/destructor
    GENERIC_DEP_STATE_CLASS(UINT32 numDep, UINT32 numRotRegs = 0);
    ~GENERIC_DEP_STATE_CLASS();

    // given a virtual register number, get the producer of that value
    OUTPUT_DEP GetProducer(UINT32 virtualNum, UINT32 *physRegNum, UINT32 rrb = 0);
    OUTPUT_DEP GetProducer(UINT32 physRegNum);

    // end-of-program cleanup
    void Terminator(void);

    // given a virtual register number, set the producer of that value
    void SetProducer(OUTPUT_DEP prod, UINT32 virtualNum, UINT32 rrb = 0);

    // given a producer, remove it from the list of producers
    void RemoveOutputDependency(OUTPUT_DEP producer);

    // on exception, restore dependency state
    void RestoreDependencyState(void);

    // dump entire state of dep table
    void DumpState();
};


#endif /* _GENERIC_DEP_STATE_ */
