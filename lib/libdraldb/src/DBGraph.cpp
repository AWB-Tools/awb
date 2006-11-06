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
  * @file DBGraph.cpp
  */

#include "asim/DBGraph.h"

#include <qvaluelist.h>
#include <qregexp.h>

/**
 * The instance is NULL at the beginning.
 */
DBGraph* DBGraph::_myInstance=NULL;

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer.
 *
 * @return the instance of the class.
 */
DBGraph*
DBGraph::getInstance()
{
    if (_myInstance==NULL)
	{
        _myInstance = new DBGraph();
	}
    return _myInstance;
}

/**
 * Destroys the unique instance of the class. The instance is
 * destroyed if it was previously created.
 *
 * @return void.
 */
void
DBGraph::destroy()
{
    if (_myInstance!=NULL)
	{
		delete _myInstance;
	}
}

/**
 * Creator of this class. Just calls the init function.
 *
 * @return new object.
 */
DBGraph::DBGraph()
{
	init();
}

/**
 * Destructor of this class. Just calls the freeAll function.
 *
 * @return destroys the object.
 */
DBGraph::~DBGraph()
{
	freeAll();
}

/**
 * Resets the class. First all the previous information
 * is cleared calling freeAll and then the new resources are
 * allocated using init.
 *
 * @return void.
 */
void
DBGraph::reset()
{
	freeAll();
	init();
}

/**
 * Initializes the class. Allocates the lists of nodes and edges
 * and gets the log manager instance. The lists are set to auto
 * delete to avoid the manual delete of all the nodes and edges
 * of the lists.
 *
 * @return void.
 */
void
DBGraph::init()
{
    // Allocates the lists.
    dbgnList       = new DBGNList(DEFAULT_DBGN_SIZE);
    dbgnListByName = new DBGNListByName(DEFAULT_DBGN_SIZE);
    dbgeList       = new DBGEList(DEFAULT_DBGE_SIZE);

    Q_ASSERT(dbgnList!=NULL);
    Q_ASSERT(dbgnListByName!=NULL);
    Q_ASSERT(dbgeList!=NULL);

    // Sets true autodelete the lists.
    dbgnList->setAutoDelete(true);
    dbgeList->setAutoDelete(true);
    // Nodes objects are the same than in dbgeList!
    dbgnListByName->setAutoDelete(false);

    myLogMgr = LogMgr::getInstance();
}

/**
 * Frees all the components of this class. When the lists are
 * deleted their components are deleted too.
 *
 * @return void.
 */
void
DBGraph::freeAll()
{
    delete dbgeList;
    delete dbgnListByName;
    delete dbgnList;
}

/**
 * Gets the edge that has the name name and the name of its
 * source node is fromstr and the name of its destination node
 * is tostr.
 *
 * @return the requested edge.
 */
DBGraphEdge*
DBGraph::findEdgeByNameFromTo(QString name,QString fromstr,QString tostr)
{
    bool fnd = false;
    QIntDictIterator<DBGraphEdge> it(*dbgeList);
    DBGraphEdge* result = it.current();

    while (!fnd && (result!=NULL) )
    {
        fnd = ( ( name      == result->getName() ) &&
                ( fromstr   == result->getSourceNodeName()) &&
                ( tostr     == result->getDestinationNodeName())
              );
        if (!fnd)
        {
            ++it;
            result = it.current();
        }
    }
    return (result);
}

/**
 * Adds a new node to the DRAL graph. A new node is created and is
 * added in the two node lists. If the node_id or the name are
 * repeated the node is not added.
 *
 * @return true if the node can be added false otherwise.
 */
bool
DBGraph::addNode(QString name, UINT16 node_id, UINT16 parent_id, UINT16 instance)
{
    // compute internal name
    QString nodeName = name+"{"+QString::number((int)instance)+"}";

    // 1) check that the ID is not repeated
    /** @todo we must relay on "long" qt type, review this! */
    DBGraphNode* dgnode = dbgnList->find((long)node_id);
    if (dgnode!=NULL)
    {
        myLogMgr->addLog ("DralDB: Repeated DRAL node detected (ID "+QString::number((int)node_id)+
        ") in DRAL file, please report this problem.");
        return (false);
    }

    // 2) check if the name is repeated
    dgnode = dbgnListByName->find(nodeName);
    if (dgnode!=NULL)
    {
        myLogMgr->addLog ("DralDB: Repeated DRAL node detected (Name "+name+
        ", Instance="+QString::number((int)instance)+") in DRAL file, please report this problem.");
        return (false);
    }

    // Creates the node and inserts it in the name and node_id lists.
    dgnode = new DBGraphNode(nodeName,node_id,parent_id,instance);Q_ASSERT(dgnode!=NULL);
    Q_ASSERT(dgnode!=NULL);
    dbgnList->insert((long)node_id, dgnode);
    dbgnListByName->insert(nodeName, dgnode);

    return (true);
}

/**
 * Adds a new edge to the DRAL graph. A new edge is created and is
 * added in the edge list. If the edge_id is repeated or the source
 * or destination node doesn't exist the edge is not added.
 *
 * @return true if the edge can be added false otherwise.
 */
bool 
DBGraph::addEdge (UINT16 source_node,UINT16 destination_node,
                          UINT16 edge_id, UINT32 bandwidth,UINT32 latency,
                          QString name)
{
    // check that the ID is not repeated
    /** @todo we must relay on "long" qt type, review this! */
    DBGraphEdge* dgedge = dbgeList->find((long)edge_id);
    if (dgedge!=NULL)
    {
        QString err = "DralDB: Repeated DRAL edge detected (ID "+QString::number((int)edge_id)+
        ") in DRAL file, please report this problem.";
        myLogMgr->addLog (err);
        return (false);
    }

    // Check that both source and destination nodes exists.
    /** @todo we must relay on "long" qt type, review this! */
    DBGraphNode* srcNode = dbgnList->find((long)source_node);
    DBGraphNode* dstNode = dbgnList->find((long)destination_node);
    if ( (srcNode==NULL)||(dstNode==NULL) )
    {
        myLogMgr->addLog ("DralDB: Adding DRAL edge over unknown nodes(from ID "+
        QString::number((int)source_node)+" to ID "+QString::number((int)destination_node)+
        "), please report this problem.");
        return (false);
    }

    // Creates the edge and inserts it in the node_id list.
    dgedge = new DBGraphEdge(srcNode,dstNode,edge_id,bandwidth,latency,name);
    Q_ASSERT(dgedge!=NULL);
    dbgeList->insert((long)edge_id, dgedge);

    return (true);
}

QString
DBGraph::normalizeNodeName(QString name)
{
    QString result=name;
    int pos = name.find(QChar('{'));
    if (pos<0)
    {
        result += "{0}";
    }
    return (result);
}

QString
DBGraph::slotedNodeName(QString name, UINT16 level, UINT32 list [])
{
    QString nname = DBGraph::normalizeNodeName(name);
    if(level == 0)
    {
        return nname;
    }
    QString slot = "{";
    for(int i=0;i<level;i++)
    {
        slot += QString::number(list[i]);
        if (i<(level-1)) slot += ",";
    }
    slot += "}";
    return nname+slot;
}

bool
DBGraph::decodeNodeSlot(QString strnodeslot,QString* rnodename,NodeSlot* rnslot)
{
    //printf (">>decodeNodeSlot called with=%s\n",strnodeslot.latin1());fflush(stdout);
    // separate node name from slot specification

    QString nodename=QString::null;
    QString slotspec=QString::null;
    int pos = strnodeslot.find(QChar(';'));
    if (pos<0)
    {
        nodename=strnodeslot;
    }
    else
    {
        nodename = strnodeslot.left(pos);
        slotspec = strnodeslot.right(strnodeslot.length()-pos-1);
    }
    // set the result node name
    *rnodename = DBGraph::normalizeNodeName(nodename);

    // if slot spec -> root & we are done
    if (slotspec==QString::null)
    {
        rnslot->specDimensions=0;
        rnslot->dimVec=NULL;
        return true;
    }

    // decode the node slot from x,y,z,d to a NodeSlot struct
    QRegExp rx( "\\d+" );
    QValueList<UINT32> tmplist;
    int count = 0;
    pos = 0;
    while ( pos >= 0 )
    {
        pos = rx.search( slotspec, pos );
        if (pos>=0)
        {
            QString dimspec = slotspec.mid(pos,rx.matchedLength());
            bool pok;
            uint dim = dimspec.toUInt (&pok);
            if (!pok) return false;
            tmplist.append((UINT32) dim);
            count++;
        }
        pos += rx.matchedLength();
    }

    if (count==0) // nothing after ;
    {
        rnslot->specDimensions=0;
        rnslot->dimVec=NULL;
        return true;
    }
    else
    {
        UINT32* vec = new UINT32[count];
        QValueList<UINT32>::Iterator it = tmplist.begin();
        int idx=0;
        while (it != tmplist.end())
        {
            vec[idx]= *it;
            ++it;
            ++idx;
        }
        rnslot->specDimensions=count;
        rnslot->dimVec=vec;
        return true;
    }
}

/**
 * Sets the layout to the node with id node_id.
 *
 * @return true if the node exists. False otherwise.
 */
bool
DBGraph::setNodeLayout(UINT16 node_id, UINT16 dimensions, UINT32 capacities [])
{
    // look for the node
    DBGraphNode* node = dbgnList->find((long)node_id);
    if (node==NULL)
    {
        myLogMgr->addLog("DralDB: Setting layout over an unknown node, (ID "+
        QString::number((int)node_id)+") on the DRAL file, please report this problem.");
        return false;
    }
    node->setLayout(dimensions, capacities);
    return (true);
}

/**
 * Creates a description of the graph using the contents of the
 * nodes and edges lists.
 *
 * @return the description of the graph.
 */
QString
DBGraph::getGraphDescription()
{
	QString result = "DRAL-Graph has ";
	result += QString::number(dbgnList->count());
	result += " nodes and ";
	result += QString::number(dbgeList->count());
	result += " edges\n\n";

	result += "Node List:\n\n";
    QIntDictIterator<DBGraphNode> nodeit( *dbgnList );
    for ( ; nodeit.current(); ++nodeit )
    {
        DBGraphNode* node = nodeit.current();
		result += "Node Id=" + QString::number((int)(node->getNodeId()));
		result += ", name="+node->getName(); 
		result += ", capacity="+node->getCapacity(); 
		result += "\n";
    }

	result += "Edge List:\n\n";
    QIntDictIterator<DBGraphEdge> edgeit( *dbgeList );
    for ( ; edgeit.current(); ++edgeit )
    {
        DBGraphEdge* node = edgeit.current();
		result += "Edge Id="+QString::number((int)(node->getEdgeId()));
		result += " name=" + node->getName();
		result += " from " + node->getSourceNodeName();
		result += " to " + node->getDestinationNodeName();
		result += " bandwidth=" + QString::number((int)(node->getBandwidth())); 
		result += " latency=" + QString::number((int)(node->getLatency())); 
		result += "\n";
    }
	return result;
}

/*
bool
DBGraph::decodeEdgeSlot(QString stredgeslot,QString* ename,QString* from, QString *to, INT32 *pos)
{
    //printf (">>decodeEdgeSlot called with=%s\n",stredgeslot.latin1());fflush(stdout);

    // get the edge name
    int pos1 = stredgeslot.find(QChar(';'));
    if (pos1<0) return false;
    *ename = stredgeslot.left((uint)pos1);
    //printf ("ename=%s\n",(*ename).latin1());

    // get the from/to node names
    int pos2 = stredgeslot.find(QChar(';'),pos1+1);
    if (pos2<0) return false;
    int pos3 = stredgeslot.find(QChar(';'),pos2+1);
    if (pos3<0) return false;

    *from = stredgeslot.mid(pos1+1,pos2-pos1-1);
    *to   = stredgeslot.mid(pos2+1,pos3-pos2-1);

    //printf ("from=%s\n",(*from).latin1());
    //printf ("to=%s\n",(*to).latin1());

    QString spos = stredgeslot.right((uint)(stredgeslot.length()-pos3-1));
    //printf ("spos=%s\n",spos.latin1());

    bool pok;
    int rpos = spos.toInt(&pok);
    if (!pok) return false;
    *pos = rpos;
    return true;
}

QString
DBGraph::slotedEdgeName(DBGraphEdge* edge, INT32 pos)
{
    QString ename = edge->getName();
    QString sname = edge->getSourceNodeName();
    QString dname = edge->getDestinationNodeName();
    return ename+";"+sname+";"+dname+";"+QString::number(pos);
}
*/


