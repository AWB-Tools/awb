// ==================================================
//Copyright (C) 2003-2006 Intel Corporation
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file TagIdVec.cpp
  */

#include "asim/TagIdVec.h"

/**
 * The static variables are set to NULL.
 */
StrTable* TagIdVecNode::strtbl = NULL;

/**
 * Gets the tag value in the cycle cycle.
 *
 * @return bool if a value is found.
 */
bool
TagIdVecNode::getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle)
{
    //printf (">> TagIdVecNode::getTagValue(cycle=%d), firstCycle=%d, lastCycle=%d\n",cycle,firstCycle,lastCycle);
    if (cycleVec==NULL)
    {
        return false;
    }
    if (cycle<firstCycle)
    {
        return false;
    }
    if (cycle>lastCycle)
    {
        if (!isFwd)
        {
            return false;
        }
        else
        {
            cycle = lastCycle;
            //printf (">> TagIdVecNode::getTagValue: lastCycle patched!\n");
        }
    }
    INT32 cycleChunk = cycle >> CYCLE_OFFSET_BITS;
    //printf (">> TagIdVecNode::getTagValue: patched cycle=%d, chunk=%d\n",cycle,cycleChunk);
    bool hit = (*cycleVec)[cycleChunk]->getTagValue(cycle,value,atcycle);
    if (hit)
    {
        //printf (">> TagIdVecNode::getTagValue: first try hit!\n");
        return true;
    }
    else
    {
        if (!isFwd)
        {
            return false;
        }
    }

    //printf (">> TagIdVecNode::getTagValue: looking for the value in previous chunks...\n");
    // if the value was not fnd in the "natural" chunk look back
    --cycleChunk;
    INT32 minChunk = firstCycle >> CYCLE_OFFSET_BITS;
    while (!hit && (cycleChunk>=minChunk))
    {
        hit = (*cycleVec)[cycleChunk--]->getTagValue(cycle,value,atcycle);
    }

    /*
    if (hit)
       printf (">> TagIdVecNode::getTagValue: match on chunk=%d\n",cycleChunk+1);
    else
       printf (">> TagIdVecNode::getTagValue: value not fnd.\n");
    */

    return hit;
}

/**
 * Gets the tag value in the cycle cycle.
 *
 * @return bool if a value is found.
 */
bool
TagIdVecNode::getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle)
{
    /// @todo implement this
    return false;
}

/**
 * Dumps the content of the vector.
 *
 * @return void.
 */
void
TagIdVecNode::dumpTagIdVector()
{
    if (cycleVec==NULL)
    {
        printf ("No available data\n");
        return;
    }

    INT32 firstChunk = firstCycle >> CYCLE_OFFSET_BITS;
    INT32 lastChunk = lastCycle >> CYCLE_OFFSET_BITS;

    for (int i=firstChunk;i<=lastChunk;i++)
    {
        if (cycleVec->hasElement(i) && ((*cycleVec)[i]!=NULL))
        {
           (*cycleVec)[i]->dumpCycleVector();
        }
    }
}

/**
 * Sets the write enable of the vector.
 *
 * @return void.
 */
void
TagIdVecNode::setWriteEnabled(bool value)
{
    if (cycleVec==NULL)
    {
        return;
    }

    INT32 firstChunk = firstCycle >> CYCLE_OFFSET_BITS;
    INT32 lastChunk = lastCycle >> CYCLE_OFFSET_BITS;

    for (int i=firstChunk;i<=lastChunk;i++)
    {
        if (cycleVec->hasElement(i) && ((*cycleVec)[i]!=NULL))
        {
           (*cycleVec)[i]->setWriteEnabled(value);
        }
    }
}

/**
  * Compresses the vector.
  *
  * @return the compressed vector.
  */
ZipObject*
TagIdVecNode::compressYourSelf(INT32 cycle, bool last)
{
    if (cycleVec==NULL)
    {
        return this;
    }

    INT32 firstChunk = firstCycle >> CYCLE_OFFSET_BITS;
    INT32 lastChunk = cycle >> CYCLE_OFFSET_BITS;

	// check incoherence conditions (typically produced by corrupted drl files)
	if (firstChunk>=TAGIDVECNODE_MAXCHUNKS) { firstChunk = TAGIDVECNODE_MAXCHUNKS-1 ; } 
	if (lastChunk>=TAGIDVECNODE_MAXCHUNKS) { lastChunk = TAGIDVECNODE_MAXCHUNKS-1 ; } 
	
	//printf ("TagIdVecNode::compressYourSelf cycle=%d,firstChunk=%d,lastChunk=%d\n",cycle,firstChunk,lastChunk);fflush(stdout);
    for (int i=firstChunk;i<=lastChunk;i++)
    {
        //printf ("TagIdVecNode::compressYourSelf checking chunk=%d, bcycle=%d\n",i,i*CYCLE_CHUNK_SIZE);
		//printf ("cycleVec->hasElement(%d) says %d\n",i,(int)cycleVec->hasElement(i));fflush(stdout);
		//if (cycleVec->hasElement(i)) { printf ("(*cycleVec)[%d] = %x\n",i,(void*)((*cycleVec)[i]));fflush(stdout); }
        if (cycleVec->hasElement(i) && ((*cycleVec)[i]!=NULL) )
        {
            //printf ("TagIdVecNode::compressYourSelf compressing chunk=%d, bcycle=%d\n",i,i*CYCLE_CHUNK_SIZE);fflush(stdout);
            TagVec* newvec = (TagVec*) ((*cycleVec)[i]->compressYourSelf(cycle,last));
            if ((*cycleVec)[i] != newvec )
            {
                //printf ("compression effective! new dense vector allocated\n");
                delete (*cycleVec)[i];
                (*cycleVec)[i] = newvec;
            }
        }
    }
    return this;
}
