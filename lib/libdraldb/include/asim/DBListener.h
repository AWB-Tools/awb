// ==================================================
// Copyright (C) 2003-2006 Intel Corporation
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file  DBListener.h
  */

#ifndef _DRALDB_DBLISTENER_H
#define _DRALDB_DBLISTENER_H

// QT Library
#include <qintdict.h>
#include <qptrlist.h>
#include <qmessagebox.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qvaluevector.h>

// Dreams Library (client)
#include <asim/dralListenerOld.h>

#include "asim/draldb_syntax.h"
#include "asim/DRALTag.h"
#include "asim/AMemObj.h"
#include "asim/StatObj.h"
#include "asim/LogMgr.h"
#include "asim/DBConfig.h"
#include "asim/TagDescVector.h"
#include "asim/ItemTagHeap.h"
#include "asim/DBGraph.h"
#include "asim/StrTable.h"
#include "asim/DBListenerDef.h"
#include "asim/TrackHeap.h"

#define MAX_ITEMID_WARNS 50
#define MAX_TRACKID_WARNS 1000

extern const bool db_listener_debug_on;

/**
  * Short desc.
  * Long  desc.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class DBListener : public AMemObj, public StatObj, public DRAL_LISTENER_OLD_CLASS
{
    public:
        static DBListener* getInstance ();
        static void destroy();

    public:
        // ---- AMemObj Interface methods
        INT64  getObjSize() const;
        QString getUsageDescription() const;
        // ---- StatObj Interface methods
        QString getStats() const;
        // -----------------------------------------------

    protected:
        DBListener();
        virtual ~DBListener();

    // -----------------------------------------------
    // Dral (1) Listener Interface methods
    // -----------------------------------------------
    public:

        void Cycle (
                   UINT64 n
                   );

        void NewItem (
                      UINT32 item_id
                     ) ;

        /**
          * Function description
          */
        void SetTagSingleValue (
                     UINT32 item_id,
                     char* tag_name,
                     UINT64 value,
                     UBYTE time_span
                    ) ;

        /**
          * Function description
          */
        void SetTagString (
                     UINT32 item_id,
                     char* tag_name,
                     char* str,
                     UBYTE time_span
                    ) ;

        /**
          * Function description
          */
        void SetTagSet (
                     UINT32 item_id,
                     char* tag_name,
                     UINT32 nval,
                     UINT64* value,
                     UBYTE time_span
                    ) ;

        /**
          * Function description
          */
        void MoveItems (
                       UINT16 edge_id,
                       UINT32 n,
                       UINT32* item_id
                      ) ;


        /**
          * Function description
          */
        void MoveItemsWithPositions (
                                     UINT16 edge_id,
                                     UINT32 numOfItems,
                                     UINT32 * items,
                                     UINT32 * positions
                                     );

        /**
          * Function description
          */
        void EnterNode (
                        UINT16 node_id,
                        UINT32 item_id,
                        UINT32 slot_index
                       ) ;

        /**
          * Function description
          */
        void ExitNode (
                       UINT16 node_id,
                       UINT32 slot_index
                      );

        /**
          * Function description
          */
        inline void DeleteItem (
                        UINT32 item_id
                        ) ;

        /**
          * Function description
          */
        void EndSimulation() ;

        /**
          * Function description
          */
        void AddNode (
                     UINT16 node_id,
                     char * node_name,
                     UINT16 parent_id,
                     UINT16 instance
                     );

        /**
          * Function description
          */
        void AddEdge (
                     UINT16 sourceNode,
                     UINT16 destNode,
                     UINT16 edge_id,
                     UINT32 bandwidth,
                     UINT32 latency,
                     char * name
                     );

        /**
          * Function description
          */
        void SetCapacity (
                         UINT16 node_id,
                         UINT32 capacity,
                         UINT32 capacities [],
                         UINT16 dimensions
                         );

        /**
          * Function description
          */
        void SetHighWaterMark (
                              UINT16 node_id,
                              UINT32 mark
                              );

        /**
          * Function description
          */
        void Comment (char * comment);

        /**
          * Function description
          */
        void Error (char * error);

        /**
          * Function description
          */
        void NonCriticalError (char * error);

    // -----------------------------------------------
    // Dral (2) Listener Interface methods
    // -----------------------------------------------

        /**
          * Function description
          */
        void Version (UINT16 version);

        void NewNode (UINT16 node_id, char * node_name,UINT16 parent_id, UINT16 instance);

        void NewEdge (UINT16 sourceNode, UINT16 destNode, UINT16 edge_id,
             UINT32 bandwidth, UINT32 latency, char * name);

        void SetNodeLayout (UINT16 node_id, UINT32 capacity, UINT16 dim, UINT32 capacities []);

        void EnterNode (UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position []);
        void ExitNode (UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position []);

        void SetCycleTag(char tag_name [], UINT64 value);
        void SetCycleTagString(char tag_name [], char str []);
        void SetCycleTagSet(char tag_name [], UINT32 nval, UINT64 set []);

        inline void SetItemTag(UINT32 item_id, char tag_name [], UINT64 value);
        inline void SetItemTagString(UINT32 item_id, char tag_name [], char str []);
        inline void SetItemTagSet(UINT32 item_id, char tag_name [], UINT32 nval, UINT64 set []);

        void SetNodeTag(UINT16 node_id, char tag_name [], UINT64 value,UINT16 level, UINT32 list []);
        void SetNodeTagString(UINT16 node_id, char tag_name [], char str [],
             UINT16 level, UINT32 list []);
        void SetNodeTagSet(UINT16 node_id, char tag_name [], UINT16 n, UINT64 set [],
             UINT16 level, UINT32 list []);

        void Comment (UINT32 magic_num, char comment []);

        void CommentBin (UINT16 magic_num, char comment [], UINT32 length);
        
        void StartActivity(UINT64);
        void SetNodeInputBandwidth(UINT16, UINT32);
        void SetNodeOutputBandwidth(UINT16, UINT32);
        void SetTagDescription(char [], char []);

        void SetNodeClock(UINT16, UINT16) {}
        void NewClock(UINT16, UINT64, UINT16, const char []) {}
        void NewClock(UINT16, UINT64, UINT16, UINT16, const char []) {}
        void Cycle(UINT16, UINT64, UINT16) {}

      // -----------------------------------------------
     // Other LiveDB public methods
     // -----------------------------------------------
     public:
        /**
          * Convenience function ...
          */
        void reset();

        /**
          * Convenience function ...
          */
        void flush();

        /**
          * Convenience function ...
          */
        inline bool getLastProcessedEventOk();

        inline INT32 getFirstEffectiveCycle();
        /**
          * Convenience function ...
          */
        inline bool nonCriticalErrors();

        inline bool reachedEOS();
        void Do_EndSimulation();

        inline INT32 getCurrentTraceVersion();
        inline bool  isProcessingDralHeader();

        // Conf Methods
        void  trackItemTags(bool value);

        // exern listeners
        void attachDralListener(DRAL_LISTENER_OLD object);

        void setTrackedEdges (INT32 value);
        void addTrackedEdges (UINT16 edgeid);
        void propagateFirstCycle();
    // -----------------------------------------------
    // -----------------------------------------------
    protected:
        /**
          * Convenience function ...
          */
        bool computeDeleteItem (UINT32 item_id);

        inline void resetTracksLastUsedLinearSlot();

        /**
          * Function description
          */
        inline void tagDiscriminatorAdder(INT32 itemid, LSetTagListNode* node);

        /**
          * Function description
          */
        bool mutableTagDiscriminatorAdder(INT32 itemid, NewTagList* list, LSetTagListNode* node);

        /**
          * Function description
          */
        inline bool hasTag(NewTagList* list,INT32 tagid,UINT64 cycle,LSetTagListNode** matchingNode);

        inline bool trackWarningDumped(INT32);
        inline void addTrackWarned(INT32);
        inline bool itemWarningDumped(UINT32 itemid);
        inline void addItemWarned(UINT32 item);

        void forceMaxIfiPurge();
        
        // dbg
        // void dumpItem(INT32 cidx);

    // -----------------------------------------------
    // -----------------------------------------------
    private:
        // basic state
        INT16 currentTraceVersion;
        INT32 currentCycle;
        INT32 firstEffectiveCycle;

        bool   eoSimulation;
        bool   firstCycle;
        bool   lastProcessedEventOk;
        bool   hasNonCriticalErros;
        bool   processingDralHeader;

        NewItemList*    itemList;

        QIntDict<INT32>* trackWarnHash;
        QIntDict<UINT32>* itemWarnHash;

        LogMgr*         myLogMgr;
        TagDescVector*  tgdescvec;
        StrTable*       strtbl;
        ItemTagHeap*    itHeap;
        TrackHeap*      trHeap;
        DBConfig*       conf;
        DBGraph*        dbGraph;

        bool doTrackItemTags;

        QPtrList<DRAL_LISTENER_OLD_CLASS>* externClients;

        INT32  numTrackedEdges;

        UINT8* lastUsedPosVector;
        UINT16 lastUsedPosVectorIndirection[65536];
        QValueList<UINT16> trackededges;

        INT32  edgeTrackIdVector[65536];
        INT32  _itemidxinid;

    private:
       static DBListener*  _myInstance;
       static UINT64       _itemCnt;
       static UINT64       _dumyCnt;
       static UINT64       _accLiveCycles;
       static UINT64       _accMoveItems;
       static UINT64       _accTags;
       static UINT64       _accItems;
       static UINT64       _cyclesCnt;
};

void
DBListener::SetItemTag(UINT32 item_id, char tag_name [], UINT64 value)
{
    SetTagSingleValue (item_id,tag_name,value,0);
}


void DBListener::SetItemTagString(UINT32 item_id, char tag_name [], char str [])
{
    SetTagString (item_id,tag_name,str,0);
}

void
DBListener::SetItemTagSet(UINT32 item_id, char tag_name [], UINT32 nval, UINT64 set [])
{
    SetTagSet (item_id, tag_name, nval,set,0);
}

INT32
DBListener::getCurrentTraceVersion()
{ return currentTraceVersion; }

bool
DBListener::isProcessingDralHeader()
{ return processingDralHeader; }

bool
DBListener::getLastProcessedEventOk()
{ return lastProcessedEventOk; }

bool
DBListener::trackWarningDumped(INT32 track_id)
{
    return (trackWarnHash->find((long)track_id)!=NULL);
}

void
DBListener::addTrackWarned(INT32 track_id)
{
    trackWarnHash->insert((long)track_id,&track_id);
}

bool
DBListener::itemWarningDumped(UINT32 itemid)
{
    return (itemWarnHash->find((long)itemid)!=NULL);
}

void
DBListener::addItemWarned(UINT32 itemid)
{
    itemWarnHash->insert((long)itemid,&itemid);
    if (itemWarnHash->count() >=MAX_ITEMID_WARNS )
    {
        myLogMgr->addLog("Too many ItemId-related warnings, going to silent mode...");
    }

}

void
DBListener::tagDiscriminatorAdder(INT32 itemid, LSetTagListNode* node)
{
    if (db_listener_debug_on)
    {
        printf ("!!! tagDiscAdder called on tagid=%d\n",node->tagid);fflush(stdout);
    }
    INT32 retidx=-1;
    INT32 tagid;

    // String type
    if (node->isString)
    {
        INT32 stridx = strtbl->addString(node->str);
        if (db_listener_debug_on) {printf("\t\t (string): %s; Index=%d\n",node->str.latin1(),stridx);fflush(stdout);}
        itHeap->newTag(itemid, node->tagid, (UINT64) stridx, node->cycle);
    }
    // Set of values
    else if (node->isSOV)
    {
        if (db_listener_debug_on) { printf(" Cycle %d, (Set of values)...",(int)currentCycle);fflush(stdout); }
        itHeap->newTag(itemid, node->tagid, node->data.sovData.sovl, node->cycle);
    }
    // Simple 64 bits value
    else
    {
        itHeap->newTag(itemid, node->tagid, node->data.value, node->cycle);
        if (db_listener_debug_on) {printf("\t\t (svalue) %d; Index=%d\n",(int)(node->data.value),retidx);fflush(stdout);}
    }
}

bool
DBListener::reachedEOS()
{ return eoSimulation; }


bool
DBListener::hasTag(NewTagList* list,INT32 tagid,UINT64 cycle,LSetTagListNode** matchingNode)
{
    int maxIdx = list->getMaxUsedIdx();
    LSetTagListNode* node=NULL;
    LSetTagListNode* nodeBkp=NULL;
    int idx=maxIdx;

    bool fnd=false;
    bool somethingfnd = false;
    bool tagmatch = false;
    bool goon = (idx>=0);

    while (goon)
    {
        node = (*list)[idx];
        if (node->used)
        {
            --idx;
            goon = (idx>=0) && !somethingfnd;
            continue;
        }

        tagmatch = (tagid == node->tagid);
        fnd = tagmatch && (cycle!=node->cycle);
        somethingfnd = somethingfnd || fnd;
        if (tagmatch && !fnd)
        {
            // more than one value in the same cycle => not mutable, get the last
            node->used=1; // will be ignored at insertion time
            //printf ("OOO ignoring repeated tag (same cycle) tag id=%d,cycle=%d\n",(int)tagid,(int)cycle);fflush(stdout);
        }
        nodeBkp = node;
        --idx;
        goon = (idx>=0) && !somethingfnd;
    }
    //printf (">>> DBListener::hasTag() iterations %d\n",__cnt);fflush(stdout);

    if (somethingfnd)
    {
    	*matchingNode = nodeBkp;
    }
    return somethingfnd;
}

void
DBListener::resetTracksLastUsedLinearSlot()
{
    //printf ("DBListener::resetTracksLastUsedLinearSlot() called\n");
    if (lastUsedPosVector!=NULL)
    {
        //printf ("DBListener:: clear performed over %d elements\n",numTrackedEdges);
        bzero(lastUsedPosVector,sizeof(UINT8)*numTrackedEdges);
    }
}

INT32
DBListener::getFirstEffectiveCycle()
{ return firstEffectiveCycle;}

#endif

