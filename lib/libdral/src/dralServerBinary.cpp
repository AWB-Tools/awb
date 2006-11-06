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
 * @file dralServerBinary.cpp
 * @author Pau Cabre 
 * @brief dral server implementation using binary output
 */

#include "asim/dralServer.h"
#include "asim/dralServerBinaryDefines.h"
#include "asim/dralServerBinary.h"

DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS(UINT16 buffer_size, bool compression)
    : DRAL_SERVER_IMPLEMENTATION_CLASS(buffer_size,compression), tag_map(256), str_val_map(65536)
{
    lastClockId = (UINT16) -1;
    lastCycle = (UINT64) -1;
    lastPhase = (UINT16) -1;

    STATS
    (
        printf("DralServer stats enabled...\n");
        cycle = 0;
        cycleSize = 0;
        newItem = 0;
        newItemSize = 0;
        moveItem = 0;
        moveItemSize = 0;
        enterNode = 0;
        enterNodeSize = 0;
        exitNode = 0;
        exitNodeSize = 0;
        deleteItem = 0;
        deleteItemSize = 0;
        newNode = 0;
        newNodeSize = 0;
        newEdge = 0;
        newEdgeSize = 0;
        setNodeLayout = 0;
        setNodeLayoutSize = 0;
        comment = 0;
        commentSize = 0;
        version = 0;
        versionSize = 0;
        setNodeTagVal = 0;
        setItemTagValSize = 0;
        setItemTagStr = 0;
        setItemTagStrSize = 0;
        setItemTagSOV = 0;
        setItemTagSOVSize = 0;
        setNodeTagVal = 0;
        setNodeTagValSize = 0;
        setNodeTagStr = 0;
        setNodeTagStrSize = 0;
        setNodeTagSOV = 0;
        setNodeTagSOVSize = 0;
        setCycleTagVal = 0;
        setCycleTagValSize = 0;
        setCycleTagStr = 0;
        setCycleTagStrSize = 0;
        setCycleTagSOV = 0;
        setCycleTagSOVSize = 0;
        setNodeInput = 0;
        setNodeInputSize = 0;
        setNodeOutput = 0;
        setNodeOutputSize = 0;
        startActivity = 0;
        startActivitySize = 0;
        setTagDescription = 0;
        setTagDescriptionSize = 0;
        setNodeClock = 0;
        setNodeClockSize = 0;
        newClock = 0;
        newClockSize = 0;


        clearMoveItems();
        clearItems();
        clearNodes();
        clearEdges();

        numState = 0;
        numOccupancy = 0;
    )

    last_item = 0;
    last_node = 0;
    last_edge = 0;
}

DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::~DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS()
{
    STATS
    (
        dumpStats();
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::Cycle (
    UINT64 n)
{
    struct cycleFormat
    {
        UINT64 commandCode  : 6;
        UINT64 n            : 58;
    } command;

    command.commandCode=DRAL3_CYCLE;
    command.n=n;

    dralWrite->Write(&command, sizeof(command));

    STATS
    (
        cycle++;
        cycleSize += sizeof(cycleFormat);

        computeMoveItems();
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::StartActivity (
    UINT64 firstActCycle)
{
    struct startActivityFormat
    {
        UINT64 commandCode      : 6;
        UINT64 firstActCycle    : 58;
    } command;
    
    command.commandCode=DRAL3_STARTACTIVITY;
    command.firstActCycle=firstActCycle;

    dralWrite->Write(&command,sizeof(command));

    STATS
    (
        startActivity++;
        startActivitySize += sizeof(startActivityFormat);
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::NewNode (
    UINT16 node_id, const char name[], UINT16 name_len, UINT16 parent_id, UINT16 instance)
{
    struct newNodeFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 2;
        UINT64 name_len     : 8;
        UINT64 node_id      : 16;
        UINT64 parent_id    : 16;
        UINT64 instance     : 16;
    } command;

    STATS
    (
        //checkRange(node_id, &last_node, node_range);
    )

    command.commandCode=DRAL3_NEWNODE;
    command.reserved=0;
    command.name_len=name_len;
    command.node_id=node_id;
    command.parent_id=parent_id;
    command.instance=instance;

    dralWrite->Write(&command,sizeof(command));
    dralWrite->Write(name,name_len);

    STATS
    (
        newNode++;
        newNodeSize += sizeof(newNodeFormat) + name_len;
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::NewEdge(
    UINT16 edge_id, UINT16 source_node, UINT16 destination_node, UINT32 bandwidth, UINT32 latency, const char name[], UINT16 name_len)
{
    struct newEdgeFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 2;
        UINT64 name_len     : 8;
        UINT64 edge_id      : 16;
        UINT64 src          : 16;
        UINT64 dest         : 16;
        UINT64 bandwidth    : 32;
        UINT64 latency      : 32;
    } command;

    STATS
    (
        //checkRange(edge_id, &last_edge, edge_range);
    )

    command.commandCode=DRAL3_NEWEDGE;
    command.reserved=0;
    command.edge_id=edge_id;
    command.name_len=name_len;
    command.src=source_node;
    command.dest=destination_node;
    command.bandwidth=bandwidth;
    command.latency=latency;

    dralWrite->Write(&command,sizeof(command));
    dralWrite->Write(name,name_len);

    STATS
    (
        if(edge_id > max_edge)
        {
            max_edge = edge_id;
        }

        newEdge++;
        newEdgeSize += sizeof(newEdgeFormat) + name_len;
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetNodeLayout (
    UINT16 node_id, UINT16 dimensions, UINT32 capacity [])
{
    struct setNodeLayoutFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 node_id;
        UINT16 dimensions;
    } command;

    STATS
    (
        //checkRange(node_id, &last_node, node_range);
    )

    command.commandCode=DRAL3_SETNODELAYOUT;
    command.reserved=0;
    command.node_id=node_id;
    command.dimensions=dimensions;

    dralWrite->Write(&command,sizeof(command));

    dralWrite->Write(capacity,sizeof(capacity[0])*dimensions);

    STATS
    (
        setNodeLayout++;
        setNodeLayoutSize += sizeof(setNodeLayoutFormat) + (sizeof(UINT32) * dimensions);
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetNodeInputBandwidth (
    UINT16 nodeId, UINT32 bandwidth)
{
    struct setNodeInputBandwidthFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 nodeId;
        UINT32 bandwidth;
    } command;
    
    STATS
    (
        //checkRange(nodeId, &last_node, node_range);
    )

    command.commandCode=DRAL3_SETNODEINPUTBANDWIDTH;
    command.reserved = 0;
    command.nodeId=nodeId;
    command.bandwidth=bandwidth;
    
    dralWrite->Write(&command,sizeof(command));

    STATS
    (
        setNodeInput++;
        setNodeInputSize += sizeof(setNodeInputBandwidthFormat);
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetNodeOutputBandwidth (
    UINT16 nodeId, UINT32 bandwidth)
{
    struct setNodeOutputBandwidthFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 nodeId;
        UINT32 bandwidth;
    } command;
    
    STATS
    (
        //checkRange(nodeId, &last_node, node_range);
    )

    command.commandCode=DRAL3_SETNODEOUTPUTBANDWIDTH;
    command.nodeId=nodeId;
    command.bandwidth=bandwidth;
    
    dralWrite->Write(&command,sizeof(command));

    STATS
    (
        setNodeOutput++;
        setNodeOutputSize += sizeof(setNodeOutputBandwidthFormat);
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::Comment(
    UINT32 magic_num, const char comment [], UINT32 length)
{
    struct commentFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT32 strlen;
        UINT32 magic_num;
    } command;

    command.commandCode=DRAL3_COMMENT;
    command.reserved=0;
    command.strlen=length;
    command.magic_num=magic_num;

    dralWrite->Write(&command,sizeof(command));
    dralWrite->Write(comment,length);

    STATS
    (
        comment++;
        commentSize += sizeof(commentFormat) + length;
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::CommentBin(
    UINT16 magic_num, const char comment [], UINT32 len)
{
    struct commentFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 magic_num;
        UINT32 length;
    } command;
    
    command.commandCode=DRAL3_COMMENTBIN;
    command.reserved=0;
    command.length=len;
    command.magic_num=magic_num;

    dralWrite->Write(&command,sizeof(command));
    dralWrite->Write(comment,len);

    STATS
    (
        comment++;
        commentSize += sizeof(commentFormat) + len;
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetNodeClock(
    UINT16 nodeId, UINT16 clockId)
{
    struct setNodeClockFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 nodeId;
        UINT16 clockId;
    } command;

    STATS
    (
        //checkRange(nodeId, &last_node, node_range);
    )

    command.commandCode=DRAL3_SETNODECLOCK;
    command.reserved=0;
    command.nodeId=nodeId;
    command.clockId=clockId;

    dralWrite->Write(&command,sizeof(command));

    STATS
    (
        setNodeClock++;
        setNodeClockSize += sizeof(setNodeClockFormat);
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::NewClock(
    UINT16 clockId, UINT64 freq, UINT16 skew, UINT16 divisions,
    const char name [], UINT16 nameLen)
{
    struct newClockFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT16 clockId;
        UINT16 skew;
        UINT16 divisions;
        UINT16 nameLen;
        UINT64 freq;
    } command;

    command.commandCode=DRAL3_NEWCLOCK;
    command.reserved=0;
    command.clockId=clockId;
    command.skew=skew;
    command.divisions=divisions;
    command.nameLen=nameLen;
    command.freq=freq;

    dralWrite->Write(&command,sizeof(command));
    dralWrite->Write(name,nameLen);

    STATS
    (
        newClock++;
        newClockSize += sizeof(newClockFormat) + nameLen;
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::Cycle (
    UINT16 clockId, UINT64 n, UINT16 phase)
{
    // If the same clock than the last one, is not stored.
    if((clockId == lastClockId) && (n == lastCycle) && (phase == lastPhase))
    {
        return;
    }

    lastClockId = clockId;
    lastCycle = n;
    lastPhase = phase;

    struct cycleFormat
    {
        UINT64 commandCode  : 6;
        UINT64 n            : 42;
        UINT64 clockId      : 16;
        UINT64 phase        : 16;
    } command;

    command.commandCode=DRAL3_CYCLEWITHCLOCK;
    command.clockId=clockId;
    command.n=n;
    command.phase=phase;

    dralWrite->Write(&command,sizeof(command));

    STATS
    (
        cycle++;
        cycleSize += sizeof(cycleFormat);

        computeMoveItems();
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetTagDescription(
    const char tag [], UINT16 tag_len, const char description [], UINT16 desc_len)
{
    struct setTagDescriptionFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT8 tag_idx;
        UINT16 desclen;
    } command;
    
    command.commandCode = DRAL3_SETTAGDESCRIPTION;
    command.reserved = 0;
    command.tag_idx = getTagIndex(tag, tag_len);
    command.desclen = desc_len;

    dralWrite->Write(&command, sizeof(setTagDescriptionFormat));
    dralWrite->Write(description, desc_len);

    STATS
    (
        setTagDescription++;
        setTagDescriptionSize += sizeof(setTagDescriptionFormat) + desc_len;
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::Version (void)
{
    struct versionFormat
    {
        UINT64 commandCode      : 6;
        UINT64 reserved         : 26;
        UINT64 major_version    : 16;
        UINT64 minor_version    : 16;
    } command;

    command.commandCode = DRAL_VERSION;
    command.reserved = 0;
    command.major_version = DRAL_SERVER_VERSION_MAJOR;
    command.minor_version = DRAL_SERVER_VERSION_MINOR;

    dralWrite->Write(&command,sizeof(command));

    STATS
    (
        version++;
        versionSize += sizeof(versionFormat);
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::NewItem (
    UINT32 item_id)
{
    struct newItemFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
    } command;

    STATS
    (
        //checkRange(item_id, &last_item, item_range);
    )

    INT32 delta;

    delta = item_id - last_item;

    command.commandCode = DRAL3_NEWITEM;
    command.item_size = getRangeSize(delta);
    dralWrite->Write(&command, sizeof(command));

    last_item = item_id;

    STATS
    (
        newItem++;
        newItemSize += sizeof(newItemFormat);
    )

    switch(command.item_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                newItemSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                newItemSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            dralWrite->Write(&delta, sizeof(INT32));

            STATS
            (
                newItemSize += sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::DeleteItem (
    UINT32 item_id)
{
    struct deleteItemFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
    } command;

    STATS
    (
        //checkRange(item_id, &last_item, item_range);
    )

    INT32 delta;

    delta = item_id - last_item;

    command.commandCode = DRAL3_DELETEITEM;
    command.item_size = getRangeSize(delta);
    dralWrite->Write(&command, sizeof(command));

    last_item = item_id;

    STATS
    (
        deleteItem++;
        deleteItemSize += sizeof(deleteItemFormat);
    )

    switch(command.item_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                deleteItemSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                deleteItemSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            dralWrite->Write(&delta, sizeof(INT32));

            STATS
            (
                deleteItemSize += sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::MoveItems (
    UINT16 edge_id, UINT32 n, UINT32 item_id [], UINT32 positions [])
{
    struct moveItemsFormat
    {
        UINT8 commandCode      : 6;
        UINT8 item_size        : 2;
        UINT8 edge_size        : 2;
        UINT8 positionsPresent : 1;
        UINT8 n                : 5;
    } command;

    STATS
    (
        //checkRange(edge_id, &last_edge, edge_range);
    )

    STATS
    (
        /*for(UINT32 i = 0; i < n; i++)
        {
            checkRange(item_id[i], &last_item, item_range);
        }*/
    )

    bool positionsPresent = (positions != NULL);
    INT32 delta = edge_id - last_edge;
    last_edge = edge_id;
    command.commandCode = DRAL3_MOVEITEMS;
    command.item_size = getMaxBitsVector(n, item_id, last_item);
    command.edge_size = getRangeSize(delta);
    command.positionsPresent = positionsPresent;
    command.n = n;

    dralWrite->Write(&command, sizeof(command));

    STATS
    (
        moveItem++;
        moveItemSize += sizeof(moveItemsFormat);
        move_item_edges[edge_id] += n;
    )

    switch(command.edge_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                moveItemSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                moveItemSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
        case VALUE_64_BITS:
            assert(false);
            break;
    }

    switch(command.item_size)
    {
        case VALUE_8_BITS:
            for(UINT32 i = 0; i < n; i++)
            {
                delta = item_id[i] - last_item;
                last_item = item_id[i];
                dralWrite->Write(&delta, sizeof(INT8));
            }

            STATS
            (
                moveItemSize += n * sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            for(UINT32 i = 0; i < n; i++)
            {
                delta = item_id[i] - last_item;
                last_item = item_id[i];
                dralWrite->Write(&delta, sizeof(INT16));
            }

            STATS
            (
                moveItemSize += n * sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            for(UINT32 i = 0; i < n; i++)
            {
                delta = item_id[i] - last_item;
                last_item = item_id[i];
                dralWrite->Write(&delta, sizeof(INT32));
            }

            STATS
            (
                moveItemSize += n * sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }

    if(positionsPresent)
    {
        dralWrite->Write(positions, n * sizeof(UINT32));

        STATS
        (
            moveItemSize += n * sizeof(UINT32);
        )
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::EnterNode (
    UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position [])
{
    struct enterNodeFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
        UINT8 node_size    : 2;
        UINT8 dim_size     : 2;
        UINT8 dimensions   : 4;
    } command;

    STATS
    (
        //checkRange(node_id, &last_node, node_range);
        //checkRange(item_id, &last_item, item_range);
    )

    INT32 delta;

    delta = item_id - last_item;
    command.commandCode = DRAL3_ENTERNODE;
    command.item_size = getRangeSize(delta);
    delta = node_id - last_node;
    command.node_size = getRangeSize(delta);
    command.dim_size = getMaxBitsVector(dim, position, 0);
    command.dimensions = dim;

    dralWrite->Write(&command, sizeof(enterNodeFormat));

    STATS
    (
        enterNode++;
        enterNodeSize += sizeof(enterNodeFormat);
    )

    delta = item_id - last_item;
    last_item = item_id;

    switch(command.item_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                enterNodeSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                enterNodeSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            dralWrite->Write(&delta, sizeof(INT32));

            STATS
            (
                enterNodeSize += sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }

    delta = node_id - last_node;
    last_node = node_id;

    switch(command.node_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                enterNodeSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                enterNodeSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
        case VALUE_64_BITS:
            assert(false);
            break;
    }

    INT32 last_dim = 0;

    switch(command.dim_size)
    {
        case VALUE_8_BITS:
            for(UINT32 i = 0; i < dim; i++)
            {
                delta = position[i] - last_dim;
                dralWrite->Write(&delta, sizeof(INT8));
                last_dim = position[i];
            }

            STATS
            (
                enterNodeSize += dim * sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            for(UINT32 i = 0; i < dim; i++)
            {
                delta = position[i] - last_dim;
                dralWrite->Write(&delta, sizeof(INT16));
                last_dim = position[i];
            }

            STATS
            (
                enterNodeSize += dim * sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            for(UINT32 i = 0; i < dim; i++)
            {
                delta = position[i] - last_dim;
                dralWrite->Write(&delta, sizeof(INT32));
                last_dim = position[i];
            }

            STATS
            (
                enterNodeSize += dim * sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::ExitNode (
    UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position [])
{
    struct exitNodeFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
        UINT8 node_size    : 2;
        UINT8 dim_size     : 2;
        UINT8 dimensions   : 4;
    } command;

    STATS
    (
        //checkRange(node_id, &last_node, node_range);
        //checkRange(item_id, &last_item, item_range);
    )

    INT32 delta;

    delta = item_id - last_item;
    command.commandCode = DRAL3_EXITNODE;
    command.item_size = getRangeSize(delta);
    delta = node_id - last_node;
    command.node_size = getRangeSize(delta);
    command.dim_size = getMaxBitsVector(dim, position, 0);
    command.dimensions = dim;

    dralWrite->Write(&command, sizeof(exitNodeFormat));

    STATS
    (
        exitNode++;
        exitNodeSize += sizeof(exitNodeFormat);
    )

    delta = item_id - last_item;
    last_item = item_id;

    switch(command.item_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                exitNodeSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                exitNodeSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            dralWrite->Write(&delta, sizeof(INT32));

            STATS
            (
                exitNodeSize += sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }

    delta = node_id - last_node;
    last_node = node_id;

    switch(command.node_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                exitNodeSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                exitNodeSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
        case VALUE_64_BITS:
            assert(false);
            break;
    }

    INT32 last_dim = 0;

    switch(command.dim_size)
    {
        case VALUE_8_BITS:
            for(UINT32 i = 0; i < dim; i++)
            {
                delta = position[i] - last_dim;
                dralWrite->Write(&delta, sizeof(INT8));
                last_dim = position[i];
            }

            STATS
            (
                exitNodeSize += dim * sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            for(UINT32 i = 0; i < dim; i++)
            {
                delta = position[i] - last_dim;
                dralWrite->Write(&delta, sizeof(INT16));
                last_dim = position[i];
            }

            STATS
            (
                exitNodeSize += dim * sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            for(UINT32 i = 0; i < dim; i++)
            {
                delta = position[i] - last_dim;
                dralWrite->Write(&delta, sizeof(INT32));
                last_dim = position[i];
            }

            STATS
            (
                exitNodeSize += dim * sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetItemTag (
    UINT32 item_id, const char tag_name[], UINT16 tag_name_len, UINT64 value)
{
    struct setItemTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 val_size     : 2;
        UINT8 tag_idx;
    } command;

    STATS
    (
        //checkRange(item_id, &last_item, item_range);
    )

    INT32 delta = item_id - last_item;
    DRAL3_VALUE_SIZE item_size = getRangeSize(delta);
    last_item = item_id;

    UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);

    command.val_size = getValSize(value);
    command.tag_idx = tag_idx;

    switch(item_size)
    {
        case VALUE_8_BITS:
            command.commandCode = DRAL3_SETITEMTAG_VALUE_8_BITS;
            dralWrite->Write(&command, sizeof(command));
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                setItemTagValSize += sizeof(setItemTagFormat) + sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            command.commandCode = DRAL3_SETITEMTAG_VALUE_16_BITS;
            dralWrite->Write(&command, sizeof(command));
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                setItemTagValSize += sizeof(setItemTagFormat) + sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            command.commandCode = DRAL3_SETITEMTAG_VALUE_32_BITS;
            dralWrite->Write(&command, sizeof(command));
            dralWrite->Write(&delta, sizeof(INT32));

            STATS
            (
                setItemTagValSize += sizeof(setItemTagFormat) + sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }

    switch(command.val_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&value, sizeof(UINT8));

            STATS
            (
                setItemTagVal++;
                setItemTagValSize += sizeof(UINT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&value, sizeof(UINT16));

            STATS
            (
                setItemTagVal++;
                setItemTagValSize += sizeof(UINT16);
            )
            break;

        case VALUE_32_BITS:
            dralWrite->Write(&value, sizeof(UINT32));

            STATS
            (
                setItemTagVal++;
                setItemTagValSize += sizeof(UINT32);
            )
            break;

        case VALUE_64_BITS:
            dralWrite->Write(&value, sizeof(UINT64));

            STATS
            (
                setItemTagVal++;
                setItemTagValSize += sizeof(UINT64);
            )
            break;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetItemTag (
    UINT32 item_id, const char tag_name[], UINT16 tag_name_len, const char str[], UINT16 str_len)
{
    struct setItemTagString
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
        UINT8 tag_idx;
        UINT16 str_idx;
    } command;

    STATS
    (
        //checkRange(item_id, &last_item, item_range);
    )

    INT32 delta = item_id - last_item;
    last_item = item_id;
    UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);
    UINT16 str_idx = getStringIndex(str, str_len);

    command.commandCode = DRAL3_SETITEMTAG_STRING;
    command.item_size = getRangeSize(delta);
    command.tag_idx = tag_idx;
    command.str_idx = str_idx;
    dralWrite->Write(&command, sizeof(command));

    STATS
    (
        setItemTagStr++;
        setItemTagStrSize += sizeof(setItemTagString);
    )

    switch(command.item_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                setItemTagStrSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                setItemTagStrSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            dralWrite->Write(&delta, sizeof(INT32));

            STATS
            (
                setItemTagStrSize += sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetItemTag (
    UINT32 item_id, const char tag_name[],UINT16 tag_name_len, UINT32 nval, UINT64 value[])
{
    struct setItemTagSetFormat
    {
        UINT8 commandCode  : 6;
        UINT8 item_size    : 2;
        UINT8 tag_idx;
        UINT16 nval;
    } command;

    STATS
    (
        //checkRange(item_id, &last_item, item_range);
    )

    INT32 delta = item_id - last_item;
    last_item = item_id;
    UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);

    command.commandCode = DRAL3_SETITEMTAG_SET;
    command.item_size = getRangeSize(delta);;
    command.tag_idx = tag_idx;
    command.nval = nval;
    dralWrite->Write(&command, sizeof(command));

    STATS
    (
        setItemTagSOV++;
        setItemTagSOVSize += sizeof(setItemTagSetFormat) + tag_name_len + nval * sizeof(UINT64);
    )

    switch(command.item_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                setItemTagSOVSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                setItemTagSOVSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
            dralWrite->Write(&delta, sizeof(INT32));

            STATS
            (
                setItemTagSOVSize += sizeof(INT32);
            )
            break;

        case VALUE_64_BITS:
            assert(false);
            break;
    }

    dralWrite->Write(value, nval * sizeof(UINT64));

}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetNodeTag(
    UINT16 node_id, const char tag_name [], UINT16 tag_name_len, UINT64 val, UINT16 levels, UINT32 list [])
{
    STATS
    (
        //checkRange(node_id, &last_node, node_range);
    )

    if(levels == 0)
    {
        struct setNodeTagFormat
        {
            UINT8 commandCode  : 6;
            UINT8 val_size     : 2;
            UINT8 tag_idx;
        } command;

        INT32 delta = node_id - last_node;
        last_node = node_id;
        DRAL3_VALUE_SIZE node_size = getRangeSize(delta);
        UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);

        command.val_size = getValSize(val);
        command.tag_idx = tag_idx;

        STATS
        (
            setNodeTagVal++;
            setNodeTagValSize += sizeof(setNodeTagFormat);
        )

        switch(node_size)
        {
            case VALUE_8_BITS:
                command.commandCode = DRAL3_SETNODETAG_VALUE_WITHOUTLEVELS_8_BITS;
                dralWrite->Write(&command, sizeof(command));
                dralWrite->Write(&delta, sizeof(INT8));

                STATS
                (
                    setNodeTagValSize += sizeof(INT8);
                )
                break;

            case VALUE_16_BITS:
                command.commandCode = DRAL3_SETNODETAG_VALUE_WITHOUTLEVELS_16_BITS;
                dralWrite->Write(&command, sizeof(command));
                dralWrite->Write(&delta, sizeof(INT16));

                STATS
                (
                    setNodeTagValSize += sizeof(INT16);
                )
                break;

            case VALUE_32_BITS:
            case VALUE_64_BITS:
                assert(false);
                break;
        }

        switch(command.val_size)
        {
            case VALUE_8_BITS:
                dralWrite->Write(&val, sizeof(UINT8));

                STATS
                (
                    setNodeTagValSize += sizeof(UINT8);
                )
                break;

            case VALUE_16_BITS:
                dralWrite->Write(&val, sizeof(UINT16));

                STATS
                (
                    setNodeTagValSize += sizeof(UINT16);
                )
                break;

            case VALUE_32_BITS:
                dralWrite->Write(&val, sizeof(UINT32));

                STATS
                (
                    setNodeTagValSize += sizeof(UINT32);
                )
                break;

            case VALUE_64_BITS:
                dralWrite->Write(&val, sizeof(UINT64));

                STATS
                (
                    setNodeTagValSize += sizeof(UINT64);
                )
                break;
        }
    }
    else
    {
        struct setNodeTagFormat
        {
            UINT8 commandCode  : 6;
            UINT8 val_size     : 2;
            UINT8 tag_idx;
            UINT16 node_size   : 2;
            UINT16 levels      : 14;
        } command;

        INT32 delta = node_id - last_node;
        last_node = node_id;
        UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);

        command.commandCode = DRAL3_SETNODETAG_VALUE_WITHLEVELS;
        command.val_size = getValSize(val);
        command.tag_idx = tag_idx;
        command.node_size = getRangeSize(delta);
        command.levels = levels;

        dralWrite->Write(&command, sizeof(command));

        STATS
        (
            setNodeTagVal++;
            setNodeTagValSize += sizeof(setNodeTagFormat) + (levels * sizeof(UINT32));
        )

        switch(command.node_size)
        {
            case VALUE_8_BITS:
                dralWrite->Write(&delta, sizeof(INT8));

                STATS
                (
                    setNodeTagValSize += sizeof(INT8);
                )
                break;

            case VALUE_16_BITS:
                dralWrite->Write(&delta, sizeof(INT16));

                STATS
                (
                    setNodeTagValSize += sizeof(INT16);
                )
                break;

            case VALUE_32_BITS:
            case VALUE_64_BITS:
                assert(false);
                break;
        }

        switch(command.val_size)
        {
            case VALUE_8_BITS:
                dralWrite->Write(&val, sizeof(UINT8));

                STATS
                (
                    setNodeTagValSize += sizeof(UINT8);
                )
                break;

            case VALUE_16_BITS:
                dralWrite->Write(&val, sizeof(UINT16));

                STATS
                (
                    setNodeTagValSize += sizeof(UINT16);
                )
                break;

            case VALUE_32_BITS:
                dralWrite->Write(&val, sizeof(UINT32));

                STATS
                (
                    setNodeTagValSize += sizeof(UINT32);
                )
                break;

            case VALUE_64_BITS:
                dralWrite->Write(&val, sizeof(UINT64));

                STATS
                (
                    setNodeTagVal++;
                    setNodeTagValSize += sizeof(UINT64);
                )
                break;
        }

        dralWrite->Write(list, levels * sizeof(UINT32));
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetNodeTag(
    UINT16 node_id, const char tag_name [], UINT16 tag_name_len, const char str [], UINT16 str_len, UINT16 levels, UINT32 list [])
{
    STATS
    (
        //checkRange(node_id, &last_node, node_range);
    )

    struct setTagNodeStringFormat
    {
        UINT8 commandCode  : 6;
        UINT8 node_size    : 2;
        UINT8 tag_idx;
        UINT16 str_idx;
        UINT16 levels;
    } command;

    INT32 delta = node_id - last_node;
    last_node = node_id;
    UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);
    UINT16 str_idx = getStringIndex(str, str_len);

    command.commandCode = DRAL3_SETNODETAG_STRING;
    command.node_size = getRangeSize(delta);
    command.tag_idx = tag_idx;
    command.str_idx = str_idx;
    command.levels = levels;

    dralWrite->Write(&command, sizeof(command));

    STATS
    (
        setNodeTagStr++;
        setNodeTagStrSize += sizeof(setTagNodeStringFormat) + (levels * sizeof(UINT32));
    )

    switch(command.node_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                setNodeTagStrSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                setNodeTagStrSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
        case VALUE_64_BITS:
            assert(false);
            break;
    }

    dralWrite->Write(list, levels * sizeof(UINT32));
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetNodeTag(
    UINT16 node_id, const char tag_name [], UINT16 tag_name_len, UINT16 n, UINT64 set [], UINT16 levels, UINT32 list [])
{
    STATS
    (
        //checkRange(node_id, &last_node, node_range);
    )

    struct setTagNodeSetFormat
    {
        UINT8 commandCode  : 6;
        UINT8 node_size    : 2;
        UINT8 tag_idx;
        UINT16 n;
        UINT16 levels;
    } command;

    INT32 delta = node_id - last_node;
    last_node = node_id;
    UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);

    command.commandCode = DRAL3_SETNODETAG_SET;
    command.node_size = getRangeSize(delta);;
    command.tag_idx = tag_idx;
    command.n = n;
    command.levels = levels;

    dralWrite->Write(&command, sizeof(command));

    STATS
    (
        setNodeTagSOV++;
        setNodeTagSOVSize += sizeof(setTagNodeSetFormat) + (levels * sizeof(UINT32)) + (n * sizeof(UINT64));
    )

    switch(command.node_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&delta, sizeof(INT8));

            STATS
            (
                setNodeTagSOVSize += sizeof(INT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&delta, sizeof(INT16));

            STATS
            (
                setNodeTagSOVSize += sizeof(INT16);
            )
            break;

        case VALUE_32_BITS:
        case VALUE_64_BITS:
            assert(false);
            break;
    }

    dralWrite->Write(list, levels * sizeof(UINT32));
    dralWrite->Write(set, n * sizeof(UINT64));
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetCycleTag (
    const char tag_name[], UINT16 tag_name_len, UINT64 value)
{
    struct setCycleTagFormat
    {
        UINT8 commandCode  : 6;
        UINT8 val_size     : 2;
        UINT8 tag_idx;
    } command;

    UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);

    command.commandCode = DRAL3_SETCYCLETAG_VALUE;
    command.val_size = getValSize(value);
    command.tag_idx = tag_idx;

    dralWrite->Write(&command,sizeof(command));

    STATS
    (
        setCycleTagVal++;
        setCycleTagValSize += sizeof(setCycleTagFormat);
    )

    switch(command.val_size)
    {
        case VALUE_8_BITS:
            dralWrite->Write(&value, sizeof(UINT8));

            STATS
            (
                setCycleTagValSize += sizeof(UINT8);
            )
            break;

        case VALUE_16_BITS:
            dralWrite->Write(&value, sizeof(UINT16));

            STATS
            (
                setCycleTagValSize += sizeof(UINT16);
            )
            break;

        case VALUE_32_BITS:
            dralWrite->Write(&value, sizeof(UINT32));

            STATS
            (
                setCycleTagValSize += sizeof(UINT32);
            )
            break;

        case VALUE_64_BITS:
            dralWrite->Write(&value, sizeof(UINT64));

            STATS
            (
                setCycleTagValSize += sizeof(UINT64);
            )
            break;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetCycleTag (
    const char tag_name[], UINT16 tag_name_len, const char str[], UINT16 str_len)
{
    struct setCycleTagString
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT8 tag_idx;
        UINT16 str_idx;
    } command;

    UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);
    UINT16 str_idx = getStringIndex(str, str_len);

    command.commandCode = DRAL3_SETCYCLETAG_STRING;
    command.reserved = 0;
    command.tag_idx = tag_idx;
    command.str_idx = str_idx;

    dralWrite->Write(&command, sizeof(command));

    STATS
    (
        setCycleTagStr++;
        setCycleTagStrSize += sizeof(setCycleTagString);
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::SetCycleTag (
    const char tag_name[],UINT16 tag_name_len, UINT32 nval, UINT64 value[])
{
    struct setCycleTagSetFormat
    {
        UINT8 commandCode  : 6;
        UINT8 reserved     : 2;
        UINT8 tag_idx;
        UINT16 nval;
    } command;

    UINT8 tag_idx = getTagIndex(tag_name, tag_name_len);

    command.commandCode = DRAL3_SETCYCLETAG_SET;
    command.reserved = 0;
    command.tag_idx = tag_idx;
    command.nval = nval;

    dralWrite->Write(&command, sizeof(command));
    dralWrite->Write(value, nval * sizeof(UINT64));

    STATS
    (
        setCycleTagSOV++;
        setCycleTagSOVSize += sizeof(setCycleTagSetFormat) + (nval * sizeof(UINT64));
    )
}

DRAL3_VALUE_SIZE
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::getMaxBitsVector(UINT32 n, UINT32 * values, INT32 last_value)
{
    DRAL3_VALUE_SIZE max_bits;
    DRAL3_VALUE_SIZE temp_max_bits;
    INT32 delta;

    max_bits = VALUE_8_BITS;

    // Gets the maximum number of bits needed to represent the deltas.
    for(UINT32 i = 0; i < n; i++)
    {
        delta = values[i] - last_value;
        temp_max_bits = getRangeSize(delta);
        if(temp_max_bits > max_bits)
        {
            max_bits = temp_max_bits;
        }
        last_value = values[i];
    }

    return max_bits;
}

STATS
(
void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::dumpStats() const
{
    STATS
    (
        printf("Dumping Dral Server statistics:\n");
        printf("Cycle, %llu, %llu\n", cycle, cycleSize);
        printf("New Item, %llu, %llu\n", newItem, newItemSize);
        printf("Delete Item, %llu, %llu\n", deleteItem, deleteItemSize);
        printf("Move Item, %llu, %llu\n", moveItem, moveItemSize);
        printf("Enter Node, %llu, %llu\n", enterNode, enterNodeSize);
        printf("Exit Node, %llu, %llu\n", exitNode, exitNodeSize);
        printf("New Node, %llu, %llu\n", newNode, newNodeSize);
        printf("New Edge, %llu, %llu\n", newEdge, newEdgeSize);
        printf("Set Node Layout, %llu, %llu\n", setNodeLayout, setNodeLayoutSize);
        printf("Comment, %llu, %llu\n", comment, commentSize);
        printf("Version, %llu, %llu\n", version, versionSize);
        printf("Set Item Tag Value, %llu, %llu\n", setItemTagVal, setItemTagValSize);
        printf("Set Item Tag String, %llu, %llu\n", setItemTagStr, setItemTagStrSize);
        printf("Set Item Tag SOV, %llu, %llu\n", setItemTagSOV, setItemTagSOVSize);
        printf("Set Node Tag Value, %llu, %llu\n", setNodeTagVal, setNodeTagValSize);
        printf("Set Node Tag String, %llu, %llu\n", setNodeTagStr, setNodeTagStrSize);
        printf("Set Node Tag SOV, %llu, %llu\n", setNodeTagSOV, setNodeTagSOVSize);
        printf("Set Cycle Tag Value, %llu, %llu\n", setCycleTagVal, setCycleTagValSize);
        printf("Set Cycle Tag String, %llu, %llu\n", setCycleTagStr, setCycleTagStrSize);
        printf("Set Cycle Tag SOV, %llu, %llu\n", setCycleTagSOV, setCycleTagSOVSize);
        printf("Set Node Input, %llu, %llu\n", setNodeInput, setNodeInputSize);
        printf("Set Node Output, %llu, %llu\n", setNodeOutput, setNodeOutputSize);
        printf("Start Activity, %llu, %llu\n", startActivity, startActivitySize);
        printf("Set Tag Description, %llu, %llu\n", setTagDescription, setTagDescriptionSize);
        printf("Set Node Clock, %llu, %llu\n", setNodeClock, setNodeClockSize);
        printf("New Clock, %llu, %llu\n", newClock, newClockSize);
        printf("\n\n");

        printf("MoveItemCollapse mean\n");
        for(UINT32 i = 0; i < 512; i++)
        {
            if(total_move_item_edges[i] != 0)
            {
                printf("%i, %llu\n", i, total_move_item_edges[i]);
            }
        }
        printf("\n\n");

        printf("Item delta bits:\n");
        printf("8 bits, %i\n", item_range[VALUE_8_BITS]);
        printf("16 bits, %i\n", item_range[VALUE_16_BITS]);
        printf("32 bits, %i\n", item_range[VALUE_32_BITS]);
        printf("64 bits, %i\n", item_range[VALUE_64_BITS]);
        printf("\n\n");

        printf("Node delta bits:\n");
        printf("8 bits, %i\n", node_range[VALUE_8_BITS]);
        printf("16 bits, %i\n", node_range[VALUE_16_BITS]);
        printf("32 bits, %i\n", node_range[VALUE_32_BITS]);
        printf("64 bits, %i\n", node_range[VALUE_64_BITS]);
        printf("\n\n");

        printf("Edge delta bits:\n");
        printf("8 bits, %i\n", edge_range[VALUE_8_BITS]);
        printf("16 bits, %i\n", edge_range[VALUE_16_BITS]);
        printf("32 bits, %i\n", edge_range[VALUE_32_BITS]);
        printf("64 bits, %i\n", edge_range[VALUE_64_BITS]);
        printf("\n\n");

        printf("Num SetNodeTag Occupancy: %i\n", numOccupancy);
        printf("Num SetNodeTag State: %i\n", numState);
        printf("\n\n");
    )
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::clearMoveItems()
{
    max_edge = 0;

    for(UINT32 i = 0; i < 65536; i++)
    {
        move_item_edges[i] = 0;
        total_move_item_edges[i] = 0;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::computeMoveItems()
{
    for(UINT32 i = 0; i < max_edge; i++)
    {
        if(move_item_edges[i] != 0)
        {
            total_move_item_edges[move_item_edges[i]]++;
        }
        move_item_edges[i] = 0;
    }
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::clearItems()
{
    item_range[VALUE_8_BITS] = 0;
    item_range[VALUE_16_BITS] = 0;
    item_range[VALUE_32_BITS] = 0;
    item_range[VALUE_64_BITS] = 0;
    last_item = 0;
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::clearNodes()
{
    node_range[VALUE_8_BITS] = 0;
    node_range[VALUE_16_BITS] = 0;
    node_range[VALUE_32_BITS] = 0;
    node_range[VALUE_64_BITS] = 0;
    last_node = 0;
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::clearEdges()
{
    edge_range[VALUE_8_BITS] = 0;
    edge_range[VALUE_16_BITS] = 0;
    edge_range[VALUE_32_BITS] = 0;
    edge_range[VALUE_64_BITS] = 0;
}

void
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::checkRange(INT32 id, INT32 * last_id, UINT32 * ranges)
{
    if(id >= * last_id)
    {
        INT64 diff = id - (* last_id);

        if(diff < 128)
        {
            ranges[VALUE_8_BITS]++;
        }
        else if(diff < 32768)
        {
            ranges[VALUE_16_BITS]++;
        }
        else
        {
            ranges[VALUE_32_BITS]++;
        }
    }
    else
    {
        INT64 diff = (* last_id) - id;

        if(diff <= 128)
        {
            ranges[VALUE_8_BITS]++;
        }
        else if(diff <= 32768)
        {
            ranges[VALUE_16_BITS]++;
        }
        else
        {
            ranges[VALUE_32_BITS]++;
        }
    }
    * last_id = id;
}

)
