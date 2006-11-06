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
  * @file  DralDB.h
  */

#ifndef _DRALDB_DRALDB_H
#define _DRALDB_DRALDB_H

// QT Library
#include <qstring.h>
#include <qregexp.h>

// DRAL Library
#include <asim/dralClient.h>
#include <asim/dralListenerConverter.h>

// local includes
#include "asim/draldb_syntax.h"
#include "asim/DRALTag.h"
#include "asim/TrackHeap.h"
#include "asim/ItemTagHeap.h"
#include "asim/ItemHandler.h"
#include "asim/DBListener.h"
#include "asim/TagDescVector.h"
#include "asim/DBConfig.h"
#include "asim/DBGraph.h"
#include "asim/LogMgr.h"
#include "asim/StrTable.h"
#include "asim/StatObj.h"
#include "asim/AEVector.h"
#include "asim/AMemObj.h"
#include "asim/DBItoa.h"
#include "asim/Dict2064.h"
#include "asim/DralDBDefinitions.h"
#include "asim/PrimeList.h"
#include "asim/ZipObject.h"
#include "asim/fvaluevector.h"

/**
  * @brief
  * This class implements the database interface with its clients.
  *
  * @description
  * This class uses the 'Facade' design template. All the calls
  * that this class defines are forwarded to the correct object
  * that is the one who resolves the call.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class DralDB
{
    public:
        static DralDB* getInstance ();
        static void destroy();

    public:
        // -------------------------------------------------------------------
        // -- File & Initialization Methods
        // -------------------------------------------------------------------
        bool openDRLFile(QString filename);
        bool closeDRLFile();
        void reset();
        inline INT32 getNumBytesRead();
        inline INT32 getFileSize();

        // -------------------------------------------------------------------
        // -- DRAL Processing Methods
        // -------------------------------------------------------------------
        inline INT32 processEvents(INT32 commands);
        bool  loadDRLHeader();
        bool  processAllEvents();
        inline bool reachedEOS();
        inline INT32 getFirstEffectiveCycle();
        // -------------------------------------------------------------------
        // -- Other DRALClient Listeners can be attached
        // -------------------------------------------------------------------
        inline void attachDralListener(DRAL_LISTENER_OLD object);

        // -------------------------------------------------------------------
        // -- Tracking Setup Methods
        // -------------------------------------------------------------------

        // track all requests
        inline bool trackEnterNode(UINT16 node_id, INT32 *fid, INT32 *lid); //all slots
        inline bool trackExitNode (UINT16 node_id, INT32 *fid, INT32 *lid); //all slots
        inline bool trackNodeTags (UINT16 node_id, INT32 *fid, INT32 *lid); //all slots

        // single trackId requests
        inline INT32 trackNodeTags (UINT16 node_id, NodeSlot nslot);
        inline INT32 trackNodeTags (QString  strnslot);

        inline INT32 trackEnterNode(UINT16 node_id, NodeSlot nslot);
        inline INT32 trackExitNode (UINT16 node_id, NodeSlot nslot);
        inline INT32 trackEnterNode(QString strnslot);
        inline INT32 trackExitNode (QString strnslot);

        inline bool trackMoveItem (UINT16 edge_id);
        inline bool trackMoveItem (QString edgename, QString fromstr, QString tostr);

        inline INT32 trackCycleTags();
        inline void  trackItemTags(bool value);

        // -------------------------------------------------------------------
        // -- Track-Tag Consult Methods
        // -------------------------------------------------------------------
        inline bool getTrackTagValue(INT32 trackId, UINT16 tagId,INT32 cycle, UINT64*  value, UINT32* atcycle=NULL);
        inline bool getTrackTagValue(INT32 trackId, UINT16 tagId,INT32 cycle, QString* value, UINT32* atcycle=NULL);
        inline bool getTrackTagValue(INT32 trackId, UINT16 tagId,INT32 cycle, SOVList** value, UINT32* atcycle=NULL);
        inline bool getTrackTagValue(INT32 trackId, QString tagName,INT32 cycle, UINT64*  value, UINT32* atcycle=NULL);
        inline bool getTrackTagValue(INT32 trackId, QString tagName,INT32 cycle, QString* value, UINT32* atcycle=NULL);
        inline bool getTrackTagValue(INT32 trackId, QString tagName,INT32 cycle, SOVList** value, UINT32* atcycle=NULL);
        inline bool getFormatedTrackTagValue(INT32 trackId, UINT16 tagId,INT32 cycle, QString* fmtvalue, UINT32* atcycle=NULL);
        inline bool getFormatedTrackTagValue(INT32 trackId, QString tagName,INT32 cycle, QString* fmtvalue, UINT32* atcycle=NULL);
		inline bool getItemInSlot(INT32 trackId, INT32 cycle, ItemHandler * handler);

        inline TagIDList* getKnownTagIDs(INT32 trackId);

        // -------------------------------------------------------------------
        // -- MoveItem-Specific Methods
        // -------------------------------------------------------------------
        inline void getMoveItem(ItemHandler * handler, UINT16 edgeid,INT32 cycle,UINT16 pos=0);

        // -------------------------------------------------------------------
        // -- Item-Handler/Tag Consult Methods
        // -------------------------------------------------------------------
        inline bool getFirstItem(ItemHandler * handler);
        inline bool getLastItem(ItemHandler * handler);

        /**
          * Look for a given value in a given context (tag) starting from a given point.
          * @param target_tagid the tag id we are looking for
          * @param target_value the tag value we are looking for
          * @param staring_point vector entry where to start the scanning
          * @return vector position or -1 if <tag-value> not found
          */
        inline void lookForIntegerValue(ItemHandler * handler, UINT16 target_tagid,
            UINT64 target_value, UINT32 cycle, INT32 starting_point = 0);

        inline void lookForStrValue(ItemHandler * handler, UINT16 target_tagid,
            QString target_value, bool csensitive, bool exactMatch, UINT32 cycle,
            INT32 starting_point = 0);

        inline void lookForStrValue(ItemHandler * handler, UINT16 target_tagid,
            QRegExp target_value, UINT32 cycle, INT32 starting_point = 0);

        /**
          * Warning, this can be O(n)!
          */
        inline bool lookForItemId(ItemHandler * handler, INT32 itemid);

        inline INT32 getItemHeapNumItems();
        // -------------------------------------------------------------------
        // -- Global Configuration Methods
        // -------------------------------------------------------------------
        inline bool getAutoPurge() ;
        inline bool getIncrementalPurge() ;
        inline bool getMaxIFIEnabled() ;
        inline bool getTagBackPropagate() ;
        inline bool getGUIEnabled() ;
        inline int  getItemMaxAge() ;
        inline int  getMaxIFI() ;
        inline bool getCompressMutable() ;

        inline void setAutoPurge(bool value) ;
        inline void setIncrementalPurge(bool value) ;
        inline void setMaxIFIEnabled(bool value) ;
        inline void setTagBackPropagate(bool value) ;
        inline void setGUIEnabled(bool value) ;
        inline void setItemMaxAge(int  value) ;
        inline void setMaxIFI(int  value) ;
        inline void setCompressMutable(bool value);
        // -------------------------------------------------------------------
        // -- Tag Descriptor (low level) Methods
        // -------------------------------------------------------------------
        inline INT32 allocateTag (QString desc, TagValueType t, INT16 base=10);
        inline TagValueType getTagValueType(INT32 id);
        inline INT16 getTagValueBase(INT32 id);
        inline QString getTagDescription(INT32 id);
        inline INT32 scanTagName(QString);
        inline void  getFormatedTagValue(QString& buffer,INT32 tagId ,UINT64 value,INT32 pad=0);
        inline INT32 newTagDescriptor(QString tag_name, TagValueType type, INT16 base=10);
        inline QStrList getKnownTags();
        inline QString getTagLongDesc(QString tgName);
        inline void    setTagLongDesc(QString tgName, QString ldesc); 

        //------------------------------------------------------------
        // String table (low level) methods
        //------------------------------------------------------------
        inline QString getString(INT32 id);
        inline INT32 addString(QString);
        inline bool hasString(QString);

        //------------------------------------------------------------
        // Graph access methods
        //------------------------------------------------------------
		QString getGraphDescription();
        void dumpGraphDescription();

        // -------------------------------------------------------------------
        // -- Misc
        // -------------------------------------------------------------------
        inline QString getFormatedTagValue(UINT16 tagid, UINT64 tagvalue);
        inline bool getHasFatalError();

        void dumpTrackHeap();
        inline void dumpRegression();

    protected:
        DralDB();
        ~DralDB();

    private:
        DBListener*    dblistener; // Database listener.
        TrackHeap*     trackHeap; // Track heap.
        ItemTagHeap*   itemTagHeap; // Item tag heap.
        TagDescVector* tagDescVector; // Tag descriptor.
        DBConfig*      dbConfig; // Database configuration.
        DBGraph*       dbGraph; // Dral graph.
        StrTable*      strtable; // Pointer to the string table.
        LogMgr*        logMgr; // Pointer to the log manager.
        QFile*         eventFile; // File with events.
        DRAL_CLIENT    dralClient; // Client of the database.
        DRAL_LISTENER_CONVERTER converter; // Converts the new callbacks to the old ones.
        INT32          numTrackedEdges; // Number of edges that are being tracked.

    private:
        static DralDB* _myInstance; // Instance of DralDB.
};

// -------------------------------------------------------------------
// -- Initialization Methods
// -------------------------------------------------------------------

/**
 * Gets the number of bytes read of the dral client.
 *
 * @return the number of bytes read.
 */
INT32
DralDB::getNumBytesRead()
{
    return dralClient->GetNumBytesRead();
}

/**
 * Gets the file size of the dral client.
 *
 * @return the file size.
 */
INT32
DralDB::getFileSize()
{
    return dralClient->GetFileSize();
}


// -------------------------------------------------------------------
// -- Tracking Setup Methods
// -------------------------------------------------------------------

/**
 * Tracks the edge edge_id. Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::trackMoveItem (UINT16 edge_id)
{
    INT32 id = trackHeap->trackMoveItem (edge_id);
    if (id>=0)
    {
        ++numTrackedEdges;
        //printf("DralDB::trackMoveItem successful on edge=%d, nte=%d\n",(int)edge_id,numTrackedEdges);
        dblistener->addTrackedEdges(edge_id);
        return true;
    }
    return false;
}

/**
 * Tracks the enter nodes of the node node_id. Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::trackEnterNode(UINT16 node_id, INT32 *fid, INT32 *lid)
{
    return trackHeap->trackEnterNode(node_id,fid,lid);
}

/**
 * Tracks the enter nodes of the node node_id. Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::trackExitNode (UINT16 node_id, INT32 *fid, INT32 *lid)
{
    return trackHeap->trackExitNode (node_id, fid, lid);
}

/**
 * Tracks the node tags of all the slots of node_id. Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::trackNodeTags (UINT16 node_id, INT32 *fid, INT32 *lid)
{
    return trackHeap->trackNodeTags (node_id, fid, lid);
}

/**
 * Tracks the cycle tags.
 *
 * @return track id for cycle tags.
 */
INT32
DralDB::trackCycleTags()
{
    return trackHeap->trackCycleTags();
}

/**
 * Sets the database to track or not the item tags.
 *
 * @return void.
 */
void
DralDB::trackItemTags(bool value)
{
    dblistener->trackItemTags(value);
}

/**
 * Tracks the node tags of the slot nslot of node_id. Forwarded
 * to track heap.
 *
 * @return the track id.
 */
INT32
DralDB::trackNodeTags (UINT16 node_id, NodeSlot nslot)
{
    return trackHeap->trackNodeTags(node_id,nslot);
}

/**
 * Tracks the node tags of the node slot strnodeslot. Forwarded
 * to track heap.
 *
 * @return the track id.
 */
INT32
DralDB::trackNodeTags (QString strnodeslot)
{
    NodeSlot nslot;
    QString  normalizednodename;
    bool ok = DBGraph::decodeNodeSlot(strnodeslot,&normalizednodename,&nslot);
    if (!ok)
    {
        return (-1);
    }

    // get node id
    DBGraphNode* nnode = dbGraph->getNode(normalizednodename);
    if (nnode==NULL)
    {
        return -1;
    }

    return trackHeap->trackNodeTags(nnode->getNodeId(),nslot);
}

/**
 * Tracks the enter nodes of the slot strnodeslot. Forwarded
 * to track heap.
 *
 * @return the track id.
 */
INT32
DralDB::trackEnterNode(QString strnodeslot)
{
    NodeSlot nslot;
    QString  normalizednodename;
    bool ok = DBGraph::decodeNodeSlot(strnodeslot,&normalizednodename,&nslot);
    if (!ok)
    {
        return (-1);
    }

    // get node id
    DBGraphNode* nnode = dbGraph->getNode(normalizednodename);
    if (nnode==NULL)
    {
        return -1;
    }

    return trackEnterNode(nnode->getNodeId(),nslot);
}

/**
 * Tracks the exit nodes of the slot strnodeslot. Forwarded
 * to track heap.
 *
 * @return the track id.
 */
INT32
DralDB::trackExitNode (QString strnodeslot)
{
    NodeSlot nslot;
    QString  normalizednodename;
    bool ok = DBGraph::decodeNodeSlot(strnodeslot,&normalizednodename,&nslot);
    if (!ok)
    {
        return (-1);
    }

    // get node id
    DBGraphNode* nnode = dbGraph->getNode(normalizednodename);
    if (nnode==NULL)
    {
        return -1;
    }

    return trackExitNode(nnode->getNodeId(),nslot);
}

/**
 * Tracks the enter nodes of the slot nslot of the node node_id.
 * Forwarded to track heap.
 *
 * @return the track id.
 */
INT32
DralDB::trackEnterNode(UINT16 node_id, NodeSlot nslot)
{
    return trackHeap->trackEnterNode(node_id,nslot);
}

/**
 * Tracks the exit nodes of the slot nslot of the node node_id.
 * Forwarded to track heap.
 *
 * @return the track id.
 */
INT32
DralDB::trackExitNode (UINT16 node_id, NodeSlot nslot)
{
    return trackHeap->trackEnterNode(node_id,nslot);
}

/**
 * Tracks the move items of the edge edgename. Forwarded to track
 * heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::trackMoveItem (QString edgename, QString fromstr, QString tostr)
{
    DBGraphEdge* edge = dbGraph->findEdgeByNameFromTo(edgename,fromstr,tostr);
    if (edge==NULL)
    {
        return false;
    }
    return trackHeap->trackMoveItem(edge->getEdgeId());
}

// -------------------------------------------------------------------
// -- Track-Tag Consult Methods
// -------------------------------------------------------------------

/**
 * Gets the value of the tagId of the trackId in the cycle cycle.
 * Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::getTrackTagValue(INT32 trackId, UINT16 tagId,INT32 cycle, UINT64*  value, UINT32* atcycle)
{
    return trackHeap->getTagValue(trackId,tagId,cycle,value,atcycle);
}

/**
 * Gets the value of the tagId of the trackId in the cycle cycle.
 * Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::getTrackTagValue(INT32 trackId, UINT16 tagId,INT32 cycle, QString* value, UINT32* atcycle)
{
    return trackHeap->getTagValue(trackId,tagId,cycle,value,atcycle);
}

/**
 * Gets the value of the tagId of the trackId in the cycle cycle.
 * Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::getTrackTagValue(INT32 trackId, UINT16 tagId,INT32 cycle, SOVList** value, UINT32* atcycle)
{
    return trackHeap->getTagValue(trackId,tagId,cycle,value,atcycle);
}

/**
 * Gets the value of the tagId of the trackId in the cycle cycle.
 * Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::getTrackTagValue(INT32 trackId, QString tagName,INT32 cycle, UINT64*  value, UINT32* atcycle)
{
    INT32 tagId = tagDescVector->scanTagName(tagName);
    if (tagId<0)
    {
        return false;
    }
    return trackHeap->getTagValue(trackId,(UINT16)tagId,cycle,value,atcycle);
}

/**
 * Gets the value of the tagId of the trackId in the cycle cycle.
 * Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::getTrackTagValue(INT32 trackId, QString tagName,INT32 cycle, QString* value, UINT32* atcycle)
{
    INT32 tagId = tagDescVector->scanTagName(tagName);
    if (tagId<0)
    {
        return false;
    }
    return trackHeap->getTagValue(trackId,(UINT16)tagId,cycle,value,atcycle);
}

/**
 * Gets the value of the tagId of the trackId in the cycle cycle.
 * Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::getTrackTagValue(INT32 trackId, QString tagName,INT32 cycle, SOVList** value, UINT32* atcycle)
{
    INT32 tagId = tagDescVector->scanTagName(tagName);
    if (tagId<0)
    {
        return false;
    }
    return trackHeap->getTagValue(trackId,(UINT16)tagId,cycle,value,atcycle);
}

/**
 * Gets the value of the tagId of the trackId in the cycle cycle.
 * Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::getFormatedTrackTagValue(INT32 trackId, UINT16 tagId,INT32 cycle, QString* fmtvalue, UINT32* atcycle)
{
    return trackHeap->getFormatedTagValue(trackId,(UINT16)tagId,cycle,fmtvalue,atcycle);
}

/**
 * Gets the item inside the slot with track trackId in the cycle cycle.
 *
 * @return if an item is inside.
 */
bool
DralDB::getItemInSlot(INT32 trackId, INT32 cycle, ItemHandler * handler)
{
	return trackHeap->getItemInSlot(trackId, cycle, handler);
}

/**
 * Gets the value of the tagId of the trackId in the cycle cycle.
 * Forwarded to track heap.
 *
 * @return if everything has gone ok.
 */
bool
DralDB::getFormatedTrackTagValue(INT32 trackId, QString tagName,INT32 cycle, QString* fmtvalue, UINT32* atcycle)
{
    INT32 tagId = tagDescVector->scanTagName(tagName);
    if (tagId<0)
    {
        return false;
    }
    return trackHeap->getFormatedTagValue(trackId,(UINT16)tagId,cycle,fmtvalue,atcycle);
}

/**
 * Gets the known tags of the track trackId. Forwarded to track
 * heap.
 *
 * @return the tag name list.
 */
TagIDList* 
DralDB::getKnownTagIDs(INT32 trackId)
{
    return trackHeap->getKnownTagIDs(trackId);
}

// -------------------------------------------------------------------
// Tag Descriptor methods
// -------------------------------------------------------------------

/**
 * Gets the value type of the tag descriptor id.
 *
 * @return the value type.
 */
TagValueType
DralDB::getTagValueType(INT32 id)
{
    return tagDescVector->getTagValueType(id);
}

/**
 * Gets the base of the tag descriptor id.
 *
 * @return the base.
 */
INT16
DralDB::getTagValueBase(INT32 id)
{
    return tagDescVector->getTagValueBase(id);
}

/**
 * Gets the description of the tag descriptor id.
 *
 * @return the description.
 */
QString
DralDB::getTagDescription(INT32 id)
{
    return tagDescVector->getTagDescription(id);
}

/**
 * Gets the index of the tag descriptor for name.
 *
 * @return the tag descriptor index.
 */
INT32
DralDB::scanTagName(QString name)
{
    return tagDescVector->scanTagName(name);
}

/**
 * Adds a new tag descriptor to the vector of tag descriptions.
 *
 * @return the tag descriptor index.
 */
INT32
DralDB::newTagDescriptor(QString tag_name, TagValueType type, INT16 base)
{
    return tagDescVector->tryToAlloc(tag_name,type,base);
}

/**
 * Gets the known tags of the tag descriptor vector.
 *
 * @return the list of tag names.
 */
QStrList
DralDB::getKnownTags()
{
    return tagDescVector->getKnownTags();
}

/**
 * Adds a new tag descriptor to the vector of tag descriptions.
 *
 * @return the tag descriptor index.
 */
INT32
DralDB::allocateTag (QString desc, TagValueType t, INT16 base)
{
    return tagDescVector->tryToAlloc(desc,t,base);
}

/**
 * Gets the long description of the tag with name tgName.
 *
 * @return the description.
 */
QString 
DralDB::getTagLongDesc(QString tgName)
{
    return tagDescVector->getTagLongDesc(tgName);
}

/**
 * Sets the long description of the tag with name tgName.
 *
 * @return void.
 */
void    
DralDB::setTagLongDesc(QString tgName, QString ldesc)
{
    tagDescVector->setTagLongDesc(tgName,ldesc);
} 

// -------------------------------------------------------------------
// String table methods
// -------------------------------------------------------------------

/**
 * Gets the string for the index id.
 *
 * @return the string.
 */
QString
DralDB::getString(INT32 id)
{
    return strtable->getString(id);
}

/**
 * Adds the string v in the string table.
 *
 * @return the index.
 */
INT32
DralDB::addString(QString v)
{
    return strtable->addString(v);
}

/**
 * Looks up for the string v in the string table.
 *
 * @return if the string is in the table.
 */
bool
DralDB::hasString(QString v)
{
    return strtable->hasString(v);
}

// -------------------------------------------------------------------
// Misc
// -------------------------------------------------------------------

/**
 * Gets the formatted value tagvalue for the tagid format.
 *
 * @return the formatted value.
 */
QString
DralDB::getFormatedTagValue(UINT16 tagid, UINT64 tagvalue)
{
    return tagDescVector->getFormatedTagValue(tagid, tagvalue);
}

/**
 * Checks if the dblistener has processed all the events without
 * problems.
 *
 * @return if a fatal error has happened.
 */
bool
DralDB::getHasFatalError()
{
    return ! dblistener->getLastProcessedEventOk();
}


// -------------------------------------------------------------------
// -- DRAL Processing Methods
// -------------------------------------------------------------------

/**
 * Process events the events to the dral clients.
 *
 * @return the result of processing the next event.
 */
INT32
DralDB::processEvents(INT32 commands)
{
    static bool firstTime = true;
    if (firstTime)
    {
        // prepare listener
        firstTime=false;
        dblistener->setTrackedEdges(numTrackedEdges);
        dblistener->propagateFirstCycle();
    }
    return dralClient->ProcessNextEvent(true,commands);
}

// -------------------------------------------------------------------
// -- Global Configuration Methods
// -------------------------------------------------------------------

/**
 * Returns the autpurge value.
 *
 * @return autopurge.
 */
bool
DralDB::getAutoPurge()
{
    return dbConfig->getAutoPurge();
}

/**
 * Returns the incrementalPurge value.
 *
 * @return incrementalPurge.
 */
bool
DralDB::getIncrementalPurge()
{
    return dbConfig->getIncrementalPurge();
}

/**
 * Returns the maxIFIEnabled value.
 *
 * @return maxIFIEnabled.
 */
bool
DralDB::getMaxIFIEnabled()
{
    return dbConfig->getMaxIFIEnabled();
}

/**
 * Returns the tagBackPropagate value.
 *
 * @return tagBackPropagate.
 */
bool
DralDB::getTagBackPropagate()
{
    return dbConfig->getTagBackPropagate();
}

/**
 * Returns the guiEnabled value.
 *
 * @return guiEnabled.
 */
bool
DralDB::getGUIEnabled()
{
    return dbConfig->getGUIEnabled();
}

/**
 * Returns the itemMaxAge value.
 *
 * @return itemMaxAge.
 */
int
DralDB::getItemMaxAge()
{
    return dbConfig->getItemMaxAge();
}

/**
 * Sets the maxIFI value.
 *
 * @return maxIFI.
 */
int
DralDB::getMaxIFI()
{
    return dbConfig->getMaxIFI();
}

/**
 * Returns the compressMutable value.
 *
 * @return compressMutable.
 */
bool
DralDB::getCompressMutable()
{
    return dbConfig->getCompressMutable();
}

/**
 * Sets the autoPurge value.
 *
 * @return void.
 */
void
DralDB::setAutoPurge(bool value) 
{
    dbConfig->setAutoPurge(value);
}

/**
 * Sets the incrementalPurge value.
 *
 * @return void.
 */
void 
DralDB::setIncrementalPurge(bool value)
{
    dbConfig->setIncrementalPurge(value);
}

/**
 * Sets the maxIFIEnabled value.
 *
 * @return void.
 */
void 
DralDB::setMaxIFIEnabled(bool value) 
{
    dbConfig->setMaxIFIEnabled(value);
}

/**
 * Sets the tagBackPropagate value.
 *
 * @return void.
 */
void 
DralDB::setTagBackPropagate(bool value) 
{
    dbConfig->setTagBackPropagate(value);
}

/**
 * Sets the guiEnabled value.
 *
 * @return void.
 */
void
DralDB::setGUIEnabled(bool value)
{
    dbConfig->setGUIEnabled(value);
}

/**
 * Sets the compressMutable value.
 *
 * @return void.
 */
void
DralDB::setCompressMutable(bool value)
{
    dbConfig->setCompressMutable(value);
}

/**
 * Sets the itemMaxAge value.
 *
 * @return void.
 */
void
DralDB::setItemMaxAge(int  value)
{
    dbConfig->setItemMaxAge(value);
}

/**
 * Sets the maxIFI value.
 *
 * @return void.
 */
void
DralDB::setMaxIFI(int  value)
{
    dbConfig->setMaxIFI(value);
}

/**
 * Attaches a dral listener.
 *
 * @return void.
 */
void
DralDB::attachDralListener(DRAL_LISTENER_OLD object)
{
    dblistener->attachDralListener(object);
}

// -------------------------------------------------------------------
// -- Item-Handler Consult Methods
// -------------------------------------------------------------------

/**
 * Tries to find the target_value for the target_tagid.
 *
 * @return void.
 */
void
DralDB::lookForIntegerValue(ItemHandler * handler, UINT16 target_tagid,UINT64 target_value,
        UINT32 cycle, INT32 starting_point)
{
    itemTagHeap->lookForIntegerValue(handler, target_tagid, target_value, cycle, starting_point);
}

/**
 * Tries to find the target_value for the target_tagid.
 *
 * @return void.
 */
void
DralDB::lookForStrValue(ItemHandler * handler, UINT16 target_tagid,QString target_value,
        bool csensitive, bool exactMatch,UINT32 cycle,INT32 starting_point)
{
    itemTagHeap->lookForStrValue(handler, target_tagid, target_value, csensitive, exactMatch, cycle, starting_point);
}

/**
 * Tries to find the target_value for the target_tagid.
 *
 * @return void.
 */
void
DralDB::lookForStrValue(ItemHandler * handler, UINT16 target_tagid,QRegExp target_value,
        UINT32 cycle,INT32 starting_point)
{
    itemTagHeap->lookForStrValue(handler, target_tagid, target_value, cycle, starting_point);
}

/**
 * Sets the ItemHandler to point to the first item in the
 * ItemTagHeap.
 *
 * @return if at least one item is in the heap.
 */
bool
DralDB::getFirstItem(ItemHandler * handler)
{
    return itemTagHeap->getFirstItem(handler);
}

/**
 * Sets the ItemHandler to point to the last item in the
 * ItemTagHeap.
 *
 * @return if at least one item is in the heap.
 */
bool
DralDB::getLastItem(ItemHandler * handler)
{
    return itemTagHeap->getLastItem(handler);
}

/**
 * Searches the item itemid in the ItemTagHeap. The call is
 * forwarded to the ItemTagHeap.
 *
 * @return if the item is found.
 */
bool
DralDB::lookForItemId(ItemHandler * handler, INT32 itemid)
{
    //printf("DralDB::lookForItemId(itemid=%d no starting point)\n",itemid);fflush(stdout);
    return itemTagHeap->lookForItemId(handler, itemid);
}

/**
 * Get the number of heap items.
 *
 * @return the number of items in the ItemTagHeap.
 */
INT32
DralDB::getItemHeapNumItems()
{
    return itemTagHeap->getNumItems();
}

/**
 * Gets the end of simulation flag. Forwarded to dblistener.
 *
 * @return if the simulation has ended.
 */
bool
DralDB::reachedEOS()
{
    return dblistener->reachedEOS();
}

/**
 * Get the first effective cycle of the actual drl trace.
 *
 * @return the first efective cycle.
 */
INT32
DralDB::getFirstEffectiveCycle()
{
    return dblistener->getFirstEffectiveCycle();
}

/**
 * Dumps to the standard output all the contents of the DataBase.
 *
 * @return void.
 */
void
DralDB::dumpRegression()
{
    trackHeap->dumpRegression();
    itemTagHeap->dumpRegression();
}

// -------------------------------------------------------------------
// -- MoveItem Consult Methods
// -------------------------------------------------------------------

/**
 * Finds the position where the move item has happened and sets the
 * ItemHandler pointing there.
 *
 * @return void.
 */
void
DralDB::getMoveItem(ItemHandler * handler, UINT16 edgeid,INT32 cycle,UINT16 pos)
{
    trackHeap->getMoveItem(handler, edgeid,cycle,pos);
}

#endif
