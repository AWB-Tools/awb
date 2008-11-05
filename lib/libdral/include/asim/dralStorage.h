/**************************************************************************
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

#ifndef __dralStorage_h
#define __dralStorage_h


#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <list>
using namespace std;

#include "asim/dralServerImplementation.h"

class DRAL_COMMAND_STORAGE_CLASS
{
  public:
    virtual void Notify (DRAL_SERVER_IMPLEMENTATION implementation)=0;
    virtual ~DRAL_COMMAND_STORAGE_CLASS(void) {};
};
typedef DRAL_COMMAND_STORAGE_CLASS * DRAL_COMMAND_STORAGE;

class DRAL_NEWNODE_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:

    UINT16 nodeId;
    char * nodeName;
    UINT16 parentId;
    UINT16 inst;

  public:

    DRAL_NEWNODE_STORAGE_CLASS (
        UINT16 node_id, const char name[], UINT16 parent_id, UINT16 instance)
    {
        nodeId = node_id;
        nodeName = strdup (name);
        assert(nodeName);
        parentId=parent_id;
        inst=instance;
    }
    
    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->NewNode(
            nodeId,nodeName,strlen(nodeName)+1,parentId,inst);
    }
    
    const char * GetNodeName(void)
    {
        return nodeName;
    }

    UINT16 GetInstance(void)
    {
        return inst;
    }

    ~DRAL_NEWNODE_STORAGE_CLASS(void)
    {
        free((char *)nodeName);
    }
};
typedef DRAL_NEWNODE_STORAGE_CLASS * DRAL_NEWNODE_STORAGE;

class DRAL_STORAGE_CLASS
{
  public:

        //hack for GetInstance
    void Store (DRAL_NEWNODE_STORAGE command, bool bothlists);

    void Store (DRAL_COMMAND_STORAGE command, bool bothlists);
    void DumpPartialList (void);
    void DumpAllCommandsList (void);
    void ResetPartialList(void);
    UINT16 GetInstance (const char node_name []);
    void ChangeImplementation(DRAL_SERVER_IMPLEMENTATION impl);
    DRAL_STORAGE_CLASS(DRAL_SERVER_IMPLEMENTATION impl);
    ~DRAL_STORAGE_CLASS(void);

  private:
  
    DRAL_SERVER_IMPLEMENTATION implementation;
    list<DRAL_COMMAND_STORAGE> allCommandsList;
    list<DRAL_COMMAND_STORAGE> partialCommandList;
    list<DRAL_NEWNODE_STORAGE> numInstanceList;

};
typedef DRAL_STORAGE_CLASS * DRAL_STORAGE;


class DRAL_NEWEDGE_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:

    UINT16 edgeId;
    UINT16 sourceNode;
    UINT16 destinationNode;
    UINT32 bw;
    UINT32 lat;
    char * edgeName;

  public:

    DRAL_NEWEDGE_STORAGE_CLASS (
        UINT16 edge_id, UINT16 source_node, UINT16 destination_node,
        UINT32 bandwidth, UINT32 latency, const char name[])
    {
        edgeId = edge_id;
        sourceNode = source_node;
        destinationNode = destination_node;
        bw = bandwidth;
        lat = latency;
        edgeName = strdup (name);
        assert(edgeName);
    }
    
    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->NewEdge(
            edgeId,sourceNode,destinationNode,bw,
            lat,edgeName,strlen(edgeName)+1);
    }

    ~DRAL_NEWEDGE_STORAGE_CLASS(void)
    {
        free(edgeName);
    }
};
typedef DRAL_NEWEDGE_STORAGE_CLASS * DRAL_NEWEDGE_STORAGE;

class DRAL_SETNODELAYOUT_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:

    UINT16 nodeId;
    UINT16 dim;
    UINT32 * cap;

  public:

    DRAL_SETNODELAYOUT_STORAGE_CLASS (
        UINT16 node_id,  UINT16 dimensions, UINT32 capacity [])
    {
        nodeId=node_id;
        dim=dimensions;
        cap = new UINT32 [dim];
        assert(cap);
        memcpy(cap,capacity,dim*sizeof(UINT32));
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetNodeLayout(nodeId,dim,cap);
    }

    ~DRAL_SETNODELAYOUT_STORAGE_CLASS(void)
    {
        delete [] cap;
    }
};
typedef DRAL_SETNODELAYOUT_STORAGE_CLASS * DRAL_SETNODELAYOUT_STORAGE;

class DRAL_SETNODETAGSET_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 nodeId;
    char * tagName;
    UINT16 numElems;
    UINT64 * setOfValues;
    UINT16 lev;
    UINT32 * list_;

  public:

    DRAL_SETNODETAGSET_STORAGE_CLASS (
        UINT16 node_id, const char tag_name [], UINT16 n, UINT64 set [],
        UINT16 level, UINT32 lst [])
    {
        nodeId = node_id;
        tagName = strdup(tag_name);
        assert(tagName);
        numElems=n;
        setOfValues = new UINT64 [n];
        memcpy(setOfValues,set,n*sizeof(*set));
        assert(setOfValues);
        lev=level;
        if (lev)
        {
            list_ = new UINT32 [lev];
            assert(list_);
            memcpy(list_,lst,lev*sizeof(*list_));
        }
        else
        {
            list_ = NULL;
        }
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetNodeTag(
            nodeId,tagName,strlen(tagName)+1,numElems,setOfValues,lev,list_);
    }

    ~DRAL_SETNODETAGSET_STORAGE_CLASS(void)
    {
        if (list_ != NULL)
        {
            delete [] list_;
        }
        delete [] setOfValues;
        free(tagName);
    }
};
typedef DRAL_SETNODETAGSET_STORAGE_CLASS * DRAL_SETNODETAGSET_STORAGE;

class DRAL_SETNODETAGSTRING_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 nodeId;
    char * tagName;
    char * string;
    UINT16 lev;
    UINT32 * list_;

  public:

    DRAL_SETNODETAGSTRING_STORAGE_CLASS (
        UINT16 node_id, const char tag_name [], const char str [],
        UINT16 level, UINT32 lst [])
    {
        nodeId = node_id;
        tagName = strdup(tag_name);
        assert(tagName);
        string = strdup(str);
        assert(string);
        lev=level;
        if (lev)
        {
            list_ = new UINT32 [lev];
            assert(list_);
            memcpy(list_,lst,lev*sizeof(*list_));
        }
        else
        {
            list_ = NULL;
        }
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetNodeTag(
            nodeId,tagName,strlen(tagName)+1,string,strlen(string)+1,lev,list_);
    }

    ~DRAL_SETNODETAGSTRING_STORAGE_CLASS(void)
    {
        if (list_ != NULL)
        {
            delete [] list_;
        }
        free((char *)string);
        free((char *)tagName);
    }
};
typedef DRAL_SETNODETAGSTRING_STORAGE_CLASS * DRAL_SETNODETAGSTRING_STORAGE;

class DRAL_SETNODETAG_STORAGE_CLASS
     : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 nodeId;
    char * tagName;
    UINT64 val;
    UINT16 lev;
    UINT32 * list_;

  public:
  
    DRAL_SETNODETAG_STORAGE_CLASS (
        UINT16 node_id, const char tag_name [], UINT64 value,
        UINT16 level, UINT32 lst [])
    {
        nodeId = node_id;
        tagName = strdup(tag_name);
        assert(tagName);
        val=value;
        lev=level;
        if (lev)
        {
            list_ = new UINT32 [lev];
            assert(list_);
            memcpy(list_,lst,lev*sizeof(*list_));
        }
        else
        {
            list_ = NULL;
        }
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetNodeTag(
            nodeId,tagName,strlen(tagName)+1,val,lev,list_);
    }

    ~DRAL_SETNODETAG_STORAGE_CLASS(void)
    {
        if (list_ != NULL)
        {
            delete [] list_;
        }
        free(tagName);
    }
};
typedef DRAL_SETNODETAG_STORAGE_CLASS *
    DRAL_SETNODETAG_STORAGE;

class DRAL_SETITEMTAGSET_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT32 itemId;
    char *tagName;
    UINT32 n;
    UINT64 * set;

  public:

    DRAL_SETITEMTAGSET_STORAGE_CLASS (
        UINT32 item_id, const char tag_name[], UINT32 nval,
        UINT64 value[])
    {
        itemId=item_id;
        tagName = strdup(tag_name);
        assert(tagName);
        n=nval;
        set = new UINT64 [n];
        assert(set);
        memcpy(set,value,n*sizeof(*set));
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetItemTag(itemId,tagName,strlen(tagName)+1,n,set);
    }

    ~DRAL_SETITEMTAGSET_STORAGE_CLASS(void)
    {
        free((char *)tagName);
        delete [] set;
    }
};
typedef DRAL_SETITEMTAGSET_STORAGE_CLASS * DRAL_SETITEMTAGSET_STORAGE;

class DRAL_SETITEMTAGSTRING_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT32 itemId;
    char * tagName;
    char * string;

  public:

    DRAL_SETITEMTAGSTRING_STORAGE_CLASS (
        UINT32 item_id, const char tag_name[], const char str[])
    {
        itemId=item_id;
        tagName = strdup(tag_name);
        assert(tagName);
        string = strdup(str);
        assert(string);
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetItemTag(
            itemId,tagName,strlen(tagName)+1,string,strlen(string)+1);
    }

    ~DRAL_SETITEMTAGSTRING_STORAGE_CLASS(void)
    {
        free((char *)tagName);
        free((char *)string);
    }
};
typedef DRAL_SETITEMTAGSTRING_STORAGE_CLASS * DRAL_SETITEMTAGSTRING_STORAGE;

class DRAL_SETITEMTAG_STORAGE_CLASS
    : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 itemId;
    char * tagName;
    UINT64 val;

  public:
  
    DRAL_SETITEMTAG_STORAGE_CLASS (
        UINT32 item_id, const char tag_name[], UINT64 value)
    {
        itemId=item_id;
        tagName = strdup(tag_name);
        assert(tagName);
        val=value;
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetItemTag(itemId,tagName,strlen(tagName)+1,val);
    }

    ~DRAL_SETITEMTAG_STORAGE_CLASS(void)
    {
        free((char *)tagName);
    }
};
typedef DRAL_SETITEMTAG_STORAGE_CLASS * 
    DRAL_SETITEMTAG_STORAGE;

class DRAL_ENTERNODE_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 nodeId;
    UINT32 itemId;
    UINT16 dim;
    UINT32 * pos;

  public:

    DRAL_ENTERNODE_STORAGE_CLASS (
        UINT16 node_id, UINT32 item_id, UINT16 dimensions, UINT32 position[])
    {
        nodeId=node_id;
        itemId=item_id;
        dim=dimensions;
        if (dim)
        {
            pos = new UINT32 [dim];
            assert(pos);
            memcpy(pos,position,sizeof(*pos)*dim);
        }
        else
        {
            pos = NULL;
        }
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->EnterNode(nodeId,itemId,dim,pos);
    }
    
    ~DRAL_ENTERNODE_STORAGE_CLASS(void)
    {
        if (pos)
        {
            delete [] pos;
        }
    }
};
typedef DRAL_ENTERNODE_STORAGE_CLASS * DRAL_ENTERNODE_STORAGE;

class DRAL_EXITNODE_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 nodeId;
    UINT32 itemId;
    UINT16 dim;
    UINT32 * pos;

  public:

    DRAL_EXITNODE_STORAGE_CLASS (
        UINT16 node_id, UINT32 item_id, UINT16 dimensions, UINT32 position[])
    {
        nodeId=node_id;
        itemId=item_id;
        dim=dimensions;
        if (dim)
        {
            pos = new UINT32 [dim];
            assert(pos);
            memcpy(pos,position,sizeof(*pos)*dim);
        }
        else
        {
            pos = NULL;
        }
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->ExitNode(nodeId,itemId,dim,pos);
    }
    
    ~DRAL_EXITNODE_STORAGE_CLASS(void)
    {
        if (pos)
        {
            delete [] pos;
        }
    }
};
typedef DRAL_EXITNODE_STORAGE_CLASS * DRAL_EXITNODE_STORAGE;

class DRAL_NEWITEM_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT32 itemId;

  public:
  
    DRAL_NEWITEM_STORAGE_CLASS (UINT32 item_id)
    {
        itemId=item_id;
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->NewItem(itemId);
    }
};
typedef DRAL_NEWITEM_STORAGE_CLASS * DRAL_NEWITEM_STORAGE;

class DRAL_DELETEITEM_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT32 itemId;

  public:
  
    DRAL_DELETEITEM_STORAGE_CLASS (UINT32 item_id)
    {
        itemId=item_id;
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->DeleteItem(itemId);
    }
};
typedef DRAL_DELETEITEM_STORAGE_CLASS * DRAL_DELETEITEM_STORAGE;

class DRAL_COMMENT_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT32 magicNum;
    char * cont;
    
  public:
  
    DRAL_COMMENT_STORAGE_CLASS (
        UINT32 magic_num, const char * contents)
    {
        magicNum=magic_num;
        cont = new char [strlen(contents)+1];
        assert(cont);
        strcpy(cont,contents);
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->Comment(magicNum,cont,strlen(cont)+1);
    }

    ~DRAL_COMMENT_STORAGE_CLASS(void)
    {
        delete [] cont;
    }
};
typedef DRAL_COMMENT_STORAGE_CLASS * DRAL_COMMENT_STORAGE;

class DRAL_COMMENTBIN_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 magicNum;
    char * cont;
    UINT32 length;
    
  public:
  
    DRAL_COMMENTBIN_STORAGE_CLASS (
        UINT16 magic_num, const char * contents, UINT32 len)
    {
        magicNum=magic_num;
        cont = new char [len];
        assert(cont);
        memcpy(cont, contents, len);
        length = len;
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->CommentBin(magicNum,cont,length);
    }

    ~DRAL_COMMENTBIN_STORAGE_CLASS(void)
    {
        delete [] cont;
    }
};
typedef DRAL_COMMENTBIN_STORAGE_CLASS * DRAL_COMMENTBIN_STORAGE;

class DRAL_MOVEITEMS_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
    
    UINT16 edgeId;
    UINT32 numItems;
    UINT32 * itemIds;
    UINT32 * poss;

  public:

    DRAL_MOVEITEMS_STORAGE_CLASS (
        UINT16 edge_id, UINT32 n, UINT32 item_id[], UINT32 position [])
    {
        edgeId=edge_id;
        numItems=n;
        itemIds = new UINT32 [numItems];
        assert(itemIds);
        memcpy(itemIds,item_id,numItems*sizeof(*item_id));
        if (position)
        {
            poss = new UINT32 [numItems];
            assert(poss);
            memcpy(poss,position,numItems*sizeof(*item_id));
        }
        else
        {
            poss=NULL;
        }
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->MoveItems(edgeId,numItems,itemIds,poss);
    }

    ~DRAL_MOVEITEMS_STORAGE_CLASS(void)
    {
        delete [] itemIds;
        if (poss)
        {
            delete [] poss;
        }
    }
};
typedef DRAL_MOVEITEMS_STORAGE_CLASS * DRAL_MOVEITEMS_STORAGE;

class DRAL_CYCLE_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT64 cy;

  public:
  
    DRAL_CYCLE_STORAGE_CLASS (UINT64 cycle)
    {
        cy=cycle;
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->Cycle(cy);
    }
};
typedef DRAL_CYCLE_STORAGE_CLASS * DRAL_CYCLE_STORAGE;

class DRAL_SETCYCLETAGSET_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    char * tagName;
    UINT32 n;
    UINT64 * set;

  public:

    DRAL_SETCYCLETAGSET_STORAGE_CLASS (
        const char tag_name[], UINT32 nval, UINT64 value[])
    {
        tagName = strdup(tag_name);
        assert(tagName);
        n=nval;
        set = new UINT64 [n];
        assert(set);
        memcpy(set,value,n*sizeof(*set));
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetCycleTag(tagName,strlen(tagName)+1,n,set);
    }

    ~DRAL_SETCYCLETAGSET_STORAGE_CLASS(void)
    {
        free((char *)tagName);
        delete [] set;
    }
};
typedef DRAL_SETCYCLETAGSET_STORAGE_CLASS * DRAL_SETCYCLETAGSET_STORAGE;

class DRAL_SETCYCLETAGSTRING_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    char * tagName;
    char * string;

  public:

    DRAL_SETCYCLETAGSTRING_STORAGE_CLASS (
        const char tag_name[], const char str[])
    {
        tagName = strdup(tag_name);
        assert(tagName);
        string = strdup(str);
        assert(string);
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetCycleTag(
            tagName,strlen(tagName)+1,string,strlen(string)+1);
    }

    ~DRAL_SETCYCLETAGSTRING_STORAGE_CLASS(void)
    {
        free((char *)tagName);
        free((char *)string);
    }
};
typedef DRAL_SETCYCLETAGSTRING_STORAGE_CLASS * DRAL_SETCYCLETAGSTRING_STORAGE;

class DRAL_SETCYCLETAG_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    char * tagName;
    UINT64 val;

  public:
  
    DRAL_SETCYCLETAG_STORAGE_CLASS (
        const char tag_name[], UINT64 value)
    {
        tagName = strdup(tag_name);
        assert(tagName);
        val=value;
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetCycleTag(tagName,strlen(tagName)+1,val);
    }

    ~DRAL_SETCYCLETAG_STORAGE_CLASS(void)
    {
        free((char *)tagName);
    }
};
typedef DRAL_SETCYCLETAG_STORAGE_CLASS *
    DRAL_SETCYCLETAG_STORAGE;

class DRAL_SETNODEINPUTBANDWIDTH_STORAGE_CLASS
    : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 nodeId;
    UINT32 bandwidth;

  public:
  
    DRAL_SETNODEINPUTBANDWIDTH_STORAGE_CLASS(UINT16 node_id, UINT32 bw)
    {
        nodeId = node_id;
        bandwidth = bw;
    }

    void Notify(DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetNodeInputBandwidth(nodeId,bandwidth);
    }
};
typedef DRAL_SETNODEINPUTBANDWIDTH_STORAGE_CLASS *
    DRAL_SETNODEINPUTBANDWIDTH_STORAGE;

class DRAL_SETNODEOUTPUTBANDWIDTH_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    UINT16 nodeId;
    UINT32 bandwidth;

  public:
  
    DRAL_SETNODEOUTPUTBANDWIDTH_STORAGE_CLASS(UINT16 node_id, UINT32 bw)
    {
        nodeId = node_id;
        bandwidth = bw;
    }

    void Notify(DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetNodeOutputBandwidth(nodeId,bandwidth);
    }
};
typedef DRAL_SETNODEOUTPUTBANDWIDTH_STORAGE_CLASS *
    DRAL_SETNODEOUTPUTBANDWIDTH_STORAGE;

class DRAL_SETTAGDESCRIPTION_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
  
    char * tag;
    char * desc;
    
  public:
  
    DRAL_SETTAGDESCRIPTION_STORAGE_CLASS (
        const char tag_name [], const char description [])
    {
        tag = strdup(tag_name);
        assert(tag);
        desc = strdup(description);
        assert(desc);
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetTagDescription(
            tag,strlen(tag)+1,desc,strlen(desc)+1);
    }

    ~DRAL_SETTAGDESCRIPTION_STORAGE_CLASS(void)
    {
        free((char *)tag);
        free((char *)desc);
    }
};
typedef DRAL_SETTAGDESCRIPTION_STORAGE_CLASS * DRAL_SETTAGDESCRIPTION_STORAGE;

class DRAL_SETNODECLOCK_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
    UINT16 nodeId;
    UINT16 clockId;

  public:
    DRAL_SETNODECLOCK_STORAGE_CLASS (
        UINT16 node_id, UINT16 clock_id)
    {
        nodeId = node_id;
        clockId = clock_id;
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->SetNodeClock(nodeId,clockId);
    }
};
typedef DRAL_SETNODECLOCK_STORAGE_CLASS * DRAL_SETNODECLOCK_STORAGE;

class DRAL_NEWCLOCK_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
    UINT16 clockId;
    UINT64 freq;
    UINT16 skew;
    UINT16 divisions;
    char * name;

  public:
    DRAL_NEWCLOCK_STORAGE_CLASS (
        UINT16 clock_id, UINT64 fr, UINT16 sk, UINT16 div, const char clock_name [])
    {
        clockId = clock_id;
        freq = fr;
        skew = sk;
        divisions = div;
        name=strdup(clock_name);
        assert(name);
    }

    ~DRAL_NEWCLOCK_STORAGE_CLASS ()
    {
	free(name);
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->NewClock(clockId, freq, skew, divisions, name, strlen(name) + 1);
    }
};
typedef DRAL_NEWCLOCK_STORAGE_CLASS * DRAL_NEWCLOCK_STORAGE;

class DRAL_CYCLEWITHCLOCK_STORAGE_CLASS : public DRAL_COMMAND_STORAGE_CLASS
{
  private:
    UINT16 clockId;
    UINT64 cycle;
    UINT16 phase;

  public:
    DRAL_CYCLEWITHCLOCK_STORAGE_CLASS (
        UINT16 clock_id, UINT64 cy, UINT16 ph)
    {
        clockId = clock_id;
        cycle = cy;
        phase = ph;
    }

    void Notify (DRAL_SERVER_IMPLEMENTATION implementation)
    {
        implementation->Cycle(clockId, cycle, phase);
    }
};
typedef DRAL_CYCLEWITHCLOCK_STORAGE_CLASS * DRAL_CYCLEWITHCLOCK_STORAGE;


#endif /* __dralStorage_h */
