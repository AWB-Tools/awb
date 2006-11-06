/*
 * Copyright (C) 2003-2006 Intel Corporation
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
  * @file DBListener.cpp
  */

#include "asim/DBListener.h"

#include <qprogressdialog.h>
#include <qapplication.h>

#include "asim/PrimeList.h"
#include "asim/DBItoa.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

DBListener* DBListener::_myInstance = NULL;

UINT64 DBListener::_itemCnt=0;
UINT64 DBListener::_dumyCnt=0;
UINT64 DBListener::_accLiveCycles=0;
UINT64 DBListener::_accMoveItems=0;
UINT64 DBListener::_accTags=0;
UINT64 DBListener::_accItems=0;
UINT64 DBListener::_cyclesCnt=0;

const bool db_listener_debug_on = false;

#define DBLISTENER_DISPATCH_LISTENERS(cmd) \
{\
    DRAL_LISTENER_OLD listener;\
    for ( listener = externClients->first(); listener!=NULL; listener = externClients->next() )\
    {\
      cmd \
    }\
}\

// -------------------------------------------------------------------
// Constructors & co
// -------------------------------------------------------------------
DBListener*
DBListener::getInstance()
{
    if (_myInstance==NULL)
        _myInstance = new DBListener();

    return _myInstance;
}

void
DBListener::destroy()
{
    if (_myInstance!=NULL) { delete _myInstance; }
}

DBListener::DBListener()
{
    itemList = new NewItemList(PrimeList::nearPrime(500000));
    Q_ASSERT(itemList!=NULL);
    itemList->setAutoDelete(true);

    trackWarnHash = new QIntDict<INT32>(PrimeList::nearPrime(MAX_TRACKID_WARNS));
    Q_ASSERT(trackWarnHash!=NULL);

    itemWarnHash = new QIntDict<UINT32>(PrimeList::nearPrime(MAX_ITEMID_WARNS));
    Q_ASSERT(itemWarnHash!=NULL);

    myLogMgr  = LogMgr::getInstance();
    tgdescvec = TagDescVector::getInstance();
    strtbl    = StrTable::getInstance();
    itHeap    = ItemTagHeap::getInstance();
    conf      = DBConfig::getInstance();
    dbGraph   = DBGraph::getInstance();
    trHeap    = TrackHeap::getInstance();

    externClients = new QPtrList<DRAL_LISTENER_OLD_CLASS>();
    lastUsedPosVector = NULL;

    reset();
}

DBListener::~DBListener()
{
    if (itemList!=NULL) delete itemList;
    delete externClients;
}


// -------------------------------------------------------------------
// DRAL LISTENER methods
// -------------------------------------------------------------------

void
DBListener::Cycle (UINT64 n)
{
    static int dcnt=0;
    ++dcnt;

    /// @todo remove this when the StartTrace() callback gets implemented...
    if (firstCycle)
    {
        StartActivity(n);
    }

    //printf("\tDBListener::cycle on %d, current=%d\n",(int)n,(int)currentCycle);fflush(stdout);
	
	// sanity check
	if ((INT32)(n) < currentCycle)
	{
		myLogMgr->addLog("DralDB Error: invalid cycle sequence, this is likely due to a corrupted drl file.");
		myLogMgr->addLog("Hitting CTRL+C during the trace generation is likely to produce CRC gzip errors!");
		lastProcessedEventOk = false;
		return;
	}
	
	if ((n-currentCycle)>10)
	{
		myLogMgr->addLog("DralDB Warning: a big cycle gap detected. This is likely due to a corrupted drl file.");
		myLogMgr->addLog("Hitting CTRL+C during the trace generation is likely to produce CRC gzip errors!");
	}
	
    resetTracksLastUsedLinearSlot();
    currentCycle = n;

    // for for auto-compression mechanisms
    if (!(dcnt%CYCLE_CHUNK_SIZE))
    {
        trHeap = (TrackHeap*) (trHeap->compressYourSelf(currentCycle));
    }

    // check for item list size
    if ( (conf->getMaxIFIEnabled()) && ((int)(itemList->count())  > conf->getMaxIFI()) ) forceMaxIfiPurge();

    // check for age purges
    if (conf->getIncrementalPurge() && (itemList->count()>2048) && (!(dcnt%256)))
    {
        // itarate throught the live guys:
        QIntDictIterator<LNewItemListNode> it(*itemList);
        int cnt=0;
        UINT64 maxAge = (UINT64)(conf->getItemMaxAge());
        while(it.current())
        {
            //printf ("purging item %d\n",it.current()->item_id);fflush(stdout);
            if ( (currentCycle - it.current()->cycle)>=maxAge)
            {
                SetTagSingleValue (it.current()->item_id,AGEPURGE_STR_TAG,1,0);
                bool deleted = computeDeleteItem(it.current()->item_id);
                if (!deleted) { ++it; }
                // propagate the death to other clients
                DBLISTENER_DISPATCH_LISTENERS(listener->DeleteItem(it.current()->item_id);)
                ++cnt;
                // dbg
                //printf ("cycle %d itemId %d\n",(int)(it.current()->cycle),(int)(it.current()->item_id));
            }
        }
        if (cnt>0)
        {
            myLogMgr->addLog("DralDB Age-Purged "+QString::number(cnt)+" in flight items.");
        }
    }

    // some statistics...
    ++_cyclesCnt;
    _accItems += itemList->count();

    // extern listeners
    if (!firstCycle)
    {
        DBLISTENER_DISPATCH_LISTENERS(listener->Cycle(n);)
    }

    ///@todo remove this when StartActicity callback works
    // not the first anymore
    firstCycle=false;
}

void
DBListener::propagateFirstCycle()
{
    DBLISTENER_DISPATCH_LISTENERS(listener->Cycle(firstEffectiveCycle);)
    resetTracksLastUsedLinearSlot();
}

void
DBListener::StartActivity(UINT64 _firstEffectiveCycle)
{
    //printf ("DBListener::StartActivity called with %d\n",(int)_firstEffectiveCycle);
	lastProcessedEventOk = true;
	processingDralHeader = false;
	firstEffectiveCycle = (INT32)_firstEffectiveCycle;
	itHeap->setFirstEffectiveCycle(firstEffectiveCycle);
	trHeap->setFirstEffectiveCycle(firstEffectiveCycle);
    _itemidxinid = tgdescvec->getItemIdxIn_TagId();
}

void
DBListener::NewItem (UINT32 item_id)
{
    //if (!lastProcessedEventOk) { return; }
    //printf("\tDBListener::NewItem on id=%d\n",(int)item_id);fflush(stdout);fflush(stderr);
    LNewItemListNode* node = new LNewItemListNode(currentCycle,item_id);
    Q_ASSERT(node!=NULL);
    itemList->insert((long)item_id,node);

    // stats
    _itemCnt++;

    // extern listeners
    DBLISTENER_DISPATCH_LISTENERS(listener->NewItem(item_id);)
}


void
DBListener::SetTagSingleValue (UINT32 item_id,char* tag_name,UINT64 value,UBYTE time_span)
{
    if (!doTrackItemTags) { return; }

    //printf ("SetTagSingleValue called on item_id=%u,tgname=%s,value=%llu\n",item_id,tag_name,value);fflush(stdout);

    bool mutant=false;
    INT32 tagid;
    QString qtag_name(tag_name);

    // I need to check that a "new item" was launched for item_id
    LNewItemListNode* newItemNode = itemList->find((long)item_id);
    if (newItemNode!=NULL)
    {
        tagid = tgdescvec->tryToAlloc(qtag_name.stripWhiteSpace(),TagIntegerValue);
        //printf("tagname=%s, tgid=%d\n",qtag_name.latin1(),tagid);
        Q_ASSERT(tagid>=0);

        LSetTagListNode* node = new LSetTagListNode(currentCycle, tagid, value);
        LSetTagListNode* matchingNode=NULL;
        Q_ASSERT(node!=NULL);
        mutant = hasTag(newItemNode->getMyTags(),tagid,currentCycle,&matchingNode);
        newItemNode->getMyTags()->append(node);
        if (mutant)
        {
            Q_ASSERT(matchingNode!=NULL);
        	matchingNode->isMutable=1;
        	node->isMutable=1;
        }
    }
    else
    {
        if (!itemWarningDumped(item_id) && itemWarnHash->count()<MAX_ITEMID_WARNS)
        {
            QString err ("DralDB Warning: DRAL incoherence detected, setting a tag after item deletion or before creation on ITEMID ");
            err = err + QString::number(item_id,10);
            myLogMgr->addLog(err);
            hasNonCriticalErros=true;
            addItemWarned(item_id);
        }
    }
    // stats
    ++DBListener::_accTags;

    // extern listeners
    DBLISTENER_DISPATCH_LISTENERS(listener->SetTagSingleValue(item_id,tag_name,value,time_span);)
}


void
DBListener::SetTagString (UINT32 item_id,char* tag_name,char* str,UBYTE time_span)
{
    if (!lastProcessedEventOk || !doTrackItemTags ) { return; }
    bool mutant=false;
    INT32 tagid;
    QString qtag_name(tag_name);
    QString qstr(str);

    //printf("\tDBListener::SetTagString id=%d, tag=%s, val=%s\n",
    //(int)item_id,tag_name,str);fflush(stdout);fflush(stderr);
    // I need to check that a "new item" was launched for item_id
    LNewItemListNode* newItemNode = itemList->find((long)item_id);
    if (newItemNode!=NULL)
    {
        tagid = tgdescvec->tryToAlloc(qtag_name.stripWhiteSpace(),TagStringValue);
        //printf("tagname=%s, tgid=%d\n",qtag_name.latin1(),tagid);
        Q_ASSERT(tagid>=0);

        LSetTagListNode* node = new LSetTagListNode(currentCycle, tagid, qstr.stripWhiteSpace());
        LSetTagListNode* matchingNode=NULL;
        Q_ASSERT(node!=NULL);
        mutant = hasTag(newItemNode->getMyTags(),tagid,currentCycle,&matchingNode);
        newItemNode->getMyTags()->append(node);
        if (mutant)
        {
            Q_ASSERT(matchingNode!=NULL);
        	matchingNode->isMutable=1;
        	node->isMutable=1;
        }
    }
    else
    {
        if (!itemWarningDumped(item_id) && itemWarnHash->count()<MAX_ITEMID_WARNS)
        {
            QString err("DralDB Warning: DRAL incoherence detected, setting a tag after item deletion or before creation on ITEMID ");
            err = err + QString::number(item_id,10);
            myLogMgr->addLog(err);
            hasNonCriticalErros=true;
            addItemWarned(item_id);
        }
    }
    // stats
    ++DBListener::_accTags;
    // extern listeners
    DBLISTENER_DISPATCH_LISTENERS(listener->SetTagString(item_id,tag_name,str,time_span);)
}


void
DBListener::SetTagSet (UINT32 item_id,char* tag_name,UINT32 nval,
               UINT64* value,UBYTE time_span)
{
    if (!lastProcessedEventOk || !doTrackItemTags) { return; }
    bool mutant=false;
    INT32 tagid;
    QString qtag_name(tag_name);
    //printf("\tDBListener::SetTagSet id=%d, tag=%s, nval=%d\n",
    //(int)item_id,tag_name,nval);fflush(stdout);fflush(stderr);
    // I need to check that a "new item" was launched for item_id
    LNewItemListNode* newItemNode = itemList->find((long)item_id);
    if (newItemNode!=NULL)
    {
        tagid = tgdescvec->tryToAlloc(qtag_name.stripWhiteSpace(),TagSetOfValues);
        //printf("tagname=%s, tgid=%d\n",qtag_name.latin1(),tagid);
        Q_ASSERT(tagid>=0);

        LSetTagListNode* node = new LSetTagListNode(currentCycle,tagid, nval, value);

        Q_ASSERT(node!=NULL);
        LSetTagListNode* matchingNode=NULL;
        mutant = hasTag(newItemNode->getMyTags(),tagid,currentCycle,&matchingNode);
        newItemNode->getMyTags()->append(node);
        if (mutant)
        {
            Q_ASSERT(matchingNode!=NULL);
        	matchingNode->isMutable=1;
        	node->isMutable=1;
        }
    }
    else
    {
        if (!itemWarningDumped(item_id) && itemWarnHash->count()<MAX_ITEMID_WARNS)
        {
            QString err("DralDB Warning: DRAL incoherence detected, setting a tag after item deletion or before creation on ITEMID ");
            err = err + QString::number(item_id,10);
            myLogMgr->addLog(err);
            hasNonCriticalErros=true;
            addItemWarned(item_id);
        }
    }
    // stats
    ++DBListener::_accTags;
    // extern listeners
    DBLISTENER_DISPATCH_LISTENERS(listener->SetTagSet(item_id,tag_name,nval,value,time_span);)
}


void
DBListener::MoveItems (UINT16 edge_id,UINT32 n,UINT32* item_id)
{
    //printf ("DBListener::MoveItems called on edge=%d,n=%d cycle = %d items:",(int)edge_id,(int)n,(int)currentCycle);
    //for (int k=0;k<n;k++) { printf("%u ",item_id[k]); printf("\n"); }

    //if (!lastProcessedEventOk) { return; }

    // check that the number of moved items is less or equal the bw of the edge
    INT32 bw = dbGraph->getEdgeBandwidth(edge_id);

    // multiple moveitems can be launched to the same edge/cycle:
    UINT8 nextPos = lastUsedPosVector[lastUsedPosVectorIndirection[edge_id]];
    //printf ("DBListener::MoveItems bw=%d, nextPos=%d, indirection idx=%d\n",(int)bw,(int)nextPos,(int)(lastUsedPosVectorIndirection[edge_id]));

    if (bw<=0)
    {
        QString err ("DralDB Warning: DRAL incoherence detected, moveitem on edge with null bandwidth on EDGEID ");
        err = err + QString::number(edge_id,10);
        myLogMgr->addLog(err);
        hasNonCriticalErros=true;
        DBLISTENER_DISPATCH_LISTENERS(listener->MoveItems(edge_id,n,item_id);)
        return;
    }


    // resolve track id...
    INT32 trackId = trHeap->resolveTrackIdEdge(edge_id);
    if (trackId<0)
    {
        //printf ("DBListener::MoveItems unrec track!?\n");
        DBLISTENER_DISPATCH_LISTENERS(listener->MoveItems(edge_id,n,item_id);)
        return;
    }

    for (UINT32 i=0;i<n;i++)
    {
        // attach it to the item until it gets deleted.
        LNewItemListNode* newItemNode = itemList->find((long)item_id[i]);
        if (newItemNode!=NULL)
        {
            if (nextPos<bw)
            {
                MoveItemList* movs = newItemNode->getMyMovs();
                movs->append(LMoveItemListNode(currentCycle,edge_id,nextPos));
                trHeap->incPendingCnt(trackId+nextPos,_itemidxinid,currentCycle);
                ++nextPos;
            }
            else
            {
                if (!itemWarningDumped(item_id[i]) && itemWarnHash->count()<MAX_ITEMID_WARNS)
                {
                    QString err("DralDB Warning: DRAL incoherence detected, moving more items than edge bandwidth. Edge id=");
                    err = err + QString::number(edge_id)+". Item id=";
                    err = err + QString::number(item_id[i],10) + ". Cycle=";
                    err = err + QString::number(currentCycle);
                    myLogMgr->addLog(err);
                    hasNonCriticalErros=true;
                    addItemWarned(item_id[i]);
                }
            }
        }
        else
        {
            if (!itemWarningDumped(item_id[i]) && itemWarnHash->count()<MAX_ITEMID_WARNS)
            {
                QString err ("DralDB Warning: DRAL incoherence detected, moveitem on item after item deletion or before creation on ITEMID ");
                err = err + QString::number(item_id[i],10);
                myLogMgr->addLog(err);
                hasNonCriticalErros=true;
                addItemWarned(item_id[i]);
            }
        }
    }
    lastUsedPosVector[lastUsedPosVectorIndirection[edge_id]] = nextPos;
    DBLISTENER_DISPATCH_LISTENERS(listener->MoveItems(edge_id,n,item_id);)
}

void
DBListener::MoveItemsWithPositions (UINT16 edge_id, UINT32 n,UINT32* item_id, UINT32* positions)
{
    DBLISTENER_DISPATCH_LISTENERS(listener->MoveItemsWithPositions(edge_id,n,item_id,positions);)
}

void
DBListener::DeleteItem (UINT32 item_id)
{
    //if (!lastProcessedEventOk) { return; }
    //printf("\tDBListener::DeleteItem id=%d\n",(int)item_id);fflush(stdout);fflush(stderr);

    // I need to check that a "new item" was launched for item_id
    LNewItemListNode* newItemNode = itemList->find((long)item_id);
    if (newItemNode==NULL)
    {
        if (!itemWarningDumped(item_id) && itemWarnHash->count()<MAX_ITEMID_WARNS)
        {
            QString err("DralDB Warning: DRAL incoherence detected, deleting item twice or before creation on ITEMID ");
            err = err + QString::number(item_id,10);
            myLogMgr->addLog(err);
            hasNonCriticalErros=true;
            addItemWarned(item_id);
        }
    }
    else
    {
    	_accLiveCycles += (currentCycle - newItemNode->cycle + 1);
        computeDeleteItem(item_id);
    }
    DBLISTENER_DISPATCH_LISTENERS(listener->DeleteItem(item_id);)
}

void
DBListener::EndSimulation()
{
    //printf ("DBListener::EndSimulation() called...\n");fflush(stdout);
    // to prevent spureous timer events
    if (eoSimulation) return;
    Do_EndSimulation();
    DBLISTENER_DISPATCH_LISTENERS(listener->EndSimulation();)
}

void
DBListener::Do_EndSimulation()
{
    //printf ("DBListener::Do_EndSimulation() called...\n");fflush(stdout);
    eoSimulation=true;

    // purge last cycles of information.
    Cycle(currentCycle+1);

    // call delete items over non explicitely deleted items if auto purge is on
    if (conf->getAutoPurge() && (itemList->count()>0) )
    {
        INT32 nitems = itemList->count();
        //printf ("nitems=%d\n",nitems);

        QProgressDialog* progress = NULL;
        if (conf->getGUIEnabled())
        {
            //printf ("DRALDB GUI ENABLED on ENDSIMULATION...\n");fflush(stdout);
            progress = new QProgressDialog ("DralDB is commiting EOF Auto-Purged Items...", "&Abort", nitems ,NULL, "commitprogress",TRUE);
            progress->setMinimumDuration(250);
        }

        // itarate throught the live guys:
        QIntDictIterator<LNewItemListNode> it(*itemList);
        //printf ("DBListener::purging EOF...\n");fflush(stdout);
        INT32 cnt=0;
        while (it.current()!=NULL)
        {
            //printf ("purging item %u at 0x%x\n",it.current()->item_id,(void*)it.current());fflush(stdout);
            SetTagSingleValue (it.current()->item_id,EOFPURGE_STR_TAG,1,0);
            bool deleted = computeDeleteItem (it.current()->item_id);

        	if ( (cnt%128==0) && conf->getGUIEnabled())
        	{
        	    //printf ("dialog refreshed with cnt=%d!\n",cnt);fflush(stdout);
                progress->setProgress(cnt);
                qApp->processEvents();
                if ( progress->wasCancelled() )
                {
                	myLogMgr->addLog("DralDB EOF Purge cancelled by user!");
                	break;
                }
            }
            if (!deleted) ++it;
            ++cnt;
        }

        if (conf->getGUIEnabled())
        {
            progress->setProgress(nitems);
            delete progress;
        }

        //printf ("DBListener::purge done\n");fflush(stdout);
        myLogMgr->addLog("DralDB Purged "+QString::number(cnt)+" in flight items.");
    }

    // clear all the tmp structs...
    itemList->clear();

    // last compression step
    trHeap = (TrackHeap*) (trHeap->compressYourSelf(currentCycle,true));

    // just to debugg
    //ColDescriptor::dumpStats();
    //double propDumy = (double)_dumyCnt/(double)_itemCnt;printf("dumy prop %f\n",propDumy);fflush(stdout);
}

void
DBListener::Comment (char * comment)
{
    /** @todo check if something more needs to be done */
    DBLISTENER_DISPATCH_LISTENERS(listener->Comment(comment);)
}

void
DBListener::AddNode (UINT16 node_id,char * node_name,UINT16 parent_id,UINT16 instance)
{
    QString qnodename(node_name);
    // errors already dumped into the log file by ConfigDB class
    lastProcessedEventOk = dbGraph->addNode (qnodename.stripWhiteSpace(),
                         node_id,parent_id,instance);
    DBLISTENER_DISPATCH_LISTENERS(listener->AddNode(node_id,node_name,parent_id,instance);)
}

void
DBListener::AddEdge (UINT16 sourceNode, UINT16 destNode, UINT16 edge_id,UINT32 bandwidth,
                 UINT32 latency,char* name)
{
//printf("DBListener::AddEdge snode=%d,dnode=%d, edge=%d,bw=%d,lat=%d,name=%s\n",(int)sourceNode,(int)destNode,(int)edge_id,(int)bandwidth,(int)latency,name);
    QString qname(name);
    // errors already dumped into the log file by ConfigDB class
    lastProcessedEventOk = dbGraph->addEdge (sourceNode,
                         destNode,edge_id,bandwidth,latency,
                         qname.stripWhiteSpace());

    DBLISTENER_DISPATCH_LISTENERS(listener->AddEdge(sourceNode,destNode,edge_id,bandwidth,latency,name);)
}

void
DBListener::SetCapacity (UINT16 node_id,UINT32 capacity,
                     UINT32 capacities [], UINT16 dimensions)
{
    lastProcessedEventOk = dbGraph->setNodeLayout(node_id,dimensions,capacities);
    DBLISTENER_DISPATCH_LISTENERS(listener->SetCapacity(node_id,capacity,capacities,dimensions);)
}

void
DBListener::SetHighWaterMark (UINT16 node_id,UINT32 mark)
{
    // ignored by the dbase
    DBLISTENER_DISPATCH_LISTENERS(listener->SetHighWaterMark(node_id,mark);)
}

void
DBListener::Error (char * error)
{
    lastProcessedEventOk = false;
    myLogMgr->addLog(QString(error));
    DBLISTENER_DISPATCH_LISTENERS(listener->Error(error);)
    return;
}

void
DBListener::NonCriticalError (char * error)
{
    myLogMgr->addLog(QString(error));
    DBLISTENER_DISPATCH_LISTENERS(listener->NonCriticalError(error);)
    return;
}

// ------------------------------------------------------------------
// -- Dral 2.0
// ------------------------------------------------------------------

void
DBListener::Version (UINT16 version)
{
    currentTraceVersion = version;
    if (currentTraceVersion<2)
    {
        myLogMgr->addLog("DralDB Warning: this an old DRAL file and will be processed with backward proopagation policy\n");
    }
    itHeap->setDralVersion(version);
    DBLISTENER_DISPATCH_LISTENERS(listener->Version(version);)
}

void
DBListener::NewNode (UINT16 node_id, char * node_name,UINT16 parent_id, UINT16 instance)
{ AddNode (node_id,node_name,parent_id, instance); }

void
DBListener::NewEdge (UINT16 sourceNode, UINT16 destNode, UINT16 edge_id,
     UINT32 bandwidth, UINT32 latency, char * name)
{ AddEdge(sourceNode,destNode,edge_id,bandwidth,latency,name); }

void
DBListener::SetNodeLayout (UINT16 node_id, UINT32 capacity, UINT16 dim, UINT32 capacities [])
{ SetCapacity (node_id,capacity,capacities,dim); }

void
DBListener::EnterNode (UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position [])
{
    //if (!lastProcessedEventOk) { return; }

    // resolve track id...
    INT32 trackId = trHeap->resolveTrackIdForEnterNode(node_id,dim,position);
    if (trackId<0)
    {
        DBLISTENER_DISPATCH_LISTENERS(listener->EnterNode(node_id,item_id,dim,position);)
        return;
    }

    // attach it to the item until it gets deleted.
    LNewItemListNode* newItemNode = itemList->find((long)item_id);
    if (newItemNode!=NULL)
    {
        EENodeList* enterNodes = newItemNode->getMyEnterNodes();
        enterNodes->append(LEENodeListNode(currentCycle,trackId));
    }
    else
    {
        if (!itemWarningDumped(item_id) && itemWarnHash->count()<MAX_ITEMID_WARNS)
        {
            QString err ("DralDB Warning: DRAL incoherence detected, enternode on item after item deletion or before creation on ITEMID ");
            err = err + QString::number(item_id,10);
            myLogMgr->addLog(err);
            hasNonCriticalErros=true;
            addItemWarned(item_id);
        }
    }

    DBLISTENER_DISPATCH_LISTENERS(listener->EnterNode(node_id,item_id,dim,position);)
}

void
DBListener::EnterNode (UINT16 node_id,UINT32 item_id,UINT32 slot_index)
{
    /// @todo check for bk compatibility on dral 1 traces...
    EnterNode(node_id,item_id,1,&slot_index);
}

void
DBListener::ExitNode (UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position [])
{
    //if (!lastProcessedEventOk) { return; }

    // resolve track id...
    INT32 trackId = trHeap->resolveTrackIdForExitNode(node_id,dim,position);
    if (trackId<0)
    {
        DBLISTENER_DISPATCH_LISTENERS(listener->ExitNode(node_id,item_id,dim,position);)
        return;
    }

    // attach it to the item until it gets deleted.
    LNewItemListNode* newItemNode = itemList->find((long)item_id);
    if (newItemNode!=NULL)
    {
        EENodeList* exitNodes = newItemNode->getMyExitNodes();
        exitNodes->append(LEENodeListNode(currentCycle,trackId));
    }
    else
    {
        if (!itemWarningDumped(item_id) && itemWarnHash->count()<MAX_ITEMID_WARNS)
        {
            QString err ("DralDB Warning: DRAL incoherence detected, exitnode on item after item deletion or before creation on ITEMID ");
            err = err + QString::number(item_id,10);
            myLogMgr->addLog(err);
            hasNonCriticalErros=true;
            addItemWarned(item_id);
        }
    }
    DBLISTENER_DISPATCH_LISTENERS(listener->ExitNode(node_id,item_id,dim,position);)
}

void
DBListener::ExitNode (UINT16 node_id,UINT32 slot_index)
{
    /// @todo check for bk compatibility on dral 1 traces...
    ExitNode(node_id,0,1,&slot_index);
}

void
DBListener::SetCycleTag(char tag_name [], UINT64 value)
{
    INT32 trackId = trHeap->resolveTrackIdForCycleTag();
    if (trackId<0)
    {
        DBLISTENER_DISPATCH_LISTENERS(listener->SetCycleTag(tag_name,value);)
        return;
    }

    // gt tag id
    QString qtag_name(tag_name);
    UINT16 tagId = tgdescvec->tryToAlloc(qtag_name.stripWhiteSpace(),TagIntegerValue);
    // put it
    bool ok = trHeap->addTagValue(trackId,tagId,currentCycle,value);
    lastProcessedEventOk = lastProcessedEventOk && ok;

    DBLISTENER_DISPATCH_LISTENERS(listener->SetCycleTag(tag_name,value);)
}

void
DBListener::SetCycleTagString(char tag_name [], char str [])
{
    INT32 trackId = trHeap->resolveTrackIdForCycleTag();
    if (trackId<0)
    {
        DBLISTENER_DISPATCH_LISTENERS(listener->SetCycleTagString(tag_name,str);)
        return;
    }

    // gt tag id
    QString qtag_name(tag_name);
    UINT16 tagId = tgdescvec->tryToAlloc(qtag_name.stripWhiteSpace(),TagIntegerValue);

    // put it
    bool ok = trHeap->addTagValue(trackId,tagId,currentCycle,QString(str));
    lastProcessedEventOk = lastProcessedEventOk && ok;

    DBLISTENER_DISPATCH_LISTENERS(listener->SetCycleTagString(tag_name,str);)
}

void
DBListener::SetCycleTagSet(char tag_name [], UINT32 nval, UINT64 set [])
{
    DBLISTENER_DISPATCH_LISTENERS(listener->SetCycleTagSet(tag_name,nval,set);)
}

void
DBListener::SetNodeTag(UINT16 node_id, char tag_name [], UINT64 value,UINT16 level, UINT32 list [])
{
    // resolve track id...
    INT32 trackId = trHeap->resolveTrackIdForNode(node_id,level,list);
    if (trackId<0)
    {
        DBLISTENER_DISPATCH_LISTENERS(listener->SetNodeTag(node_id,tag_name,value,level,list);)
        return;
    }
    //printf("setnodetag on node=%d,trackid=%d\n",(int)node_id,(int)trackId);
    // gt tag id
    QString qtag_name(tag_name);
    UINT16 tagId = tgdescvec->tryToAlloc(qtag_name.stripWhiteSpace(),TagIntegerValue);

    // put it
    bool ok = trHeap->addTagValue(trackId, tagId,currentCycle,value);
    lastProcessedEventOk = lastProcessedEventOk && ok;
    DBLISTENER_DISPATCH_LISTENERS(listener->SetNodeTag(node_id,tag_name,value,level,list);)
}

void
DBListener::SetNodeTagString(UINT16 node_id, char tag_name [], char str [],
     UINT16 level, UINT32 list [])
{
    // resolve track id...
    INT32 trackId = trHeap->resolveTrackIdForNode(node_id,level,list);
    if (trackId<0)
    {
        DBLISTENER_DISPATCH_LISTENERS(listener->SetNodeTagString(node_id,tag_name,str,level,list);)
        return;
    }

    // gt tag id
    QString qtag_name(tag_name);
    UINT16 tagId = tgdescvec->tryToAlloc(qtag_name.stripWhiteSpace(),TagIntegerValue);

    // put it
    bool ok = trHeap->addTagValue(trackId, tagId,currentCycle,QString(str));
    lastProcessedEventOk = lastProcessedEventOk && ok;
    DBLISTENER_DISPATCH_LISTENERS(listener->SetNodeTagString(node_id,tag_name,str,level,list);)
}

void
DBListener::SetNodeTagSet(UINT16 node_id, char tag_name [], UINT16 n, UINT64 set [],
     UINT16 level, UINT32 list [])
{
    DBLISTENER_DISPATCH_LISTENERS(listener->SetNodeTagSet(node_id,tag_name,n,set,level,list);)
}

void
DBListener::Comment (UINT32 magic_num, char comment [])
{
    DBLISTENER_DISPATCH_LISTENERS(listener->Comment(magic_num,comment);)
}

void
DBListener::CommentBin (UINT16 magic_num, char comment [], UINT32 length)
{
    DBLISTENER_DISPATCH_LISTENERS(listener->CommentBin(magic_num, comment, length);)
}

void 
DBListener::SetNodeInputBandwidth(UINT16, UINT32)
{
}

void 
DBListener::SetNodeOutputBandwidth(UINT16, UINT32)
{
}

void 
DBListener::SetTagDescription(char tag[], char desc[])
{
    tgdescvec->setTagLongDesc(QString(tag),QString(desc));
}

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// ------------------------------------------------------------------
// ------------------------------------------------------------------
// ------------------------------------------------------------------

bool
DBListener::computeDeleteItem (UINT32 item_id)
{
    //printf("\tDBListener::ComputeDeleteItem called on id=%d\n",(int)item_id);fflush(stdout);

    // At this point I need to look for tags on the given item id
    // and put on ZDBase the item itself and all the related tags
    // toghether.

    // I need to check that a "new item" was launched for item_id
    LNewItemListNode* newItemNode = itemList->find((long)item_id);

    // If there is not such a "new item" command...
    if (newItemNode==NULL)
    {
        if (!itemWarningDumped(item_id) && itemWarnHash->count()<MAX_ITEMID_WARNS)
        {
            QString err ("DralDB Warning: DRAL incoherence detected, deleting unknown ITEMID: ");
            err = err + QString::number(item_id,10);
            myLogMgr->addLog(err);
            hasNonCriticalErros=true;
            addItemWarned(item_id);
        }
        // ignore the command!
        return false;
    }

    // ok, add the new item to the tag-heap
    INT32 itemEntryPoint = itHeap->newItemBlock(item_id);
    if (db_listener_debug_on)
    {
        printf("\t\tDBListener:: item %d added to ZDB, ok=%d\n", (int) item_id, (int) itemEntryPoint);
        fflush(stdout);
    }

    if(itemEntryPoint < 0)
    {
        lastProcessedEventOk = false;
        return false;
    }

    // now check for ALL the related tags:
    if (newItemNode->hasTags())
    {
        NewTagList* tgl = newItemNode->getMyTags();
        //QPtrListIterator<LSetTagListNode> tag_it(*tgl);
        LSetTagListNode* tgnode;
        bool tagok=true;
        int maxIdx = tgl->getMaxUsedIdx();
        int i=0;
        while ( tagok && (i<=maxIdx) )
        {
            tgnode = (*tgl)[i];
            if (tgnode->used)
            {
                ++i;
                continue;
            }

            if (tgnode->isMutable )
            { tagok = tagok && mutableTagDiscriminatorAdder(item_id,tgl,tgnode); }
            else
            { tagDiscriminatorAdder(item_id,tgnode); }

            ++i;
        }
        if (!tagok)
        {
            // tagDiscAdder produce an error... leave!
            lastProcessedEventOk = false;
            return false;
        }
    }
    UINT16 tagItemIdxInId = TagDescVector::getItemIdxIn_TagId();

    // now check all the enter node commands
    if (newItemNode->hasEnterNodes())
    {
        EENodeList* mlst = newItemNode->getMyEnterNodes();
        LEENodeListNode minode;
        EENodeList::iterator movit;
        for ( movit = mlst->begin(); movit != mlst->end(); ++movit )
        {
            minode = *movit;
            // TODO: Should it be the item_id instead of itemEntryPoint??
            bool ok = trHeap->addTagValue(minode.track_id,tagItemIdxInId,
                      minode.cycle,(UINT64)itemEntryPoint);

            lastProcessedEventOk = lastProcessedEventOk && ok;
        }
        if (!lastProcessedEventOk) return false;
    }
    // now check all the exit node commands
    if (newItemNode->hasExitNodes())
    {
        EENodeList* mlst = newItemNode->getMyExitNodes();
        LEENodeListNode minode;
        EENodeList::iterator movit;
        for ( movit = mlst->begin(); movit != mlst->end(); ++movit )
        {
            minode = *movit;

            bool ok = trHeap->addTagValue(minode.track_id,tagItemIdxInId,
                      minode.cycle,(UINT64)itemEntryPoint);

            lastProcessedEventOk = lastProcessedEventOk && ok;
        }
        if (!lastProcessedEventOk) return false;
    }
    // finally check all the moveitem commands
    if (newItemNode->hasMovs())
    {
        MoveItemList* mlst = newItemNode->getMyMovs();
        for (int i=0;i<=mlst->getMaxUsedIdx();i++)
        {
            /*
            printf (
                    "adding moveitem on edge-id=%d,pos=%d,cycle=%d,itemidx=%d,tgid=%d\n",
                    (int)((*mlst)[i].edgeid),(int)((*mlst)[i].pos),
                    (int)((*mlst)[i].cycle),itemEntryPoint,(int)_itemidxinid
                   );
            */
            bool ok = trHeap->addMoveItem((*mlst)[i].edgeid,itemEntryPoint,(*mlst)[i].cycle,(*mlst)[i].pos);
            lastProcessedEventOk = lastProcessedEventOk && ok;
            if (ok)
            {
                trHeap->decPendingCnt(
                        trHeap->resolveTrackIdEdge((*mlst)[i].edgeid) + (*mlst)[i].pos,
                        _itemidxinid,(*mlst)[i].cycle
                        );
            }
            else
            {
                QString err = "DralDB Internal Error: moveitem addition failed on edge ";
                err = err + QString::number((*mlst)[i].edgeid) + " position ";
                err = err + QString::number((*mlst)[i].pos) + " entry point ";
                err = err + QString::number(itemEntryPoint) + " cycle ";
                err = err + QString::number((*mlst)[i].cycle)+"\n";
                myLogMgr->addLog(err);
            }
        }
    }
    // delete the item_id from the "new" item list
    bool delete_ok = itemList->remove((long)item_id);
    Q_ASSERT(delete_ok);
    return (true);
}

bool
DBListener::mutableTagDiscriminatorAdder(INT32 itemid, NewTagList* list, LSetTagListNode* node)
{
    if (db_listener_debug_on)
    {
        printf ("DBListener::mutableTagDisc tagid=%d cycle=%d\n",
        (int)(node->tagid),(int)(node->cycle));fflush(stdout);
    }

    // mutable tags must be single value integer
    if (node->isSOV)
    {
        myLogMgr->addLog("DralDB Internal DBase error trying to add a new mutable tag.\n"
        "Sorry, mutable tags not supported on set of values type.");

        myLogMgr->addLog("Cycle "+QString(DBItoa::uint642str(node->cycle))+", ItemId " +
        QString::number(itemid) + ", TagId " + QString::number((ulong)(node->tagid)) +
        ", TagName " + tgdescvec->getTagDescription(node->tagid) );

        lastProcessedEventOk = false;
        return (false);
    }

    if (node->isString)
    {
        INT32 stridx = strtbl->addString(node->str);
        node->data.value = (UINT64)stridx;
    }

    // get the first entry
    //if (db_listener_debug_on) printf(">> first value (%d) on cycle=%d\n",(int)(node->value),(int)(node->cycle));fflush(stdout);    // mark current tag as used
    itHeap->newTag(itemid, node->tagid, node->data.value, (UINT32)(node->cycle));

    node->used = 1;

    // mutable tags must be put in cycle order and at once
    // now check for other values for the same tag..
    LSetTagListNode* it_node;
    bool ok=true;
    int maxIdx = list->getMaxUsedIdx();
    for (int i=0;i<=maxIdx;i++)
    {
        it_node = (*list)[i];
        if (it_node->used) { continue; }
        if (it_node==node) { continue; }

        if (it_node->tagid == node->tagid)
        {
            if (it_node->isString)
            {
                INT32 stridx = strtbl->addString(it_node->str);
                it_node->data.value = (UINT64)stridx;
            }
            if (db_listener_debug_on)
            {
                printf(">> adding another value (%d) on cycle=%d\n",(int)(it_node->data.value),(int)(it_node->cycle));fflush(stdout);
            }
            itHeap->newTag(itemid, it_node->tagid, it_node->data.value,(UINT32)(it_node->cycle));
            it_node->used = 1;
        }
    }
    return true;
}

void
DBListener::reset()
{
    // statistics
    DBListener::_itemCnt=0;
    DBListener::_dumyCnt=0;
    DBListener::_accLiveCycles=0;
    DBListener::_accMoveItems=0;
    DBListener::_accTags=0;
    DBListener::_accItems=0;
    DBListener::_cyclesCnt=0;

    currentCycle=-1;
    lastProcessedEventOk=true;
    hasNonCriticalErros=false;
    eoSimulation = false;
    currentTraceVersion=-1;
    processingDralHeader=true;
    doTrackItemTags = true;
    firstEffectiveCycle=-1;
    firstCycle = true;

    if (itemList!=NULL)  itemList->clear();
    if (trackWarnHash!=NULL) trackWarnHash->clear();
    if (itemWarnHash!=NULL) itemWarnHash->clear();

    // clear track id on edges
    bzero((char*)edgeTrackIdVector,sizeof(edgeTrackIdVector));

    trackededges.clear();
}

void
DBListener::trackItemTags(bool value)
{doTrackItemTags=value;}

void
DBListener::flush()
{
    if (eoSimulation) return;
    Do_EndSimulation();
}

void
DBListener::forceMaxIfiPurge()
{
	int numItems = itemList->count();
	int targetItems = (int) ( (double)(conf->getMaxIFI()) * 0.90 );
	int numRed  = numItems - targetItems;
	Q_ASSERT(numRed>0);

    //printf ("DBListener::forceAfePurge called on numItems=%d, maxIfi=%d, target=%d, red=%d\n",numItems,conf->getMaxIFI(),targetItems,numRed);fflush(stdout);
	AuxItemList auxList;

    // itarate throught the live guys:
    QIntDictIterator<LNewItemListNode> it(*itemList);
    for ( ; it.current(); ++it )
    { auxList.append(new AuxItemListNode(it.current()->cycle,it.current()->item_id)); }

	// sort "live" items by cycle
    auxList.sort();

    // iterate through the first numRed items
    int cnt = 0;
    AuxItemListNode* current = auxList.first();
    while ( (current!=NULL) && (cnt<numRed) )
    {
    	computeDeleteItem(current->item_id);
        // propagate the death to other clients
        DBLISTENER_DISPATCH_LISTENERS(listener->DeleteItem(current->item_id);)

        cnt++;
        current = auxList.next();
    }
    Q_ASSERT(cnt==numRed);
    if (cnt>0)
    {
        myLogMgr->addLog("DralDB Warning: reaching Maximum Infligh items leads to elimination of "+QString::number(cnt)+" items");
    }
}

void
DBListener::attachDralListener(DRAL_LISTENER_OLD object)
{
    externClients->append(object);
}

/*
void
DBListener::dumpItem(INT32 cidx)
{
    printf ("ItemId: %d at idx %d\n",itHeap->getItemId(cidx),cidx);
    bool eol = itHeap->isLast(cidx);
    while (!eol)
    {
        ++cidx;
        eol = itHeap->isLast(cidx);
        printf("TagId = %d (%s),Value=%llu, PostCycle=%d, SOV=%d, Mutable=%d, Idx=%d\n",
               itHeap->getTagId(cidx),tgdescvec->getTagDescription(itHeap->getTagId(cidx)).latin1(),
               itHeap->getValue(cidx),
               itHeap->getCycle(cidx),(int)(itHeap->isSOV(cidx)),
               (int)(itHeap->isMutable(cidx)),
               cidx
               );

    }
}
*/

void
DBListener::setTrackedEdges (INT32 value)
{
    //printf ("DBListener::setTrackedEdges with value=%d, tec=%d\n",value,trackededges.count());

    if (lastUsedPosVector!=NULL)
    {
        delete [] lastUsedPosVector;
    }
    numTrackedEdges = QMAX(value,1);
    lastUsedPosVector = new UINT8[numTrackedEdges];

    // check coherence
    Q_ASSERT ((INT32)(trackededges.count()) == value);
    bzero((char*)lastUsedPosVectorIndirection,sizeof(lastUsedPosVectorIndirection));
    int current =0;
    for (int i=0;i<value;i++)
    {
        lastUsedPosVectorIndirection[trackededges[i]]=i;
        //printf(">>>lastUsedPosVectorIndirection[%d]=%d\n",trackededges[i],i);
    }
}

void
DBListener::addTrackedEdges (UINT16 edgeid)
{
    // just put it on a tmp list
    trackededges.append(edgeid);
}

// -------------------------------------------------------------------
// AMemObj methods
// -------------------------------------------------------------------

INT64
DBListener::getObjSize() const
{
    INT64 result=sizeof(DBListener);
    return result;
}

QString
DBListener::getUsageDescription() const
{
    return QString("");
}

QString
DBListener::getStats() const
{
    QString result = "";

    double dummyProp = ((double)DBListener::_dumyCnt/(double)DBListener::_itemCnt)*100.0;
    double movsProps = ((double)DBListener::_accMoveItems/(double)DBListener::_itemCnt);
    double tagsProps = ((double)DBListener::_accTags/(double)DBListener::_itemCnt);
    double lifeMean  = ((double)DBListener::_accLiveCycles/(double)DBListener::_itemCnt);
    double liveMean  = ((double)DBListener::_accItems/(double)DBListener::_cyclesCnt);

    result +="\tProcessed items:\t\t\t"+QString::number((long)(DBListener::_itemCnt))+"\n";
    result +="\tUseless items:\t\t\t"+QString::number(dummyProp)+"%\n";
    result +="\tMoveitems per item:\t\t"+QString::number(movsProps)+"\n";
    result +="\tTags per item:\t\t\t"+QString::number(tagsProps)+"\n";
    result +="\tNumber of cycles items are live:\t"+QString::number(lifeMean)+"\n";
    result +="\tNumber of inflight (live) items:\t"+QString::number(liveMean)+"\n";
    return (result);
}


