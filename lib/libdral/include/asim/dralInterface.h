/**************************************************************************
 *Copyright (C) 2004-2006 Intel Corporation
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
 * @file dralInterface.h
 * @author Julio Gago
 * @brief dral "low-level-but-pretty" interface
 */

/*
 * The purpose of the classes defined here is to provide a somewhat clean and
 * compact "low level" access to DRAL features (nodes, items, edges, tags, and
 * their relationship). Here "low level" means that DRAL is not accessed
 * through the high-level, "hidden" and structure-based interface automatically
 * created when ASIM modules and ports are used.
 *
 * It is safe to mix both interfaces to DRAL (id's management is automatic and
 * duplication will not happen).
 */

#ifndef __dralInterface_h
#define __dralInterface_h

#include <sstream>
#include <unistd.h>
#include <map>

#include "asim/dralServer.h"

/*
 * A single "default" DRAL server, used when the user
 * does not explicitly create and provide one (or more). This is almost
 * always true.
 */
extern DRAL_SERVER_CLASS defaultDralServer;

/*
 * Several operations forwarded to default DRAL server
 */
void DralStartActivity(); 
void DralTurnOn(void);
void DralCycle(UINT64 n);
void DralTurnOff(void);

/*
 * The following name/value "pair" classes (one per basic datatype) are not
 * designed to be used directlty by a DRAL user, but as a convenient
 * implementation of the TAG abstraction and 'compact' value assignation using
 * the () operator. This is defined later in this file.
 */

/*
 * A name/value TAG pair for integers
 */
class DRAL_uTAG 
{ 
  public:
    const char *name; 
    UINT64 value; 

    inline DRAL_uTAG(const char tag_name[], UINT64 tag_value) { name = tag_name; value = tag_value; }
};

/*
 * A name/value TAG pair for chars
 */
class DRAL_cTAG 
{ 
  public:
    const char *name; 
    char value; 

    inline DRAL_cTAG(const char tag_name[], char tag_value) { name = tag_name; value = tag_value; }
};

/*
 * A name/value TAG pair for strings
 */
class DRAL_sTAG
{
  public:
    const char *name;
    const char *value;

    inline DRAL_sTAG(const char tag_name[], const char tag_value[]) { name = tag_name; value = tag_value; }
};

/*
 * Syntax sugar to create name/value TAG pairs using a single and compact
 * function name, but without the use of a DRAL_TAG objects, which is in any
 * case recommended :-).
 */
DRAL_uTAG dTag(const char tag_name[], UINT64 value);
DRAL_uTAG dTag(const char tag_name[], INT64 value);
DRAL_uTAG dTag(const char tag_name[], UINT32 value);
DRAL_uTAG dTag(const char tag_name[], INT32 value);
DRAL_cTAG dTag(const char tag_name[], char value);
DRAL_sTAG dTag(const char tag_name[], const char value[]);

/* 
 * A 'TAG' class to ease the creation and consistent reference to DRAL tags.
 * Also allows a very 'compact' specification of TAG/value pairs by overloading
 * the operator () with different datatypes.
 * 
 * TAGs are created with:               DRAL_TAG mytag("MY_TAG", "this tag represents bla bla bla");
 * TAG/value pairs instantiated with:   mytag(443) mytag("string value")
 *
 * See below for interaction with ITEMS and NODES.
 */
class DRAL_TAG 
{ 
  private: 
    const char *myName;


  public:
    inline DRAL_TAG(const char name[], const char desc[], DRAL_SERVER_CLASS &server = defaultDralServer)
    { 
        /*
         * note that we only need to keep tag's name, no need to keep either server
	 * or description
	 */
        myName = name;
	server.SetTagDescription(name, desc);
    }

    inline DRAL_uTAG operator()(UINT64 value)       { return *(new DRAL_uTAG(myName, value)); }
    inline DRAL_uTAG operator()(INT64 value)        { return *(new DRAL_uTAG(myName, (UINT64) value)); }
    inline DRAL_uTAG operator()(UINT32 value)       { return *(new DRAL_uTAG(myName, (UINT64) value)); }
    inline DRAL_uTAG operator()(INT32 value)        { return *(new DRAL_uTAG(myName, (UINT64) value)); }
    inline DRAL_cTAG operator()(char value)         { return *(new DRAL_cTAG(myName, value)); }
    inline DRAL_sTAG operator()(const char value[]) { return *(new DRAL_sTAG(myName, value)); }
};

/*
 * An 'ITEM' class designed to allow compact and convenient creation of items
 * and assignation of tag/value pairs to items to finally enter into nodes or
 * edges. 'itemId' management is automatic and compatible with high-level DRAL
 * interface (both uses can be mixed).
 *
 * ITEM creation:    DRAL_ITEM i;
 * TAG assignation:  i << MY_TAG("my_value") << MY_OTHER_TAG("my_other_value");
 *
 */

class DRAL_ITEM
{
  private:
    DRAL_SERVER myServer;
    UINT32 myItemId;
    static UINT32 current_id;
  
  public:
    inline DRAL_ITEM(DRAL_SERVER_CLASS &server = defaultDralServer)
    {
        myServer = &server;
        myItemId = myServer->NewItem();
    }

    inline DRAL_ITEM &operator<<(DRAL_uTAG t)
    {
        myServer->SetItemTag(myItemId, t.name, t.value);
        return *this;
    }
    
    inline DRAL_ITEM &operator<<(DRAL_cTAG t)
    {
        myServer->SetItemTag(myItemId, t.name, t.value);
        return *this;
    }

    inline DRAL_ITEM &operator<<(DRAL_sTAG t)
    {
        myServer->SetItemTag(myItemId, t.name, t.value);
        return *this;
    }
    
    inline UINT32 GetItemId() { return myItemId; }
};

/*
 * A 'node' class designed to ease DRAL node creation and item insertion with
 * automatic management of 'nodeId's and compatible with high-level DRAL
 * interface (both uses can be mixed). Autoflush enternode/exitnode mode used
 * by default. Operators << and () overloaded for compact interaction with
 * items and node tags:
 *
 * NODE creation:              DRAL_NODE mynode;
 * NODE initialization:        mynode.Init("MAF", 3, 16);
 * both together:              DRAL_NODE mynode("MAF", 3, 16);
 * Assignation of NODE tag's:  mynode << MY_TAG("my_value");
 * Insertion of items:         mynode(12) << myitem;
 *
 * If you're wondering WHY initialization is separeted from creation, here is
 * the answer:
 *
 * It is usual in simulators and models to have several "pieces" with several
 * creation/initialization/run phases, and this tries to mimic this to ease
 * integration. It is usual that the 'instance' number of a node is not known
 * until a later phase of initialization (after creation), so forcing
 * initialization at creation time (class constructor) may force the
 * utilization of pointers, which is not always convenient. Specifically,
 * operators are overloaded for objects, not for pointers to objects, as C++
 * forbids that for some operators.
 */
class DRAL_NODE_POS;
class DRAL_NODE
{
  private:
    DRAL_SERVER myServer;
    UINT32 myNodeId;
    static UINT32 current_id;

  public:
    inline DRAL_NODE()
    {
        myServer = NULL;
    }

    inline DRAL_NODE(const char name[], UINT16 instance, UINT32 size, DRAL_SERVER_CLASS &server = defaultDralServer)
    {
        myServer = NULL;
	Init(name, instance, size, server);
    }
  
    inline void Init(const char name[], UINT16 instance, UINT32 size, DRAL_SERVER_CLASS &server = defaultDralServer)
    {
        DRAL_ASSERT(myServer == NULL, "DRAL error: trying to initialize a node twice");
        myServer = &server;
	myNodeId = myServer->NewNode(name, 0, instance, true, true, false);
        myServer->SetNodeLayout(myNodeId, size);
    }

    inline DRAL_SERVER GetServer()
    {
        DRAL_ASSERT(myServer, "DRAL error: trying to use node before initialization");
        return myServer; 
    }
    
    inline UINT32 GetNodeId()
    {
        DRAL_ASSERT(myServer, "DRAL error: trying to use node before initialization");
        return myNodeId; 
    }

    inline DRAL_NODE_POS & operator()(UINT32 i);
    
    inline void EnterNode(UINT32 itemId, UINT32 position) 
    { 
        DRAL_ASSERT(myServer, "DRAL error: trying to use node before initialization");
        myServer->EnterNode(myNodeId, itemId, position);
    }

    inline DRAL_NODE &operator<<(DRAL_uTAG t)
    {
        DRAL_ASSERT(myServer, "DRAL error: trying to use node before initialization");
        myServer->SetNodeTag(myNodeId, t.name, t.value);
        return *this;
    }
    
    inline DRAL_NODE &operator<<(DRAL_cTAG t)
    {
        DRAL_ASSERT(myServer, "DRAL error: trying to use node before initialization");
        myServer->SetNodeTag(myNodeId, t.name, t.value);
        return *this;
    }
    
    inline DRAL_NODE &operator<<(DRAL_sTAG t)
    {
        DRAL_ASSERT(myServer, "DRAL error: trying to use node before initialization");
        myServer->SetNodeTag(myNodeId, t.name, t.value);
        return *this;
    }
};

/*
 * This class is not designed for the DRAL user to directly use it, but as a
 * convenient implementation for insertion of items into node positions or
 * slots.
 * 
 * The operator () of a node returns a "node position" in which DRAL items
 * can be inserted using the << operator
 *
 * MAF(6) << my_item;
 */
class DRAL_NODE_POS
{
  private:
    DRAL_NODE *myNode;
    UINT32 myPosition;

  public:
    inline DRAL_NODE_POS(DRAL_NODE *node, UINT32 position) { myNode = node; myPosition = position; } 
    inline DRAL_ITEM &operator<<(DRAL_ITEM &i) { myNode->EnterNode(i.GetItemId(), myPosition); return i; }
};

inline DRAL_NODE_POS & DRAL_NODE::operator()(UINT32 i)
{
    DRAL_ASSERT(myServer, "DRAL error: trying to use node before initialization");
    return *(new DRAL_NODE_POS(this, i));
}

/*
 * An 'edge' class designed to ease DRAL edge creation and item "movement" with
 * automatic management of 'edgeId's. Operator << overloaded for compact interaction with
 * items:
 *
 * EDGE creation:              DRAL_EDGE myedge;
 * EDGE initialization:        myedge.Init("ARB2PIPE", ARB, PIPE, 2, 3);
 * both together:              DRAL_EDGE myedge("ARB2PIPE", ARB, PIPE, 2, 3); 
 * Movement of items:          myedge << myitem [ << mytag("myvalue") ];
 *
 * If you're wondering WHY initialization is separeted from creation, 
 * the answer is in the comments for the 'node' class above :-).
 */
class DRAL_EDGE
{
  private:
    DRAL_SERVER myServer;
    UINT32 myEdgeId;
    static UINT32 current_id;

  public:
    inline DRAL_EDGE()
    {
        myServer = NULL;
    }
  
    inline DRAL_EDGE(const char name[], DRAL_NODE src, DRAL_NODE dst, UINT32 bandwidth,
                     UINT32 latency, DRAL_SERVER_CLASS &server = defaultDralServer)
    {
        myServer = NULL;
        Init(name, src, dst, bandwidth, latency, server);
    }

    inline void Init(const char name[], DRAL_NODE src, DRAL_NODE dst, UINT32 bandwidth,
                     UINT32 latency, DRAL_SERVER_CLASS &server = defaultDralServer)
    {
        DRAL_ASSERT(myServer == NULL, "DRAL error: trying to initialize an edge twice");
        myServer = &server;
        myEdgeId = myServer->NewEdge(src.GetNodeId(), dst.GetNodeId(), bandwidth, latency, name, true);
    }

    inline DRAL_ITEM & operator<<(DRAL_ITEM &i) 
    {
        DRAL_ASSERT(myServer, "DRAL error: trying to use edge before initialization");
        myServer->MoveItem(myEdgeId, i.GetItemId());
	return i; 
    }
}; 

#endif /* __dralInterface_h */
