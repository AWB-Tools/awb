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
  * @file TrackVec.cpp
  */

#include "asim/TrackVec.h"
#include "asim/TagDescVector.h"

/**
 * Creator of this class. Set defaults values.
 *
 * @return new object.
 */
TrackIDNode::TrackIDNode()
{
    minTgId = 2000000000;
    maxTgId = -1;
}

/**
 * Clears the content of the tracking vector.
 *
 * @return destroys the object.
 */
TrackIDNode::~TrackIDNode()
{
    tgIdVector.clear();
}

/**
 * Sets the type of tracking to move item. Tracks the edge edge_id.
 *
 * @return void.
 */
void
TrackIDNode::setType_MoveItem(UINT16 edge_id)
{
    type = TRACKIDTYPE_MOVEITEM;
    trackSpec.node_edge_id = edge_id;
}

/**
 * Sets the type of tracking to node tag. Tracks the slot slot of
 * the node node_id.
 *
 * @return void.
 */
void
TrackIDNode::setType_NodeTag(UINT16 node_id, NodeSlot slot)
{
    type = TRACKIDTYPE_NODETAG;
    trackSpec.node_edge_id = node_id;
    trackSpec.slot = slot;
}

/**
 * Sets the type of tracking to enter node. Tracks the slot slot of
 * the node node_id.
 *
 * @return void.
 */
void
TrackIDNode::setType_EnterNode(UINT16 node_id, NodeSlot slot)
{
    type = TRACKIDTYPE_ENTERNODE;
    trackSpec.node_edge_id = node_id;
    trackSpec.slot = slot;
}

/**
 * Sets the type of tracking to exit node. Tracks the slot slot of
 * the node node_id.
 *
 * @return void.
 */
void
TrackIDNode::setType_ExitNode(UINT16 node_id, NodeSlot slot)
{
    type = TRACKIDTYPE_EXITNODE;
    trackSpec.node_edge_id = node_id;
    trackSpec.slot = slot;
}

/**
 * Sets the type of tracking to cycle tag.
 *
 * @return void.
 */
void
TrackIDNode::setType_CycleTag()
{
    type = TRACKIDTYPE_CYCLETAG;
}

/**
 * Dumps the content of the vector.
 *
 * @return void.
 */
void
TrackIDNode::dumpTrackId()
{
    switch(type)
    {
        case TRACKIDTYPE_MOVEITEM:
        dumpTrackId_MoveItem();
        break;

        case TRACKIDTYPE_NODETAG:
        dumpTrackId_NodeTag();
        break;

        case TRACKIDTYPE_ENTERNODE:
        dumpTrackId_EnterNode();
        break;

        case TRACKIDTYPE_EXITNODE:
        dumpTrackId_ExitNode();
        break;

        case TRACKIDTYPE_CYCLETAG:
        dumpTrackId_CycleTag();
        break;
    }
}

/**
 * Dumps the content of the vector.
 *
 * @return void.
 */
void
TrackIDNode::dumpRegression()
{
    char str[256];

    int mintgid = tgIdVector.getMinUsedIdx();
    int maxtgid = tgIdVector.getMaxUsedIdx();
    TagDescVector* tgdescvector = TagDescVector::getInstance();
    for(UINT16 i = mintgid; i <= maxtgid; i++)
    {
        sprintf(str, FMT16X, i);
        printf("T=%s\n", str);
        if(i != TagDescVector::getInstance()->getItemIdxIn_TagId())
        {
            tgIdVector[i].dumpTagIdVector();
        }
    }
}

/**
 * Dumps the content of the track.
 *
 * @return void.
 */
void
TrackIDNode::dumpTrackId_MoveItem()
{
    printf ("\n*) dumping Move Item track on edge_id=%d\n",
           (int)(trackSpec.node_edge_id));
    tgIdVector[TagDescVector::getItemIdxIn_TagId()].dumpTagIdVector();
}

/**
 * Dumps the content of the track.
 *
 * @return void.
 */
void
TrackIDNode::dumpTrackId_NodeTag()
{
    printf ("\n*) dumping Node Tag track on node_id=%d slot ",(int)(trackSpec.node_edge_id));
    NodeSlot nslot = trackSpec.slot;
    for (int i=0;i<(nslot.specDimensions-1);i++)
    {
        printf ("%i,",nslot.dimVec[i]);
    }
    if (nslot.specDimensions>0)
    {
        printf ("%d.\n",(int)(nslot.dimVec[nslot.specDimensions - 1]));
    }
    else
    {
        printf ("root.\n");
    }

    int mintgid = tgIdVector.getMinUsedIdx();
    int maxtgid = tgIdVector.getMaxUsedIdx();
    TagDescVector* tgdescvector = TagDescVector::getInstance();
    for (int i=mintgid;i<=maxtgid;i++)
    {
        printf("Dumping info for TagId=%d => %s: ",i,tgdescvector->getTagDescription(i).latin1());
        tgIdVector[i].dumpTagIdVector();
    }
}

/**
 * Dumps the content of the track.
 *
 * @return void.
 */
void
TrackIDNode::dumpTrackId_EnterNode()
{
    printf ("\n*) dumping Enter Node track on node_id=%d slot ",(int)(trackSpec.node_edge_id));
    NodeSlot nslot = trackSpec.slot;
    for (int i=0;i<(nslot.specDimensions-1);i++) printf ("%i,",nslot.dimVec[i]);
    if (nslot.specDimensions>0)
    {
        printf ("%d.\n",(int)(nslot.dimVec[nslot.specDimensions - 1]));
    }
    else
    {
        printf ("root.\n");
    }
    tgIdVector[TagDescVector::getItemIdxIn_TagId()].dumpTagIdVector();
}

/**
 * Dumps the content of the track.
 *
 * @return void.
 */
void
TrackIDNode::dumpTrackId_ExitNode()
{
    printf ("\n*) dumping Exit Node track on node_id=%d slot ",(int)(trackSpec.node_edge_id));
    NodeSlot nslot = trackSpec.slot;
    for (int i=0;i<(nslot.specDimensions-1);i++)
    {
        printf ("%i,",nslot.dimVec[i]);
    }
    if (nslot.specDimensions>0)
    {
        printf ("%d.\n",(int)(nslot.dimVec[nslot.specDimensions - 1]));
    }
    else
    {
        printf ("root.\n");
    }
    tgIdVector[TagDescVector::getItemIdxIn_TagId()].dumpTagIdVector();
}

/**
 * Dumps the content of the track.
 *
 * @return void.
 */
void
TrackIDNode::dumpTrackId_CycleTag()
{
    printf ("\n*) dumping Cycle Tag track...\n");
    int mintgid = tgIdVector.getMinUsedIdx();
    int maxtgid = tgIdVector.getMaxUsedIdx();
    TagDescVector* tgdescvector = TagDescVector::getInstance();
    for (int i=mintgid;i<=maxtgid;i++)
    {
        printf("Dumping info for TagId=%d => %s: ",i,tgdescvector->getTagDescription(i).latin1());
        tgIdVector[i].dumpTagIdVector();
    }
}

/**
  * Compresses the vector to a dense vector.
  *
  * @return the compressed vector.
  */
ZipObject*
TrackIDNode::compressYourSelf(INT32 cycle, bool last)
{
    for (int i=minTgId;i<=maxTgId;i++)
    {
        if (tgIdVector.hasElement(i))
        {
           tgIdVector[i].compressYourSelf(cycle,last);
        }
    }
    return this;
}

/**
 * Returns a list with all the known tag ids.
 *
 * @return a list with the tag ids.
 */
TagIDList*
TrackIDNode::getKnownTagIDs()
{
    TagIDList* result = new TagIDList();
    for (int i=minTgId;i<=maxTgId;i++)
    {
        if (tgIdVector.hasElement(i) && tgIdVector[i].hasData())
        {
            result->append(i);
        }
    }
    return result;
}
