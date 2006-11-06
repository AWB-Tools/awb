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
 * @file dralClientBinary_v3.cpp
 * @author Pau Cabre
 * @brief dral client binary version 1 implementation
 */

#include "asim/dralClientBinary_v3.h"

/**
 * Constructor of the ascii specific implementation of the dral client
 * it only invokes the generic implementation constructor
 */
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS(
    DRAL_BUFFERED_READ dral_read,
    DRAL_LISTENER listener)
    : DRAL_CLIENT_IMPLEMENTATION_CLASS (dral_read,listener)
{
    // The last item, node and edge id starts with 0.
    last_item = 0;
    last_node = 0;
    last_edge = 0;
    EOS = false;
}

DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::~DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS()
{
}

/**
 * This is the method that uses the buffered read class. It returns a pointer
 * to the read bytes.
 * Warning: n must be != 0 (it would return NULL)
 * It returns NULL if any error found
 */
void *
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::ReadBytes (UINT32 n)
{
    INT64 k;
    void * b;

    b = dralRead->Read(n, &k);
    if(k == -1)
    {
        dralListener->Error("Error reading from the file descriptor");
        return NULL;
    }
    else if(k < n)
    {
        EOS = true;
        return NULL;
    }
    return b;
}

UINT64
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::getValue(
    DRAL3_VALUE_SIZE val_size)
{
    UINT64 value_64;
    UINT32 value_32;
    UINT16 value_16;
    UINT8  value_8;
    void * ptr;

    value_64 = 0;

    switch(val_size)
    {
        case VALUE_8_BITS:
            ptr = ReadBytes(sizeof(UINT8));
            if(!EOS)
            {
                value_8 = * (UINT8 *) ptr;
                value_64 = (UINT64) value_8;
            }
            break;

        case VALUE_16_BITS:
            ptr = ReadBytes(sizeof(UINT16));
            if(!EOS)
            {
                value_16 = * (UINT16 *) ptr;
                value_64 = (UINT64) value_16;
            }
            break;

        case VALUE_32_BITS:
            ptr = ReadBytes(sizeof(UINT32));
            if(!EOS)
            {
                value_32 = * (UINT32 *) ptr;
                value_64 = (UINT64) value_32;
            }
            break;

        case VALUE_64_BITS:
            ptr = ReadBytes(sizeof(UINT64));
            if(!EOS)
            {
                value_64 = * (UINT64 *) ptr;
            }
            break;
    }

    return value_64;
}

INT32
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::getDelta(
    DRAL3_VALUE_SIZE item_size)
{
    INT32 delta_32;
    INT16 delta_16;
    INT8  delta_8;
    void * ptr;

    switch(item_size)
    {
        case VALUE_8_BITS:
            ptr = ReadBytes(sizeof(INT8));
            if(!EOS)
            {
                delta_8 = * (INT8 *) ptr;
                return (INT32) delta_8;
            }
            break;

        case VALUE_16_BITS:
            ptr = ReadBytes(sizeof(INT16));
            if(!EOS)
            {
                delta_16 = * (INT16 *) ptr;
                return (INT32) delta_16;
            }
            break;

        case VALUE_32_BITS:
            ptr = ReadBytes(sizeof(INT32));
            if(!EOS)
            {
                delta_32 = * (INT32 *) ptr;
                return (INT32) delta_32;
            }
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }
    return 0;
}

/**
 * This method will read from the file descriptor in order to process n
 * event by calling its specific method of the listener.
 * It can be either blocking or non-blocking according to the parameter.
 */
UINT16
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::ProcessNextEvent (
    bool , UINT16 num_events)
{

   /*
    * Since this version gets the information from a file
    * the parameter blocking it is not used.
    */


    /*
     * buffer with the data read
     */
    void * buffer;

    struct dralCommand
    {
        UINT8 command : 6;
        UINT8 dummy   : 2;
    } command;

    bool r;

    UINT16 n_events;

    if (num_events == 0 || errorFound)
    {
        return 0; // if there is an error this method must do nothing
    }

    for (n_events=0;n_events<num_events;n_events++)
    {
        buffer = ReadBytes(1);

        if(EOS)
        {
            dralListener->EndSimulation();
            errorFound = true;
            break;
        }

        command = * (dralCommand *) buffer;

        /*char str[1024];
        sprintf(str, "Command is: %i", command.command);
        dralListener->Comment(0, str);*/

        switch(command.command)
        {
          case DRAL3_CYCLE:
            r=Cycle();
            break;
          case DRAL3_STARTACTIVITY:
            r=StartActivity();
            break;
          case DRAL3_NEWNODE:
            r=NewNode();
            break;
          case DRAL3_NEWEDGE:
            r=NewEdge();
            break;
          case DRAL3_SETNODELAYOUT:
            r=SetNodeLayout();
            break;
          case DRAL3_SETNODEINPUTBANDWIDTH:
            r=SetNodeInputBandwidth();
            break;
          case DRAL3_SETNODEOUTPUTBANDWIDTH:
            r=SetNodeOutputBandwidth();
            break;
          case DRAL3_COMMENT:
            r=Comment();
            break;
          case DRAL3_COMMENTBIN:
            r=CommentBin();
            break;
          case DRAL3_SETNODECLOCK:
            r=SetNodeClock();
            break;
          case DRAL3_NEWCLOCK:
            r=NewClock();
            break;
          case DRAL3_CYCLEWITHCLOCK:
            r=CycleWithClock();
            break;
          case DRAL3_SETTAGDESCRIPTION:
            r=SetTagDescription();
            break;
          case DRAL3_NEWITEM:
            r=NewItem(buffer);
            break;
          case DRAL3_DELETEITEM:
            r=DeleteItem(buffer);
            break;
          case DRAL3_MOVEITEMS:
            r=MoveItems();
            break;
          case DRAL3_ENTERNODE:
            r=EnterNode();
            break;
          case DRAL3_EXITNODE:
            r=ExitNode();
            break;
          case DRAL3_SETITEMTAG_VALUE_8_BITS:
            r=SetItemTagValue(VALUE_8_BITS);
            break;
          case DRAL3_SETITEMTAG_VALUE_16_BITS:
            r=SetItemTagValue(VALUE_16_BITS);
            break;
          case DRAL3_SETITEMTAG_VALUE_32_BITS:
            r=SetItemTagValue(VALUE_32_BITS);
            break;
          case DRAL3_SETITEMTAG_STRING:
          case DRAL3_SETITEMTAG_SET:
            r=SetItemTagOther();
            break;
          case DRAL3_SETNODETAG_VALUE_WITHLEVELS:
            r=SetNodeTagValueLevels();
            break;
          case DRAL3_SETNODETAG_VALUE_WITHOUTLEVELS_8_BITS:
            r=SetNodeTagValueNoLevels(VALUE_8_BITS);
            break;
          case DRAL3_SETNODETAG_VALUE_WITHOUTLEVELS_16_BITS:
            r=SetNodeTagValueNoLevels(VALUE_16_BITS);
            break;
          case DRAL3_SETNODETAG_STRING:
          case DRAL3_SETNODETAG_SET:
            r=SetNodeTagOther();
            break;
          case DRAL3_SETCYCLETAG_VALUE:
            r=SetCycleTagValue();
            break;
          case DRAL3_SETCYCLETAG_STRING:
          case DRAL3_SETCYCLETAG_SET:
            r=SetCycleTagOther();
            break;
          case DRAL3_NEWTAG:
            r=NewTag();
            break;
          case DRAL3_NEWSTRINGVALUE:
            r=NewStringValue();
            break;
          default:
            r=Error();
            break;
        }

        if (!r)
        {
            dralListener->Error(
                "Error reading the events file. If it is compressed, "
                "it may have a CRC error");
            dralListener->EndSimulation();
            errorFound=true;
            break;
        }
    }
    return n_events;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::Error()
{
    dralListener->Error(
        "Unknown command code\n"
        "Please report this bug to dral@bssad.intel.com attaching "
        "input files (if any)"
        );
    return false;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::Cycle()
{
    struct cycleFormat
    {
        UINT64 commandCode  : 6;
        UINT64 n            : 58;
    } * commandCycle;

    commandCycle = (cycleFormat *) ((char *) ReadBytes(sizeof(cycleFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    dralListener->Cycle(commandCycle->n);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::StartActivity()
{
    struct startActivityFormat
    {
        UINT64 commandCode      : 6;
        UINT64 firstActCycle    : 58;
    } * commandStartActivity;

    commandStartActivity = (startActivityFormat *) ((char *) ReadBytes(sizeof(startActivityFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    dralListener->StartActivity(commandStartActivity->firstActCycle);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::NewNode()
{
    char * buffer2;
    UINT16 node_id;
    UINT16 parent_id;
    UINT16 instance;

    struct newNodeFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 2;
        UINT64 name_len     : 8;
        UINT64 node_id      : 16;
        UINT64 parent_id    : 16;
        UINT64 instance     : 16;
    } * commandNewNode;

    commandNewNode = (newNodeFormat *) ((char *) ReadBytes(sizeof(newNodeFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    if (commandNewNode->name_len == 0)
    {
        dralListener->NonCriticalError(
            "Found a newNode command with node name length == 0");
        return true;
    }

    node_id=commandNewNode->node_id;
    parent_id=commandNewNode->parent_id;
    instance=commandNewNode->instance;

    //read the node name
    buffer2 = (char *) ReadBytes(commandNewNode->name_len);
    if(EOS)
    {
        return false;
    }

    dralListener->NewNode(node_id,buffer2,parent_id,instance);
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::NewEdge()
{
    struct newEdgeFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 2;
        UINT64 name_len     : 8;
        UINT64 edge_id      : 16;
        UINT64 src          : 16;
        UINT64 dest         : 16;
    } * commandNewEdge;

    char * buffer2;
    UINT32 * buffer3;
    UINT16 edge_id;
    UINT16 src;
    UINT16 dest;
    UINT32 bandwidth;
    UINT32 latency;
    UINT16 name_len;

    commandNewEdge = (newEdgeFormat *) ((char *) ReadBytes(sizeof(newEdgeFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    if (commandNewEdge->name_len == 0)
    {
        dralListener->NonCriticalError(
            "Found a addedge command with edge name length == 0");
        return true;
    }

    edge_id=commandNewEdge->edge_id;
    src=commandNewEdge->src;
    dest=commandNewEdge->dest;
    name_len=commandNewEdge->name_len;

    buffer3 = (UINT32 *) ReadBytes(8); //read bandwidth and latency
    if(EOS)
    {
        return false;
    }

    bandwidth=buffer3[0];
    latency=buffer3[1];

    buffer2 = (char *) ReadBytes(name_len);
    if(EOS)
    {
        return false;
    }

    dralListener->NewEdge(src,dest,edge_id,bandwidth,latency,buffer2);
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetNodeLayout()
{
    struct setNodeLayoutFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 node_id;
        UINT16 dimensions;
    } * commandSetNodeLayout;

    UINT32 capacity;
    UINT32 * capacities;
    UINT16 node_id;
    UINT16 dimensions;

    capacity = 1;

    commandSetNodeLayout = (setNodeLayoutFormat *) ((char *) ReadBytes(sizeof(setNodeLayoutFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    node_id=commandSetNodeLayout->node_id;
    dimensions=commandSetNodeLayout->dimensions;

    if (dimensions == 0)
    {
        dralListener->NonCriticalError(
            "Found a setcapacity command with 0 dimensions");
        return true;
    }

    capacities = (UINT32 *)ReadBytes(sizeof(UINT32)*dimensions);
    if(EOS)
    {
        return false;
    }

    for (UINT16 i=0; i<dimensions; i++)
    {
        capacity*=capacities[i];
    }

    dralListener->SetNodeLayout(node_id,capacity,dimensions,capacities);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetNodeInputBandwidth()
{
    struct setNodeInputBandwidthFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 nodeId;
        UINT32 bandwidth;
    } * commandSetNodeInputBandwidth;

    commandSetNodeInputBandwidth = (setNodeInputBandwidthFormat *) ((char *) ReadBytes(sizeof(setNodeInputBandwidthFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    dralListener->SetNodeInputBandwidth(commandSetNodeInputBandwidth->nodeId, commandSetNodeInputBandwidth->bandwidth);
    
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetNodeOutputBandwidth()
{
    struct setNodeOutputBandwidthFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 nodeId;
        UINT32 bandwidth;
    } * commandSetNodeOutputBandwidth;

    commandSetNodeOutputBandwidth = (setNodeOutputBandwidthFormat *) ((char *) ReadBytes(sizeof(setNodeOutputBandwidthFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    dralListener->SetNodeOutputBandwidth(commandSetNodeOutputBandwidth->nodeId, commandSetNodeOutputBandwidth->bandwidth);
    
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::Comment()
{
    char * buffer2;
    UINT32 magic_num;
    UINT32 length;

    struct commentFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 strlen;
        UINT32 magic_num;
    } * commandComment;

    commandComment = (commentFormat *) ((char *) ReadBytes(sizeof(commentFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    if (commandComment->strlen == 0)
    {
        dralListener->NonCriticalError(
            "Found a comment command with comment length == 0");
        return true;
    }

    magic_num=commandComment->magic_num;
    length=commandComment->strlen;
    buffer2 = (char *) ReadBytes(commandComment->strlen);
    if(EOS)
    {
        return false;
    }

    dralListener->Comment(magic_num,buffer2);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::CommentBin()
{
    char * buffer2;

    struct commentBinFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 magic_num;
        UINT32 length;
    } * commandCommentBin;

    commandCommentBin = (commentBinFormat *) ((char *) ReadBytes(sizeof(commentBinFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT32 length = commandCommentBin->length;
    UINT16 magic_num = commandCommentBin->magic_num;

    if(length == 0)
    {
        dralListener->NonCriticalError(
            "Found a comment command with comment length == 0");
        return true;
    }

    buffer2 = (char *) ReadBytes(length);
    if(EOS)
    {
        return false;
    }

    dralListener->CommentBin(magic_num,buffer2,length);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetNodeClock()
{
    struct setNodeClockFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 nodeId;
        UINT16 clockId;
    } * commandSetNodeClockFormat;

    commandSetNodeClockFormat = (setNodeClockFormat *) ((char *) ReadBytes(sizeof(setNodeClockFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    dralListener->SetNodeClock(commandSetNodeClockFormat->nodeId, commandSetNodeClockFormat->clockId);
        
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::NewClock()
{
    char * buffer2;

    struct newClockFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 clockId;
        UINT16 skew;
        UINT16 divisions;
        UINT16 nameLen;
        UINT64 freq;
    } * commandNewClockFormat;

    commandNewClockFormat = (newClockFormat *) ((char *) ReadBytes(sizeof(newClockFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT16 clockId = commandNewClockFormat->clockId;
    UINT16 skew = commandNewClockFormat->skew;
    UINT16 divisions = commandNewClockFormat->divisions;
    UINT16 nameLen = commandNewClockFormat->nameLen;
    UINT64 freq = commandNewClockFormat->freq;

    buffer2 = (char *) ReadBytes(nameLen);
    if(EOS)
    {
        return false;
    }

    dralListener->NewClock(clockId, freq, skew, divisions, buffer2);
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::CycleWithClock()
{
    struct cycleFormat
    {
        UINT64 commandCode  : 6;
        UINT64 n            : 42;
        UINT64 clockId      : 16;
        UINT64 phase        : 16;
    } * commandCycleFormat;

    commandCycleFormat = (cycleFormat *) ((char *) ReadBytes(sizeof(cycleFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    dralListener->Cycle(
        commandCycleFormat->clockId, commandCycleFormat->n, commandCycleFormat->phase);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetTagDescription()
{
    char * buffer2;

    struct setTagDescriptionFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT8 tag_idx;
        UINT16 desclen;
    } * commandSetTagDescription;
    
    commandSetTagDescription = (setTagDescriptionFormat *) ((char *) ReadBytes(sizeof(setTagDescriptionFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 tag_idx = commandSetTagDescription->tag_idx;
    UINT16 desc_len = commandSetTagDescription->desclen;

    buffer2 = (char *) ReadBytes(desc_len);
    if(EOS)
    {
        return false;
    }

    dralListener->SetTagDescription(tag_idx, buffer2);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::NewItem(void * buffer)
{
    struct newItemFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
    } * commandNewItem;

    // WARNING: No bytes needed to be read, if called the function a NULL will be returned.
    //commandNewItem = (newItemFormat *) ((char *) ReadBytes(sizeof(newItemFormat) - 1) - 1);

    commandNewItem = (newItemFormat *) buffer;

    last_item = last_item + getDelta((DRAL3_VALUE_SIZE) commandNewItem->item_size);
    if(EOS)
    {
        return false;
    }

    dralListener->NewItem(last_item);
    
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::DeleteItem(void * buffer)
{
    struct deleteItemFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
    } * commandDeleteItem;

    // WARNING: No bytes needed to be read, if called the function a NULL will be returned.
    //commandDeleteItem = (deleteItemFormat *) ((char *) ReadBytes(sizeof(deleteItemFormat) - 1) - 1);
    commandDeleteItem = (deleteItemFormat *) buffer;

    last_item = last_item + getDelta((DRAL3_VALUE_SIZE) commandDeleteItem->item_size);
    if(EOS)
    {
        return false;
    }

    dralListener->DeleteItem(last_item);
    
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::MoveItems()
{
    struct moveItemsFormat
    {
        UINT8 commandCode      : 6;
        UINT8 item_size        : 2;
        UINT8 edge_size        : 2;
        UINT8 positionsPresent : 1;
        UINT8 n                : 5;
    } * commandMoveItems;

    commandMoveItems = (moveItemsFormat *) ((char *) ReadBytes(sizeof(moveItemsFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    //the values must be saved because '*buffer' may change if full
    UINT16 n = commandMoveItems->n;
    UINT8 positionsPresent = commandMoveItems->positionsPresent;
    DRAL3_VALUE_SIZE item_size = (DRAL3_VALUE_SIZE) commandMoveItems->item_size;

    if(n == 0)
    {
        dralListener->NonCriticalError("Found a move items command with n == 0");
        return true;
    }

    last_edge = last_edge + getDelta((DRAL3_VALUE_SIZE) commandMoveItems->edge_size);
    // MoveItems without positions
    UINT32 items[32];
    for(UINT32 i = 0; i < n; i++)
    {
        last_item = last_item + getDelta(item_size);
        items[i] = last_item;
    }

    if(EOS)
    {
        return false;
    }

    if(!positionsPresent)
    {
        dralListener->MoveItems(last_edge, n, items);
    }
    else
    {
        // MoveItems with positions.
        // Reading items and positions, all in one.
        UINT32 * positions = (UINT32 *) ReadBytes(n * sizeof(UINT32));
        if(EOS)
        {
            return false;
        }

        dralListener->MoveItemsWithPositions(last_edge, n, items, positions);
    }
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::EnterNode()
{
    struct enterNodeFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
        UINT8 node_size    : 2;
        UINT8 dim_size     : 2;
        UINT8 dimensions   : 4;
    } * commandEnterNode;

    commandEnterNode = (enterNodeFormat *) ((char *) ReadBytes(sizeof(enterNodeFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    DRAL3_VALUE_SIZE node_size = (DRAL3_VALUE_SIZE) commandEnterNode->node_size;
    DRAL3_VALUE_SIZE dim_size = (DRAL3_VALUE_SIZE) commandEnterNode->dim_size;
    DRAL3_VALUE_SIZE item_size = (DRAL3_VALUE_SIZE) commandEnterNode->item_size;
    UINT16 dimensions = commandEnterNode->dimensions;

    // Gets the item and node.
    last_item = last_item + getDelta(item_size);
    last_node = last_node + getDelta(node_size);

    if(EOS)
    {
        return false;
    }

    assert(dimensions < 16);

    UINT32 position[16];
    UINT32 last_pos = 0;

    // Computes the positions of the enter node.
    for(UINT32 i = 0; i < dimensions; i++)
    {
        last_pos = last_pos + getDelta(dim_size);
        position[i] = last_pos;
    }

    if(EOS)
    {
        return false;
    }

    dralListener->EnterNode(last_node, last_item, dimensions, position);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::ExitNode()
{
    struct exitNodeFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
        UINT8 node_size    : 2;
        UINT8 dim_size     : 2;
        UINT8 dimensions   : 4;
    } * commandExitNode;

    commandExitNode = (exitNodeFormat *) ((char *) ReadBytes(sizeof(exitNodeFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    DRAL3_VALUE_SIZE node_size = (DRAL3_VALUE_SIZE) commandExitNode->node_size;
    DRAL3_VALUE_SIZE dim_size = (DRAL3_VALUE_SIZE) commandExitNode->dim_size;
    DRAL3_VALUE_SIZE item_size = (DRAL3_VALUE_SIZE) commandExitNode->item_size;
    UINT16 dimensions = commandExitNode->dimensions;

    // Gets the item and node.
    last_item = last_item + getDelta(item_size);
    last_node = last_node + getDelta(node_size);

    if(EOS)
    {
        return false;
    }

    assert(dimensions < 16);

    UINT32 position[16];
    UINT32 last_pos = 0;

    // Computes the positions of the exit node.
    for(UINT32 i = 0; i < dimensions; i++)
    {
        last_pos = last_pos + getDelta(dim_size);
        position[i] = last_pos;
    }

    if(EOS)
    {
        return false;
    }

    dralListener->ExitNode(last_node, last_item, dimensions, position);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetItemTagValue(DRAL3_VALUE_SIZE item_size)
{
    struct setItemTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 val_size     : 2;
        UINT8 tag_idx;
    } * commandSetItemTag;

    commandSetItemTag = (setItemTagFormat *) ((char *) ReadBytes(sizeof(setItemTagFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 tag_idx = commandSetItemTag->tag_idx;
    DRAL3_VALUE_SIZE val_size = (DRAL3_VALUE_SIZE) commandSetItemTag->val_size;
    last_item = last_item + getDelta(item_size);

    UINT64 value = getValue(val_size);
    if(EOS)
    {
        return false;
    }

    dralListener->SetItemTag(last_item, tags[tag_idx], value);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetItemTagOther()
{
    struct setItemTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
        UINT8 tag_idx;
        UINT16 n;
    } * commandSetItemTag;
    
    commandSetItemTag = (setItemTagFormat *) ((char *) ReadBytes(sizeof(setItemTagFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    // The values must be saved because the internal buffer may change if full.
    UINT8 commandCode = commandSetItemTag->commandCode;
    UINT8 tag_idx = commandSetItemTag->tag_idx;
    UINT16 n = commandSetItemTag->n;
    UINT64 * value;
    last_item = last_item + getDelta((DRAL3_VALUE_SIZE) commandSetItemTag->item_size);

    if(EOS)
    {
        return false;
    }

    switch (commandCode)
    {
      case DRAL3_SETITEMTAG_STRING:
        dralListener->SetItemTagString(last_item, tags[tag_idx], strings[n]);
        break;

      case DRAL3_SETITEMTAG_SET:
        value = (UINT64 *) ReadBytes(n * sizeof(UINT64)); //read the set
        if(EOS)
        {
            return false;
        }

        dralListener->SetItemTagSet(last_item, tags[tag_idx], n, value);
        break;

      default:
        dralListener->Error(
            "Unknown SetTag variant\n"
            "Please report this bug to dral@bssad.intel.com attaching "
            "input files (if any)"
            );
        return false;
    }

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetNodeTagValueNoLevels(
    DRAL3_VALUE_SIZE node_size)
{
    struct setNodeTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 val_size     : 2;
        UINT8 tag_idx;
    } * commandSetNodeTag;

    commandSetNodeTag = (setNodeTagFormat *) ((char *) ReadBytes(sizeof(setNodeTagFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 tag_idx = commandSetNodeTag->tag_idx;
    DRAL3_VALUE_SIZE val_size = (DRAL3_VALUE_SIZE) commandSetNodeTag->val_size;
    last_node = last_node + getDelta(node_size);
    UINT64 value = getValue(val_size);
    if(EOS)
    {
        return false;
    }

    dralListener->SetNodeTag(last_node, tags[tag_idx], value, 0, NULL);
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetNodeTagValueLevels()
{
    struct setNodeTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 val_size     : 2;
        UINT8 tag_idx;
        UINT16 node_size   : 2;
        UINT16 levels      : 14;
    } * commandSetNodeTag;

    commandSetNodeTag = (setNodeTagFormat *) ((char *) ReadBytes(sizeof(setNodeTagFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 tag_idx = commandSetNodeTag->tag_idx;
    UINT16 levels = commandSetNodeTag->levels;
    DRAL3_VALUE_SIZE val_size = (DRAL3_VALUE_SIZE) commandSetNodeTag->val_size;
    DRAL3_VALUE_SIZE node_size = (DRAL3_VALUE_SIZE) commandSetNodeTag->node_size;
    last_node = last_node + getDelta(node_size);
    UINT64 value = getValue((DRAL3_VALUE_SIZE) val_size);
    if(EOS)
    {
        return false;
    }

    UINT32 * level_list = (UINT32 *) ReadBytes(levels * sizeof(UINT32));

    if(EOS)
    {
        return false;
    }

    dralListener->SetNodeTag(last_node, tags[tag_idx], value, levels, level_list);
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetNodeTagOther()
{
    UINT32 * list;
    UINT32 * buffer3 = NULL; // "= NULL" to avoid warning message
    UINT64 * value;
    bool mustDeleteList = false;

    struct setNodeTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 node_size    : 2;
        UINT8 tag_idx;
        UINT16 n;
        UINT16 levels;
    } * commandSetNodeTag;

    commandSetNodeTag = (setNodeTagFormat *) ((char *) ReadBytes(sizeof(setNodeTagFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 command = commandSetNodeTag->commandCode;
    UINT16 n = commandSetNodeTag->n;
    UINT8 tag_idx = commandSetNodeTag->tag_idx;
    UINT16 levels = commandSetNodeTag->levels;
    DRAL3_VALUE_SIZE node_size = (DRAL3_VALUE_SIZE) commandSetNodeTag->node_size;

    last_node = last_node + getDelta(node_size);

    if(EOS)
    {
        return false;
    }

    if (levels)
    {
        buffer3 = (UINT32 *) ReadBytes(levels*sizeof(UINT32)); // read the list
        if(EOS)
        {
            return false;
        }

        list = buffer3;
    }
    else
    {
        list=NULL;
    }

    switch (command)
    {
      case DRAL3_SETNODETAG_STRING:
        dralListener->SetNodeTagString(last_node, tags[tag_idx], strings[n], levels, list);
        break;

      case DRAL3_SETNODETAG_SET:
        if (dralRead->AvailableBytes() < n*sizeof(UINT64))
        {
            if (levels)
            {
                list = new UINT32 [levels];
                memcpy(list,buffer3,levels*sizeof(UINT32));
                mustDeleteList=true;
            }
        }

        value = (UINT64 *) ReadBytes(n*sizeof(UINT64)); //read the set
        if(EOS)
        {
            return false;
        }

        dralListener->SetNodeTagSet(last_node, tags[tag_idx], n, value, levels, list);
        break;

      default:
        dralListener->Error(
            "Unknown SetTag variant\n"
            "Please report this bug to dral@bssad.intel.com attaching "
            "input files (if any)"
            );
        return false;
    }

    if (mustDeleteList)
    {
        delete [] list;
    }
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetCycleTagValue()
{
    struct setCycleTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 val_size     : 2;
        UINT8 tag_idx;
    } * commandSetCycleTag;

    commandSetCycleTag = (setCycleTagFormat *) ((char *) ReadBytes(sizeof(setCycleTagFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 tag_idx = commandSetCycleTag->tag_idx;
    DRAL3_VALUE_SIZE val_size = (DRAL3_VALUE_SIZE) commandSetCycleTag->val_size;
    UINT64 value = getValue(val_size);
    if(EOS)
    {
        return false;
    }

    dralListener->SetCycleTag(tags[tag_idx], value);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::SetCycleTagOther()
{
    struct setCycleTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT8 tag_idx;
        UINT16 nval;
    } * commandSetCycleTag;

    char * buffer2;
    UINT64 * value;

    commandSetCycleTag = (setCycleTagFormat *) ((char *) ReadBytes(sizeof(setCycleTagFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 tag_idx = commandSetCycleTag->tag_idx;
    UINT16 n = commandSetCycleTag->nval;

    switch (commandSetCycleTag->commandCode)
    {
      case DRAL3_SETCYCLETAG_STRING:
        dralListener->SetCycleTagString(tags[tag_idx], strings[n]);
        break;

      case DRAL3_SETCYCLETAG_SET:
        value = (UINT64 *) ReadBytes(n * sizeof(UINT64)); //read the set
        if(EOS)
        {
            return false;
        }

        dralListener->SetCycleTagSet(tags[tag_idx], n, value);
        break;

      default:
        dralListener->Error(
            "Unknown SetTag variant\n"
            "Please report this bug to dral@bssad.intel.com attaching "
            "input files (if any)"
            );
        return false;
    }
    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::NewTag()
{
    struct newTagEntryFormat
    {
        UINT8 commandCode : 6;
        UINT8 reserved    : 2;
        UINT8 tag_id;
        UINT8 tag_len;
    } * tagEntry;

    tagEntry = (newTagEntryFormat *) ((char *) ReadBytes(sizeof(newTagEntryFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 tag_id = tagEntry->tag_id;
    UINT8 tag_len = tagEntry->tag_len;

    const char * tag = (const char *) ReadBytes(tag_len);

    if(EOS)
    {
        return false;
    }

    // Gets the mapping.
    tags[tag_id] = getTagIndex(tag, tag_len);

    return true;
}

bool
DRAL_CLIENT_BINARY_3_IMPLEMENTATION_CLASS::NewStringValue()
{
    struct newStringEntryFormat
    {
        UINT8 commandCode : 6;
        UINT8 reserved    : 2;
        UINT8 str_len;
        UINT16 str_id;
    } * stringEntry;

    stringEntry = (newStringEntryFormat *) ((char *) ReadBytes(sizeof(newStringEntryFormat) - 1) - 1);

    if(EOS)
    {
        return false;
    }

    UINT8 str_len = stringEntry->str_len;
    UINT16 str_id = stringEntry->str_id;

    const char * str = (const char *) ReadBytes(str_len);

    if(EOS)
    {
        return false;
    }

    strings[str_id] = getStringIndex(str, str_len);

    return true;
}
