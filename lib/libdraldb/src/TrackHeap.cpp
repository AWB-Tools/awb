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
  * @file  TrackHeap.cpp
  * @brief
  */

#include "asim/TrackHeap.h"

/**
 * The instance is NULL at the beginning.
 */
TrackHeap* TrackHeap::_myInstance=NULL;

/**
 * The static variables are set to NULL.
 */
DBGraph*   TrackHeap::dbgraph=NULL;

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer.
 *
 * @return the instance of the class.
 */
TrackHeap*
TrackHeap::getInstance()
{
    if (_myInstance==NULL)
    {
        _myInstance = new TrackHeap();
    }

    Q_ASSERT(_myInstance!=NULL);
    return _myInstance;
}

/**
 * Destroys the unique instance of the class. The instance is
 * destroyed if it was previously created.
 *
 * @return void.
 */
void
TrackHeap::destroy()
{
    if (_myInstance!=NULL)
    {
        delete _myInstance;
    }
}

/**
 * Creator of this class. Creates all the hashes needed and get
 * the static instances. Finally calls reset.
 *
 * @return new object.
 */
TrackHeap::TrackHeap()
{
    nextTrackID = 0;
    cycleTagTrackID = -1;
    // Creates the the tracking dictionaries.
    trackedEnterNodes = new QDict<INT32>(997);
    trackedExitNodes  = new QDict<INT32>(997);
    trackedNodes = new QDict<INT32>(997);
    Q_ASSERT(trackedEnterNodes!=NULL);
    Q_ASSERT(trackedExitNodes!=NULL);
    Q_ASSERT(trackedNodes!=NULL);
    // Sets the autodelete to avoid memory leaks.
    trackedEnterNodes->setAutoDelete(true);
    trackedExitNodes->setAutoDelete(true);
    trackedNodes->setAutoDelete(true);
    // Gets the instances.
    dbgraph = DBGraph::getInstance();
    tagdescvec= TagDescVector::getInstance();
    tgItemIdxID = TagDescVector::getItemIdxIn_TagId();
    reset();
}

/**
 * Destructor of this class. Deletes the hashes.
 *
 * @return destroys the object.
 */
TrackHeap::~TrackHeap()
{
    delete trackedEnterNodes;
    delete trackedExitNodes;
    delete trackedNodes;
}

/**
 * Tracks the slot nslot of the node node_id.
 *
 * @return the trackid.
 */
INT32
TrackHeap::trackNodeTags (UINT16 node_id, NodeSlot nslot)
{
    // get the node
    DBGraphNode* gnode = dbgraph->getNode(node_id);
    if (gnode==NULL)
    {
        return -1;
    }

    // get the snname
    QString nname = gnode->getName();
    QString snname = DBGraph::slotedNodeName(nname, nslot.specDimensions, nslot.dimVec);
    //printf("TrackHeap::trackNodeTags !! snname=%s\n", snname.latin1());

    INT32 result = resolveTrackIdForNode(node_id, nslot.specDimensions, nslot.dimVec);
    //printf("Returned from resolveTrackIdForNode\n");
    if (result>=0)
    {
        return result;
    }

/*    printf("Clear this...\n");
    if(nslot.specDimensions!=0) return -1;*/
    result = nextTrackID++;
    INT32 * temp = new INT32(result);
    trackedNodes->insert(snname, temp);

    //nodeTrackTbl[node_id] = result;
    trackIDVector.allocateElement(result);
    trackIDVector[result].setType_NodeTag(node_id,nslot);
    return result;
}

/**
 * Tracks the enter nodes in the slot nslot of the node node_id.
 *
 * @return the trackid.
 */
INT32
TrackHeap::trackEnterNode(UINT16 node_id, NodeSlot nslot)
{
    // get the node
    DBGraphNode* gnode = dbgraph->getNode(node_id);
    if (gnode==NULL)
    {
        return -1;
    }

    // get the snname
    QString nname = gnode->getName();
    QString snname = DBGraph::slotedNodeName(nname,nslot.specDimensions, nslot.dimVec);

    // check whether this node_id/slot is already been tracked
    // should be trackedNodes?
    INT32* ptrack = trackedEnterNodes->find(snname);
    if (ptrack!=NULL)
    {
        return -1;
    }

    // get a track entry
    INT32 result = nextTrackID++;
    trackIDVector.allocateElement(result);
    trackIDVector[result].setType_EnterNode(node_id,nslot);

    // update the hash
    trackedEnterNodes->insert(snname,new INT32(result));
    return result;
}

/**
 * Tracks the exit nodes in the slot nslot of the node node_id.
 *
 * @return the trackid.
 */
INT32
TrackHeap::trackExitNode (UINT16 node_id, NodeSlot nslot)
{
    // get the node
    DBGraphNode* gnode = dbgraph->getNode(node_id);
    if (gnode==NULL)
    {
        return -1;
    }

    // get the snname
    QString nname = gnode->getName();
    QString snname = DBGraph::slotedNodeName(nname,nslot.specDimensions,nslot.dimVec);

    // check whether this node_id/slot is already been tracked
    INT32* ptrack = trackedExitNodes->find(snname);
    if (ptrack!=NULL)
    {
        return -1;
    }

    // get a track entry
    INT32 result = nextTrackID++;
    trackIDVector.allocateElement(result);
    trackIDVector[result].setType_ExitNode(node_id,nslot);

    // update the hash
    trackedExitNodes->insert(snname,new INT32(result));
    return result;
}

/**
 * Tracks the move items in the edge edge_id.
 *
 * @return true if the edge is tracked.
 */
bool
TrackHeap::trackMoveItem (UINT16 edge_id)
{
    DBGraphEdge* edge = dbgraph->getEdge(edge_id);
    if (edge==NULL)
    {
        return false;
    }

    INT32 result;

    // check whether this node_id/slot is already been tracked
    result = edgeTrackTbl[edge_id];
    if (result>=0)
    {
        return true;
    }

    // get a new track entry
    result = nextTrackID;
    //printf ("TrackHeap::trackMoveItem on %d, base trackid =%d, edge_bw=%d\n",(int)edge_id,result,(int)(edge->getBandwidth()));
    // update the tbl
    edgeTrackTbl[edge_id] = result;

    for (int i=0;i<(int)(edge->getBandwidth());i++)
    {
        trackIDVector.allocateElement(result+i);
        trackIDVector[result+i].setType_MoveItem(edge_id);
    }

    // update nextTrackID
    nextTrackID += edge->getBandwidth();

    return true;
}

/**
 * Tracks the enter nodes in the node node_id.
 *
 * @return false.
 */
bool
TrackHeap::trackEnterNode(UINT16 node_id, INT32 *fid, INT32 *lid)
{
    /// @todo implement this
    return false;
}

/**
 * Tracks the exit nodes in the node node_id.
 *
 * @return false.
 */
bool
TrackHeap::trackExitNode (UINT16 node_id, INT32 *fid, INT32 *lid)
{
    /// @todo implement this
    return false;
}

/**
 * Tracks all the slots of the node node_id.
 *
 * @return true if everything is okay.
 */
bool
TrackHeap::trackNodeTags (UINT16 node_id, INT32 *fid, INT32 *lid)
{
    /// @todo implement this
    return false;
}

/**
 * Tracks the cycle tags.
 *
 * @return the trackid.
 */
INT32
TrackHeap::trackCycleTags()
{
    INT32 result = nextTrackID++;
    trackIDVector.allocateElement(result);
    trackIDVector[result].setType_CycleTag();
    cycleTagTrackID = result;
    return result;
}

/**
 * Resets the content of the track heap.
 *
 * @return void.
 */
void
TrackHeap::reset()
{
    firstEffectiveCycle=-9999999;
    nextTrackID=0;
    trackIDVector.clear();
    trackedNodes->clear();
    trackedEnterNodes->clear();
    trackedExitNodes->clear();
    for (int i=0;i<65536;i++)
    {
        edgeTrackTbl[i] = -1;
    }
}

/**
 * Dumps the content of the track heap.
 *
 * @return void.
 */
void
TrackHeap::dumpTrackHeap()
{
    char str[256];

    for (int i=0;i<nextTrackID;i++)
    {
        sprintf(str, FMT32U, i);
        printf("TrackId=%s\n", str);
        trackIDVector[i].dumpTrackId();
    }
}

/**
 * Dumps the content of the track heap.
 *
 * @return void.
 */
void
TrackHeap::dumpRegression()
{
    char str[256];

    for (int i=0;i<nextTrackID;i++)
    {
        sprintf(str, FMT32X, i);
        printf("TR=%s\n", str);
        trackIDVector[i].dumpRegression();
    }
}

/**
 * Computes the size of this object.
 *
 * @return the size of this object.
 */
INT64
TrackHeap::getObjSize() const
{
    return 0;
}

/**
 * Creates a string with the description of this object.
 *
 * @return the description of this object.
 */
QString
TrackHeap::getUsageDescription() const
{
    QString result = "";
    return result;
}

/**
 * Creates a string with the stats of this object.
 *
 * @return the stats of this object.
 */
QString
TrackHeap::getStats() const
{
    QString result = "";
    return (result);
}

/**
 * Compresses the track id vector.
 *
 * @return itself.
 */
ZipObject*
TrackHeap::compressYourSelf(INT32 cycle, bool last)
{
    /// @todo check for self-compression on moveitems,enter/exitNode commands...
	//printf("TrackHeap::compressYourSelf called on cycle=%d,fec=%d,last=%d\n",cycle,firstEffectiveCycle,(int)last);fflush(stdout);
    for (int i=0;i<nextTrackID;i++)
    {
        trackIDVector[i].compressYourSelf(cycle-firstEffectiveCycle,last);
    }
    return this;
}
