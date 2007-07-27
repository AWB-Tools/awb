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

/**
 * @file dralServer.cpp
 * @author Pau Cabre 
 * @brief dral server
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>
#include <string.h>

using namespace std;

#include "asim/dralServer.h"
#include "asim/dralServerImplementation.h"
#include "asim/dralServerBinary.h"
#include "asim/dralServerAscii.h"
#include "asim/dralDesc.h"

// hack: not all platforms define O_LARGEFILE so
#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

#define ENABLE_VALIST_CODE COMPILE_DRAL_WITH_PTV


// used for va_list iteration and conversion
// FIXME: this is not portable!!!
typedef union VA_LIST_UNION_TYPE {
    va_list ap;
    void *p;
    DRAL_DATA_DESC_CLASS **ddpp;
    PTV_DATA_TYPE_CLASS **pdpp;
} VA_LIST_UNION_T;

/*
 * constructor with the name of the file that will be used
 * to write the events and the size of the write buffer
 */
DRAL_SERVER_CLASS::DRAL_SERVER_CLASS(
    char * fileName, UINT16 buffer_size, bool avoid_rep,
    bool compression, bool embededTarFile)
{
    Init(buffer_size,avoid_rep,compression);
    file_name = fileName;
    openedWithFileName=true;
    fileOpened=false;
    embeded_tar_file=embededTarFile;
    com_edge_bw=false;
    max_edge_bw=NULL;
}

/*
 * constructor with the file descriptor that will be used
 * to write the events and the size of the write buffer
 * Note: The file descriptor will not be closed
 */
DRAL_SERVER_CLASS::DRAL_SERVER_CLASS(
    int fd, UINT16 buffer_size, bool avoid_rep,
    bool compression, bool embededTarFile)
{
    Init(buffer_size,avoid_rep,compression);
    implementation->SetFileDescriptor(fd);
    implementation->Version();  /* Send the Dral Server version */
    openedWithFileName=false;
    embeded_tar_file=embededTarFile;
    com_edge_bw=false;
    max_edge_bw=NULL;
}

void
DRAL_SERVER_CLASS::Init(UINT16 buffer_size, bool avoid_rep, bool compression)
{
    implementation= new DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS(
        buffer_size,compression);
    dralStorage=new DRAL_STORAGE_CLASS(implementation);
    turnedOn=false;
    buff_size=buffer_size;
    avoid_node_reps=avoid_rep;
    item_id=1; // itemId 0 is reserved (used as 'invalid' itemId value)
    node_id=0;
    edge_id=0;
    clock_id=0;

    // FEDE: for convenience, make autocompress for node tags on by default
    nodetagAutocompress = true;
    //nodetagAutocompress = false;
    // DBG HACK
    char* compress = getenv("DRAL_COMPRESS_NODETAG");
    if (compress!=NULL)
    {
        nodetagAutocompress = (!strcasecmp(compress,"true"));
        cout << "autocompress set to " << nodetagAutocompress << endl;
    }
}


/*
 * void destructor
 */
DRAL_SERVER_CLASS::~DRAL_SERVER_CLASS()
{
    delete implementation; // this will flush the buffer
    delete dralStorage; // this will free the memory
    if (openedWithFileName && fileOpened)
    {
        close(file_descriptor);

#if COMPILE_DRAL_WITH_PTV
        std::string ptv_file_name = file_name;
        ptv_file_name += ".pt";

	pipe_close_file(PIPE_NO_RECORD);
        //pipe_dump_buffer(ptv_file_name.c_str(), 
        //                 20000,
        //                 PT_Binary);
#endif        

    }
    if(com_edge_bw)
    {
        delete [] max_edge_bw;
    }
}


/*
 * public methods used to enable or disable the writing to the file
 * descriptor
 * By default, it is turned off
 */
void
DRAL_SERVER_CLASS::TurnOn()
{
    if (openedWithFileName && !fileOpened)
    {
        std::string dral_file_name = file_name;
        std::string ptv_file_name = file_name;

        dral_file_name += ".drl.gz";
        ptv_file_name += ".pt";

        file_descriptor=open(
            dral_file_name.c_str(),O_CREAT|O_WRONLY|O_TRUNC|O_LARGEFILE,00660);
        DRAL_ASSERT(
            file_descriptor!=-1,
            "Error opening the file: " << strerror(errno));
        implementation->SetFileDescriptor(file_descriptor);
        fileOpened=true;
        implementation->Version(); /* Send the Dral Server version */
        dralStorage->DumpAllCommandsList();

        

#if COMPILE_DRAL_WITH_PTV
        thread_initialize(60000); // deadlock detector count
        pipe_threads_per_proc(2);
        pipe_init(20000);
        pipe_disable_implicit_stats();
        pipe_enable_collection();
        int ptid;
        pipe_open_file(ptv_file_name.c_str(),
                       //(pt_ascii ? PT_ASCII : PT_Binary),
                       PT_Binary,
                       PIPE_NO_RECORD, PIPE_PDL_MAX, &ptid);
        DRAL_ASSERT(ptid == TRACE_FILE_DEFAULT, "unexpected ptv id");
#endif
    }
    else
    {
        dralStorage->DumpPartialList();
        DumpLiveItemIds();
    }
    dralStorage->ResetPartialList();
    turnedOn=true;
}

void
DRAL_SERVER_CLASS::TurnOff (void)
{
    if (turnedOn)
    {
        turnedOn=false;
        implementation->Flush();
    }
    else
    {
        DRAL_WARNING(
            "You have tried to turn off a dral server already turned off");
    }
}

void
DRAL_SERVER_CLASS::ChangeFileName(const char * fileName)
{
    if (!turnedOn && openedWithFileName)
    {
        if (fileOpened)
        {
            close(file_descriptor);
        }
        fileOpened=false;
        file_name = fileName;
    }
    else
    {
        DRAL_WARNING(
            "Dral file name only can be changed "
            "when the events are turned off.");
    }
}

/*
 * public methods to write events to the file descriptor
 */
void
DRAL_SERVER_CLASS::Cycle (UINT64 n, bool persistent)
{
    DRAL_ASSERT(!(n >> 58),"Parameter n is too large");

    if(com_edge_bw)
    {
        UpdateEdgeMaxBandwidth();
    }
    // flush auto-flush nodes (if any) :-)
    if (!auto_flush_nodes.empty()) AutoFlush(n);
    
    if (turnedOn)
    {
        implementation->Cycle(n);
    }
    if (persistent)
    {
        DRAL_CYCLE_STORAGE c = new DRAL_CYCLE_STORAGE_CLASS(n);
        dralStorage->Store(c,!turnedOn);
    }
}

/*
 * Private method to flush nodes with autoflush attribute set
 */
void
DRAL_SERVER_CLASS::AutoFlush(UINT64 n)
{
    unsigned int i;
    UINT16 nodeId;

    // process all the nodes with auto-flush attribute set
    for (i = 0; i < auto_flush_nodes.size(); i++)
    {
        nodeId = auto_flush_nodes[i];
        DRAL_ASSERT(nodeId < auto_flush.size(), "Internal DRAL server error, sorry :-)");
        DRAL_ASSERT(auto_flush[nodeId] == true, "Internal DRAL server error, sorry :-)");
        DRAL_ASSERT(nodeId < auto_flush_data.size(), "Internal DRAL server error, sorry :-)");
	auto_flush_entry &e = auto_flush_data[nodeId];

	// any node with autoflush MUST be configured with SetNodeLayout...
	DRAL_ASSERT(e.dimensions != 0, "DRAL error: a node was created with autoflush but its layout was not specified with SetNodeLayout");

        // An automatic flush (ExitNode) must be triggered exactly when
	// we get the sequence EnterNode/Nothing in consecutive cycles.
	// So, iterate for all the 'previous clock' EnterNodes and see check
	// whether they have also been EnterNode'd in 'current' clock.
	enternode_map::iterator j;
	for (j = e.previous.begin(); j != e.previous.end(); j++)
	{
	    if (e.current.find(j->first) == e.current.end())
	    {
	        // AutoFlush item at slot '*j' in node 'nodeId' and clear the slot
		DralExitNode(nodeId, e.contents[j->first], e.dimensions, e.previous[j->first].position, e.previous[j->first].persistent);
		e.contents[j->first] = 0;
	    }
	}

	// swap current and previous maps for the next clock
	e.previous = e.current;
	e.current.clear();
    }
}
 
void
DRAL_SERVER_CLASS::NewItem (UINT32 itemId, bool persistent)
{
    DRAL_ASSERT(itemId != 0, "Sorry, itemId 0 is reserved and cannot be used");
    if (turnedOn)
    {
        implementation->NewItem(itemId);
    }
    else if (!persistent)
    {
        liveItems.insert(itemId);
    }
    if (persistent)
    {
        DRAL_NEWITEM_STORAGE ni = new DRAL_NEWITEM_STORAGE_CLASS(itemId);
        dralStorage->Store(ni,!turnedOn);
    }
}


UINT32
DRAL_SERVER_CLASS::NewItem (bool persistent)
{
    NewItem(item_id,persistent);
    item_id++;
    if (item_id == UINT32_MAX)
    {
        cout<<"We have reached the limit here.."<<endl;
        item_id = 1;
    }
    return (item_id-1);
}


/*
 * set item tag single value variant
 */
void
DRAL_SERVER_CLASS::SetItemTag (
    UINT32 itemId, const char tag_name[], UINT64 value, bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    if (turnedOn)
    {
        implementation->SetItemTag(itemId,tag_name,tag_name_len,value);
    }
    if (persistent)
    {
        DRAL_SETITEMTAG_STORAGE sitsv =
            new DRAL_SETITEMTAG_STORAGE_CLASS(itemId,tag_name,value);
        dralStorage->Store(sitsv,!turnedOn);
    }
}


/*
 * set item tag string variant
 */
void
DRAL_SERVER_CLASS::SetItemTag (
    UINT32 itemId, const char tag_name[], const char str[], bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    DRAL_ASSERT(str!=NULL,"No string provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    UINT32 str_len = strlen(str)+1;
    DRAL_ASSERT(str_len < 65536 && str_len != 0,"Wrong string length");
    if (turnedOn)
    {
        implementation->SetItemTag(
            itemId,tag_name,tag_name_len,str,str_len);
    }
    if (persistent)
    {
        DRAL_SETITEMTAGSTRING_STORAGE sitstring =
            new DRAL_SETITEMTAGSTRING_STORAGE_CLASS(itemId,tag_name,str);
        dralStorage->Store(sitstring,!turnedOn);
    }
}


/*
 * set item tag set variant
 */
void
DRAL_SERVER_CLASS::SetItemTag (
    UINT32 itemId, const char tag_name [],
    UINT32 nval, UINT64 value[], bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    DRAL_ASSERT(nval<65536 && nval!=0 && value != NULL,
        "The set size is not valid");
    if (turnedOn)
    {
        implementation->SetItemTag(itemId,tag_name,tag_name_len,nval,value);
    }
    if (persistent)
    {
        DRAL_SETITEMTAGSET_STORAGE sitset =
            new DRAL_SETITEMTAGSET_STORAGE_CLASS(itemId,tag_name,nval,value);
        dralStorage->Store(sitset,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::MoveItems (
    UINT16 edgeId, UINT32 n, UINT32 itemId[],
    UINT32 position [], bool persistent)
{
    DRAL_ASSERT(n<32,"Parameter n has to be fewer than 31");
    if(com_edge_bw)
    {
        map<UINT16, UINT32>::iterator it = edge_bw.find(edgeId);
        if(it == edge_bw.end())
        {
            edge_bw[edgeId] = n;
        }
        else
        {
            it->second += n;
        }
    }
    if (turnedOn && n!=0) // we do not want an error if n == 0, just ignore it
    {
        implementation->MoveItems(edgeId,n,itemId,position);
    }
    if (persistent && n!=0)
    {
        DRAL_MOVEITEMS_STORAGE mi = new DRAL_MOVEITEMS_STORAGE_CLASS (
            edgeId,n,itemId,position);
        dralStorage->Store(mi,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::EnterNode (
    UINT16 nodeId, UINT32 itemId, UINT16 dim,
    UINT32 position [], bool persistent)
{
    UINT32 final_position, oldItemId;
    enternode_data ed;

    //
    // AutoFlush implementation of EnterNode. This routine acts as a "smart"
    // interface between user's calls to EnterNode/Exitnode and the real DRAL
    // commands sent between DRAL server and client, implemented by
    // DralEnterNode and DralExitNode private methods.
    // 
    if (auto_flush[nodeId] == false)
    {
        // if the node was not created with the autoflush attribute set, then
	// just call the regular dral methods (now private)
	DralEnterNode(nodeId, itemId, dim, position, persistent);
    }
    else
    {
        // first make sure the received nodeId is not out of range
        DRAL_ASSERT(nodeId <= auto_flush.size(), "Invalid nodeId specified in EnterNode (out of range)");
	
        // make sure the layout was set for the node and matches current position
	DRAL_ASSERT(auto_flush_data[nodeId].dimensions != 0, "EnterNode() issued with no previous node layout definition!");
	DRAL_ASSERT(auto_flush_data[nodeId].dimensions == dim, "EnterNode() issued with an invalid position reference!");

	// compute the final position of the item in the node contents array
	final_position = ComputeNodePosition(nodeId, dim, position);
	ed.position = new UINT32[dim];
	memcpy(ed.position, position, sizeof(position));
	ed.persistent = persistent;

	// now look at the contents array and decide what to do
	oldItemId = auto_flush_data[nodeId].contents[final_position];
	if (!oldItemId)
	{
	    // position was empty, so issue a regular DRAL EnterNode command and remember it
	    DralEnterNode(nodeId, itemId, dim, position, persistent);
	    auto_flush_data[nodeId].contents[final_position] = itemId;
	    auto_flush_data[nodeId].current[final_position] = ed;
	}
	else
	{
	    if (oldItemId == itemId)
	    {
	        // same item in same position, so DO NOT issue any DRAL command
		// (optimization)
            }
	    else
	    {
	        // different item in the same position, issue exitnode + enternode
		DralExitNode(nodeId, oldItemId, dim, position, persistent);
		DralEnterNode(nodeId, itemId, dim, position, persistent);
		auto_flush_data[nodeId].contents[final_position] = itemId;
	        auto_flush_data[nodeId].current[final_position] = ed;
	    }
	}    
    }
}

void
DRAL_SERVER_CLASS::ExitNode (
    UINT16 nodeId, UINT32 itemId, UINT16 dim,
    UINT32 position [], bool persistent)
{
    UINT32 final_position, oldItemId;

    //
    // AutoFlush implementation of ExitNode. This routine acts as a "smart"
    // interface between user's calls to EnterNode/Exitnode and the real DRAL
    // commands sent between DRAL server and client and implemented by
    // DralEnterNode and DralExitNode private methods.
    // 
    
    if (auto_flush[nodeId] == false)
    {
        // if the node was not created with the autoflush attribute set, then
	// just call the regular dral methods (now private)
	DralExitNode(nodeId, itemId, dim, position, persistent);
    }
    else
    {
        // only "relaxed" autoflush allows ExitNodes to be issued, so check that
        DRAL_ASSERT(auto_flush_data[nodeId].relaxed == true, "Calls to ExitNode not allowed for auto-flush nodes. If you want to allow that, set the 'relaxed' attribute at node creation (NewNode)");
	
        // first make sure the received nodeId is not out of range
        DRAL_ASSERT(nodeId <= auto_flush.size(), "Invalid nodeId specified in ExitNode (out of range)");
	
        // make sure the layout was set for the node and matches current position
	DRAL_ASSERT(auto_flush_data[nodeId].dimensions != 0, "ExitNode() issued with no previous node layout definition!");
	DRAL_ASSERT(auto_flush_data[nodeId].dimensions == dim, "ExitNode() issued with an invalid position reference!");

	// compute the final position of the item in the node contents array
	final_position = ComputeNodePosition(nodeId, dim, position);

        // the same item must be in that position
	oldItemId = auto_flush_data[nodeId].contents[final_position];
	DRAL_ASSERT(oldItemId != 0, "ExitNode issued with no previous item in specified position!");
	DRAL_ASSERT(oldItemId == itemId, "Incorrect ExitNode issued: the previous item does not match the exiting item");

	// ok, we can issue the regular DRAL ExitNode command 
	DralExitNode(nodeId, itemId, dim, position, persistent);
	auto_flush_data[nodeId].contents[final_position] = 0;
    }
}

UINT32 DRAL_SERVER_CLASS::ComputeNodePosition(
    UINT16 nodeId, UINT16 dim, UINT32 position[])
{
    UINT32 final_position, size_multiplier;
    INT32 i;

    DRAL_ASSERT(position[dim-1] < auto_flush_data[nodeId].capacity[dim-1], "Enter/ExitNode() issued with an invalid position reference!");
    final_position = position[dim-1];
    size_multiplier = 1;
    for (i = dim-2; i >= 0; i--)
    {
        DRAL_ASSERT(position[i] < auto_flush_data[nodeId].capacity[i], "Enter/ExitNode() issued with an invalid position reference!");
	size_multiplier *= auto_flush_data[nodeId].capacity[i];
	final_position += position[i] * size_multiplier;
    }
    return final_position;
}    

void
DRAL_SERVER_CLASS::DralEnterNode (
    UINT16 nodeId, UINT32 itemId, UINT16 dim,
    UINT32 position [], bool persistent)
{
    DRAL_ASSERT(dim!=0 && position!=NULL,
        "Number of dimensions == 0 or position list == NULL");
    DRAL_ASSERT(dim < 16, "Number of dimensions must be lower than 16");
    if (turnedOn)
    {
        implementation->EnterNode(nodeId,itemId,dim,position);
    }
    if (persistent)
    {
        DRAL_ENTERNODE_STORAGE en = new DRAL_ENTERNODE_STORAGE_CLASS (
            nodeId,itemId,dim,position);
        dralStorage->Store(en,!turnedOn);
    }
}    

void
DRAL_SERVER_CLASS::DralExitNode (
    UINT16 nodeId, UINT32 itemId, UINT16 dim,
    UINT32 position [], bool persistent)
{
    DRAL_ASSERT(dim!=0 && position!=NULL,
        "Number of dimensions == 0 or position list == NULL");
    DRAL_ASSERT(dim < 16, "Number of dimensions must be lower than 16");
    if (turnedOn)
    {
        implementation->ExitNode(nodeId,itemId,dim,position);
    }
    if (persistent)
    {
        DRAL_EXITNODE_STORAGE en = new DRAL_EXITNODE_STORAGE_CLASS (
            nodeId,itemId,dim,position);
        dralStorage->Store(en,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::DeleteItem (UINT32 itemId, bool persistent)
{
    if (turnedOn)
    {
        implementation->DeleteItem(itemId);
    }
    else if (!persistent)
    {
        liveItemsList::iterator it = liveItems.find(itemId);
        if (it != liveItems.end())
        {
            liveItems.erase(it);
        }
    }
    if (persistent)
    {
        DRAL_DELETEITEM_STORAGE di = new DRAL_DELETEITEM_STORAGE_CLASS (itemId);
        dralStorage->Store(di,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::NewNode (
    UINT16 nodeId, const char name[], UINT16 parent_id,
    UINT16 instance, bool persistent, bool autoflush, bool relaxed)
{
    UINT32 name_len = strlen(name)+1;
    DRAL_ASSERT((name!=NULL) && (name_len<1024),
        "No node name provided or too long node name");
    UINT16 instance_temp=((instance == 0 && avoid_node_reps)?
        dralStorage->GetInstance(name) : instance);
    if (turnedOn)
    {
        implementation->NewNode(
            nodeId,name,name_len,parent_id,instance_temp);
    }
    if (persistent)
    {
        DRAL_NEWNODE_STORAGE nn = new DRAL_NEWNODE_STORAGE_CLASS (
            nodeId,name,parent_id,instance_temp);
        dralStorage->Store(nn,!turnedOn);
    }

    // Autoflush implementation

    // first, make sure we have space in the status arrays (indexed by nodeId)
    if (nodeId >= auto_flush.capacity())
    {
        unsigned long new_size = ((nodeId / chunk_size) + 1) * chunk_size;
        auto_flush.reserve(new_size);
        auto_flush_data.reserve(new_size);
        DRAL_ASSERT(auto_flush.capacity() == new_size, "Unable to allocate memory for auto_flush vector");
        DRAL_ASSERT(auto_flush_data.capacity() == new_size, "Unable to allocate memory for auto_flush_data vector");

        // empty the new space
	auto_flush.resize(new_size, false);
	auto_flush_data.resize(new_size);
    }
    
    // remember whether this node was created with autoflush
    auto_flush[nodeId] = autoflush;

    // now, if autoflush is requested, grab some more information
    if (autoflush)
    {
	// Before the call to SetLayout, we don't know how many items we need
	// to keep, so don't new() yet
	auto_flush_data[nodeId].relaxed = relaxed;
	auto_flush_data[nodeId].dimensions = 0;
	auto_flush_data[nodeId].capacity = NULL;
	auto_flush_data[nodeId].contents = NULL;
	auto_flush_data[nodeId].current.clear();
	auto_flush_data[nodeId].previous.clear();
       	
	// finally, add the node to the list of nodes with autoflush
	auto_flush_nodes.push_back(nodeId);
    } 
}

UINT16
DRAL_SERVER_CLASS::NewNode (
    const char name[], UINT16 parent_id, UINT16 instance, 
    bool persistent, bool autoflush, bool relaxed)
{
    NewNode(node_id,name,parent_id,instance,persistent,autoflush,relaxed);
    node_id++;
    return (node_id-1);
}

void
DRAL_SERVER_CLASS::NewEdge (
    UINT16 edgeId, UINT16 source_node, UINT16 destination_node,
    UINT32 bandwidth, UINT32 latency, const char name[], bool persistent)
{
    UINT32 name_len = strlen(name)+1;
    DRAL_ASSERT((name!=NULL) && (name_len<1024),
        "No node name provided or too long edge name");
    if (turnedOn)
    {
        implementation->NewEdge(
            edgeId,source_node,destination_node,
            bandwidth,latency,name,strlen(name)+1);
    }
    if (persistent)
    {
        DRAL_NEWEDGE_STORAGE ne = new DRAL_NEWEDGE_STORAGE_CLASS (
            edgeId,source_node,destination_node,bandwidth,latency,name);
        dralStorage->Store(ne,!turnedOn);
    }
}

UINT16
DRAL_SERVER_CLASS::NewEdge (
    UINT16 source_node, UINT16 destination_node,
    UINT32 bandwidth, UINT32 latency, const char name[], bool persistent)
{
    NewEdge(
        edge_id,source_node,destination_node,bandwidth,latency,name,persistent);
    edge_id++;
    return (edge_id-1);
}


void
DRAL_SERVER_CLASS::SetNodeLayout (
    UINT16 nodeId, UINT16 dimensions, UINT32 capacity [], bool persistent)
{
    DRAL_ASSERT((dimensions!=0) && (capacity!=NULL),
        "Bad dimension or capacity");
    if (turnedOn)
    {
        implementation->SetNodeLayout(nodeId,dimensions,capacity);
    }
    if (persistent)
    {
        DRAL_SETNODELAYOUT_STORAGE sl = new DRAL_SETNODELAYOUT_STORAGE_CLASS (
            nodeId,dimensions,capacity);
        dralStorage->Store(sl,!turnedOn);
    }
    
    // make sure the received nodeId is not out of range
    DRAL_ASSERT(nodeId <= auto_flush.size(), "Invalid nodeId specified in SetNodeLayout (out of range)");

    // if node was created with auto-flush flag, we need to allocate storage
    // for node contents (items)
    if (auto_flush[nodeId])
    {
        UINT16 i, total_size;
	
        auto_flush_data[nodeId].dimensions = dimensions;
        auto_flush_data[nodeId].capacity = new UINT32[dimensions];
	total_size = 1;
        for (i = 0; i < dimensions; i++)
        {
            auto_flush_data[nodeId].capacity[i] = capacity[i];
	    total_size *= capacity[i];
        }
        auto_flush_data[nodeId].contents = new UINT32[total_size];
	memset(auto_flush_data[nodeId].contents, 0, sizeof(auto_flush_data[nodeId].contents));
    }
}


void
DRAL_SERVER_CLASS::Comment(
    UINT32 magic_num, const char comment [], bool persistent)
{
    UINT32 comment_len = strlen(comment)+1;
    DRAL_ASSERT(comment!=NULL && comment_len<(UINT32_MAX) && comment_len != 0,
        "Comment == NULL or wrong comment length");
    if (turnedOn)
    {
        implementation->Comment(magic_num,comment,comment_len);
    }
    if (persistent)
    {
        DRAL_COMMENT_STORAGE c = new DRAL_COMMENT_STORAGE_CLASS (
            magic_num,comment);
        dralStorage->Store(c,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::CommentBin(
    UINT16 magic_num, const char * contents, UINT32 length, bool persistent)
{
    DRAL_ASSERT(length != 0, "Binary content with length 0 bytes");
    if (turnedOn)
    {
        implementation->CommentBin(magic_num,contents,length);
    }
    if (persistent)
    {
        DRAL_COMMENTBIN_STORAGE cb = new DRAL_COMMENTBIN_STORAGE_CLASS (
            magic_num,contents,length);
        dralStorage->Store(cb,!turnedOn);
    }
}

Nodetagcache_tag_map*
DRAL_SERVER_CLASS::getNodeTagMap(const char tag_name[])
{
    Nodetagcache_tag_map* result=NULL;
    
    Nodetagcache_map::iterator it = nodetagcache.find(tag_name);
    if (it != nodetagcache.end())
    {
        result = it->second;
    }
    else
    {
        // add an entry for this tag
        result = new Nodetagcache_tag_map;
        nodetagcache[tag_name] = result;
    }
    return result;
}


void
DRAL_SERVER_CLASS::SetNodeTag(
    UINT16 node_id, const char tag_name [], UINT64 value,
    UINT16 level, UINT32 list [], bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    DRAL_ASSERT(level < 16384, "Too many levels specified");

    Nodetagcache_tag_map* tgmap=NULL;
    bool doCmd=true;

    if (turnedOn)
    {
        // check for compression oportunity
        if (nodetagAutocompress && !level)
        {
            tgmap=getNodeTagMap(tag_name);
            Nodetagcache_tag_map::iterator tgit = tgmap->find(node_id);
            if (tgit!=tgmap->end())
            {
                //cerr << "cacheed value for node_id " << node_id << " tg " << tag_name << " value " << tgit->second << endl;
                if (tgit->second == value) 
                { 
                    //cerr << "\t value matches, cmd ignored..."  << endl;
                    doCmd=false;
                }
            }
        }

        if (doCmd) 
        {
            implementation->SetNodeTag(node_id,tag_name,tag_name_len,value,level,list);
            
            // update compression struct if needed
            if (nodetagAutocompress && !level)
            {
                tgmap=getNodeTagMap(tag_name);
                pair<UINT16,UINT64> nodeval(node_id,value);
                pair<Nodetagcache_tag_map::iterator,bool> p = tgmap->insert(nodeval);
                Nodetagcache_tag_map::iterator i = p.first;
                i->second=value;
                //cerr << "upd cache node_id " << node_id << " tg " << tag_name << " value " << value << endl;
            }
        }
    }
    if (persistent && doCmd)
    {
        DRAL_SETNODETAG_STORAGE sntsv = 
            new DRAL_SETNODETAG_STORAGE_CLASS (
                node_id,tag_name,value,level,list);
        dralStorage->Store(sntsv,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::SetNodeTag(
    UINT16 node_id, const char tag_name [], const char str [],
    UINT16 level, UINT32 list [], bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    DRAL_ASSERT(str!=NULL,"No string provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    UINT32 str_len = strlen(str)+1;
    DRAL_ASSERT(str_len < 65536 && str_len != 0,"Wrong string length");
    if (turnedOn)
    {
        implementation->SetNodeTag(
            node_id,tag_name,tag_name_len,str,str_len,level,list);
    }
    if (persistent)
    {
        DRAL_SETNODETAGSTRING_STORAGE sntstr = 
            new DRAL_SETNODETAGSTRING_STORAGE_CLASS (
                node_id,tag_name,str,level,list);
        dralStorage->Store(sntstr,!turnedOn);
    }    
}

void
DRAL_SERVER_CLASS::SetNodeTag(
    UINT16 node_id, const char tag_name [], UINT16 nval, UINT64 set [],
    UINT16 level, UINT32 list [], bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    DRAL_ASSERT(nval<65535 && nval!=0 && set != NULL,
        "The set size is not valid");
    if (turnedOn)
    {
        implementation->SetNodeTag(
            node_id,tag_name,tag_name_len,nval,set,level,list);
    }
    if (persistent)
    {
        DRAL_SETNODETAGSET_STORAGE sntset = 
            new DRAL_SETNODETAGSET_STORAGE_CLASS (
                node_id,tag_name,nval,set,level,list);
        dralStorage->Store(sntset,!turnedOn);
    }
}


void
DRAL_SERVER_CLASS::SetCycleTag (
    const char tag_name[], UINT64 value, bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    if (turnedOn)
    {
        implementation->SetCycleTag(tag_name,tag_name_len,value);
    }
    if (persistent)
    {
        DRAL_SETCYCLETAG_STORAGE sctsv =
            new DRAL_SETCYCLETAG_STORAGE_CLASS(tag_name,value);
        dralStorage->Store(sctsv,!turnedOn);
    }
}


void
DRAL_SERVER_CLASS::SetCycleTag (
    const char tag_name[], const char str[], bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    DRAL_ASSERT(str!=NULL,"No string provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    UINT32 str_len = strlen(str)+1;
    DRAL_ASSERT(str_len < 65536 && str_len != 0,"Wrong string length");
    if (turnedOn)
    {
        implementation->SetCycleTag(tag_name,tag_name_len,str,str_len);
    }
    if (persistent)
    {
        DRAL_SETCYCLETAGSTRING_STORAGE sctstring =
            new DRAL_SETCYCLETAGSTRING_STORAGE_CLASS(tag_name,str);
        dralStorage->Store(sctstring,!turnedOn);
    }
}


void
DRAL_SERVER_CLASS::SetCycleTag (
    const char tag_name [],
    UINT32 nval, UINT64 value[], bool persistent)
{
    DRAL_ASSERT(tag_name!=NULL,"No tag name provided");
    UINT16 tag_name_len = strlen(tag_name)+1;
    DRAL_ASSERT(tag_name_len < 256 && tag_name_len != 0,
        "Parameter tag_name " << tag_name << " is too long");
    DRAL_ASSERT(nval<65535 && nval!=0 && value != NULL,
        "The set size is not valid");
    if (turnedOn)
    {
        implementation->SetCycleTag(tag_name,tag_name_len,nval,value);
    }
    if (persistent)
    {
        DRAL_SETCYCLETAGSET_STORAGE sctset =
            new DRAL_SETCYCLETAGSET_STORAGE_CLASS(tag_name,nval,value);
        dralStorage->Store(sctset,!turnedOn);
    }
}


void
DRAL_SERVER_CLASS::SwitchToDebug(int fd, bool compression)
{
    if (!openedWithFileName || fileOpened || turnedOn)
    {
        DRAL_WARNING("No way to switch to debug mode");
    }
    else
    {
        delete implementation; // this will flush the buffer
        implementation=new DRAL_SERVER_ASCII_IMPLEMENTATION_CLASS(
            buff_size, compression);
        dralStorage->ChangeImplementation(implementation);
        implementation->SetFileDescriptor(fd);
        implementation->Version(); /* Send the Dral Server version */
        dralStorage->DumpAllCommandsList();
        dralStorage->ResetPartialList();
        openedWithFileName=false;
    }
}

void
DRAL_SERVER_CLASS::SetNodeInputBandwidth(
    UINT16 nodeId, UINT32 bandwidth, bool persistent)
{
    if (turnedOn)
    {
        implementation->SetNodeInputBandwidth(nodeId,bandwidth);
    }
    if (persistent)
    {
        DRAL_SETNODEINPUTBANDWIDTH_STORAGE snib =
            new DRAL_SETNODEINPUTBANDWIDTH_STORAGE_CLASS (nodeId,bandwidth);
        dralStorage->Store(snib,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::SetNodeOutputBandwidth(
    UINT16 nodeId, UINT32 bandwidth, bool persistent)
{
    if (turnedOn)
    {
        implementation->SetNodeOutputBandwidth(nodeId,bandwidth);
    }
    if (persistent)
    {
        DRAL_SETNODEOUTPUTBANDWIDTH_STORAGE snob =
            new DRAL_SETNODEOUTPUTBANDWIDTH_STORAGE_CLASS (nodeId,bandwidth);
        dralStorage->Store(snob,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::StartActivity(UINT64 firstActivityCycle)
{
    UINT32 size;
    char * tmp = GetTar(&size);
    if (embeded_tar_file)
    {
        implementation->CommentBin(TAR_FILE_MAGICNUM,tmp,size);
    }
    implementation->StartActivity(firstActivityCycle);
    DumpLiveItemIds();
}

void
DRAL_SERVER_CLASS::SetTagDescription(
    const char tag [], const char description [], bool persistent)
{
    DRAL_ASSERT(tag!=NULL,"No tag name provided");
    UINT32 tag_len = strlen(tag)+1;
    DRAL_ASSERT(tag_len < 256 && tag_len != 0,
        "Parameter tag " << tag << " is too long or void");
    UINT32 desc_len = strlen(description)+1;
    DRAL_ASSERT(description!=NULL && desc_len<65536 && desc_len != 0,
        "Description == NULL or wrong description length");
    if (turnedOn)
    {
        implementation->SetTagDescription(tag,tag_len,description,desc_len);
    }
    if (persistent)
    {
        DRAL_SETTAGDESCRIPTION_STORAGE stdc =
            new DRAL_SETTAGDESCRIPTION_STORAGE_CLASS (
                tag,description);
        dralStorage->Store(stdc,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::SetNodeClock(
    UINT16 nodeId, UINT16 clockId, bool persistent)
{
    if (turnedOn)
    {
        implementation->SetNodeClock(nodeId,clockId);
    }
    if (persistent)
    {
        DRAL_SETNODECLOCK_STORAGE snc =
            new DRAL_SETNODECLOCK_STORAGE_CLASS(nodeId, clockId);
        dralStorage->Store(snc,!turnedOn);
    }
}

UINT16
DRAL_SERVER_CLASS::NewClock(
    UINT64 freq, UINT16 skew, UINT16 divisions, const char name [], bool persistent)
{
    NewClock(clock_id,freq,skew,divisions,name,persistent);
    clock_id++;
    return (clock_id-1);
}

void
DRAL_SERVER_CLASS::NewClock(
    UINT16 clockId, UINT64 freq, UINT16 skew, UINT16 divisions,
    const char name [], bool persistent)
{
    UINT32 nameLen = strlen(name) + 1;
    DRAL_ASSERT((name) && (nameLen < 65535),
        "No clock name provided or too long clock name");
    if (turnedOn)
    {
        implementation->NewClock(clockId,freq,skew,divisions,name,nameLen);
    }
    if (persistent)
    {
        DRAL_NEWCLOCK_STORAGE nc =
            new DRAL_NEWCLOCK_STORAGE_CLASS(clockId,freq,skew,divisions,name);
        dralStorage->Store(nc,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::Cycle (UINT16 clockId, UINT64 n, UINT16 phase, bool persistent)
{
    DRAL_ASSERT(!(n >> 42),"Parameter n is too large");
 
    if(com_edge_bw)
    {
        UpdateEdgeMaxBandwidth();
    }

    // flush auto-flush nodes (if any) :-)
    if (!auto_flush_nodes.empty()) AutoFlush(n);
    
    if (turnedOn)
    {
        implementation->Cycle(clockId,n,phase);
    }
    if (persistent)
    {
        DRAL_CYCLEWITHCLOCK_STORAGE c =
            new DRAL_CYCLEWITHCLOCK_STORAGE_CLASS(clockId,n,phase);
        dralStorage->Store(c,!turnedOn);
    }
}

void
DRAL_SERVER_CLASS::DumpLiveItemIds()
{
    liveItemsList::iterator it = liveItems.begin();
    while (it != liveItems.end())
    {
        implementation->NewItem(*it);
        ++it;
    }
    liveItems.clear();
}

void
DRAL_SERVER_CLASS::ComputeEdgeMaxBandwidth()
{
    com_edge_bw=true;
    max_edge_bw=new UINT32[65536];
}

UINT32
DRAL_SERVER_CLASS::GetEdgeMaxBandwidth(UINT16 edgeId)
{
    if(!com_edge_bw)
    {
        return 0;
    }
    return max_edge_bw[edgeId];
}

void
DRAL_SERVER_CLASS::UpdateEdgeMaxBandwidth()
{
    map<UINT16, UINT32>::iterator it = edge_bw.begin();

    while(it != edge_bw.end())
    {
        if(it->second > max_edge_bw[it->first])
        {
            max_edge_bw[it->first] = it->second;
        }
        ++it;
    }
    edge_bw.clear();
}

UINT32 
DRAL_SERVER_CLASS::OpenEventRec(
    DRAL_ITEM_DESC_CLASS *rec, 
    UINT32 thread_id, 
    UINT32 parent_id)
{
    DRAL_ASSERT(rec, "rec is NULL in OpenEventRec()");

    if (!turnedOn)
    {        
        return 0;
    }

    UINT32 rec_id = 0;
        
#if COMPILE_DRAL_WITH_PTV
        rec_id = pipe_open_record_inst(rec->GetPtvRecType(), thread_id, parent_id, 
                                       TRACE_FILE_DEFAULT, __FILE__, __LINE__);
#endif

    return rec_id;
}

void
DRAL_SERVER_CLASS::CloseEventRec(
    UINT32 rec_id)
{
    if (rec_id == 0)
    {
        return;
    }
    
#if COMPILE_DRAL_WITH_PTV
    pipe_close_record_inst(rec_id, __FILE__, __LINE__);
#endif

    rec_id = 0;

    return;
}

UINT32 
DRAL_SERVER_CLASS::RefEventRec(
    UINT32 rec_id)
{
    if (!turnedOn)
    {        
        return 0;
    }
    
    else if (rec_id == 0)
    {
        return 0;
    }

    UINT32 id = 0;

#if COMPILE_DRAL_WITH_PTV
    id = pipe_reference_record_inst(rec_id, __FILE__, __LINE__);
#endif
    
    return id;
}

UINT32 
DRAL_SERVER_CLASS::GetDralVaListSize(
    DRAL_DATA_DESC_CLASS *dd, 
    va_list ap)
{
    UINT32 va_size = 0;

#if ENABLE_VALIST_CODE

    VA_LIST_UNION_T i; i.ap = ap;

    while (dd)
    {
        // get the size of this data value + data pointer
        const UINT32 d_size = dd->GetVaSize(ap);

        // increment the pointer by value size
        i.p = (void*)((PTR_SIZED_UINT)i.p + (PTR_SIZED_UINT)d_size);
        
        // the new "current" data
        dd = (*i.ddpp);

        // increment the pointer again to consume the descriptor
        i.p = (void*)((PTR_SIZED_UINT)i.p + (PTR_SIZED_UINT)sizeof(DRAL_DATA_DESC_CLASS *));

        // add cum size change
        va_size += d_size + sizeof(DRAL_DATA_DESC_CLASS *);
    }

#endif 

    return va_size;
}


void 
DRAL_SERVER_CLASS::CvtApListDralToPtv(
    va_list & dst, 
    DRAL_DATA_DESC_CLASS *src_dd, 
    va_list src, 
    UINT32 size)
{
#if COMPILE_DRAL_WITH_PTV
    // just make sure out pointer arithmetic is safe.  :)
    DRAL_ASSERT(sizeof(UINT64) >= sizeof(void*), "unsafe pointer arithmetic");

    // start by copying over the raw data.  we only need to convert the 
    // data desciptor pointers.  the rest of the data remains unchanged.
    memcpy(dst, src, size);

    // create the iterator to go over the dst va_list
    VA_LIST_UNION_T i; i.ap = dst;
    
    // iterate over the new list and convert the DRAL_DATA_DESC_CLASS
    // pointers to Pipe_Datatype pointers.
    DRAL_DATA_DESC_CLASS *dd = src_dd;
    while (dd)
    {
        UINT32 size = dd->GetVaSize(i.ap);
        
        // increment the pointer by value size
        i.p = (void*)((PTR_SIZED_UINT)i.p + (PTR_SIZED_UINT)size);
        
        // the new "current" data descriptor pointer
        dd = (*i.ddpp);

        // convert the data pointer
        if (dd)
        {
            // real pointer, so we can continune
            (*i.pdpp) = dd->GetPtvDataType();
        }
        else
        {
            // NULL pointer indicates end of list
            (*i.pdpp) = PIPE_NO_DATATYPE;
            break;
        }
            
        // increment the pointer again to consume the descriptor
        i.p = (void*)((PTR_SIZED_UINT)i.p + (PTR_SIZED_UINT)sizeof(DRAL_DATA_DESC_CLASS *));
    }
#endif
    return;
}   

void 
DRAL_SERVER_CLASS::SetItemTag(
    UINT32 item_id, 
    UINT32 rec_id,
    DRAL_DATA_DESC_CLASS *data, 
    va_list & ap)
{
#if ENABLE_VALIST_CODE

    DRAL_DATA_DESC_CLASS *orig_data = data;;
    va_list orig_ap = ap;


    // TODO: add DRAL log code


#if COMPILE_DRAL_WITH_PTV
    // only if we opened this record can we record this event.
    if (rec_id != 0)
    {
        UINT32 va_size = GetDralVaListSize(orig_data, orig_ap);
        
        // allocate the new va list and copy over data of the original
        VA_LIST_UNION_T new_va; new_va.p = alloca(va_size);
        
        if (va_size)
        {
            CvtApListDralToPtv(new_va.ap, orig_data, orig_ap, va_size);
        }
        
        pipe_record_data_va(rec_id, 
                            orig_data ? orig_data->GetPtvDataType() : PIPE_NO_DATATYPE, 
                            (va_list*)&(new_va.ap));
        
    }
#endif
#endif
    return;
}

void
DRAL_SERVER_CLASS::SetEvent(
    UINT32 item_id,
    UINT32 rec_id,
    DRAL_EVENT_DESC_CLASS *desc, 
    UINT64 cycle, 
    UINT64 duration, 
    DRAL_DATA_DESC_CLASS *data, 
    va_list & ap)
{
    DRAL_ASSERT(desc, "NULL descriptor in SetEvent()");

    if (!turnedOn)
    {        
        return;
    }

#if ENABLE_VALIST_CODE

    DRAL_DATA_DESC_CLASS *orig_data = data;;
    va_list orig_ap = ap;

    // the DRAL code to process this event.
    SetItemTag(item_id, desc->GetName().c_str(), "true");

    static const std::string dot_str = ".";

    std::string item_base_str = desc->GetName() + dot_str;

    std::string cycle_name_str = item_base_str + std::string("cycle");

    char c_s[20]; 
    sprintf(c_s, "%d", (int)cycle);
    std::string cycle_val_str = c_s;
    if (duration != 1)
    {
        char d_s[20]; 
        sprintf(d_s, "%d", (int)duration);

        cycle_val_str += "/";
        cycle_val_str += d_s;
    }

    SetItemTag(item_id, cycle_name_str.c_str(), cycle_val_str.c_str());
    
    data = orig_data;
    while (data)
    {
        std::string data_name_str = item_base_str + data->GetName();
        std::string data_val_str = data->GetDralDataStr(ap, true);

        SetItemTag(item_id, data_name_str.c_str(), data_val_str.c_str());

        data = va_arg(ap, DRAL_DATA_DESC_CLASS *);
    }
 

#if COMPILE_DRAL_WITH_PTV
    // only if we opened this record can we record this event.
    if (rec_id != 0)
    {
        UINT32 va_size = GetDralVaListSize(orig_data, orig_ap);

        // allocate the new va list and copy over data of the original
        VA_LIST_UNION_T new_va; new_va.p = alloca(va_size);
        
        if (va_size)
        {
            CvtApListDralToPtv(new_va.ap, orig_data, orig_ap, va_size);
        }
        
        // the PTV code to process this event.
        pipe_record_event_time(rec_id, desc->GetPtvEventType(), 
                               (UINT32)cycle, (UINT32)duration, 
                               orig_data ? orig_data->GetPtvDataType() : PIPE_NO_DATATYPE, 
                               (va_list*)&(new_va.ap));
    }
#endif
#endif
    return;
}

DRAL_EVENT_DESC_CLASS *
DRAL_SERVER_CLASS::FindEventDesc(const char *event_name)
{
    // TODO: write me!
    return NULL;
}

void 
DRAL_SERVER_CLASS::AddEventDesc(DRAL_EVENT_DESC_CLASS *event)
{
    // TODO: write me!
    return;
}
